#include "pch.h"
#include "AAnimatableObject.h"
#include "AModelRepos.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAnimatableObject );

// $Barnyard: FUNCTION 0057cff0
AAnimatableObject::AAnimatableObject()
    : m_pUnk1( TNULL )
    , m_pObjectType( TNULL )
    , m_pModelInstance( TNULL )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 0057d0a0
// $Barnyard: FUNCTION 0057c530
AAnimatableObject::~AAnimatableObject()
{
	m_llAnimControllers.DeleteAll();

	if ( m_pModelInstance )
	{
		AModelInstance* pModelInstance = m_pModelInstance;
		m_pModelInstance               = TNULL;

		AModelRepos::GetSingleton()->DestroyModelInstance( pModelInstance );
	}

	m_llAttachedObjects.Clear();
}

// $Barnyard: FUNCTION 0057c650
TBOOL AAnimatableObject::Create( AAnimatableObjectType* a_pObjectType, void* a_Unk1, CREATE_FLAGS a_eFlags )
{
	m_pUnk1       = a_Unk1;
	m_pObjectType = a_pObjectType;

	// Instantiate a model
	m_pModelInstance = AModelRepos::GetSingleton()->InstantiateModel( a_pObjectType->GetModel() );

	if ( m_pModelInstance )
	{
		if ( !T2String8::CompareNoCase( "sunglasses", a_pObjectType->GetName(), 10 ) )
		{
			// Instantiating sunglasses model
			m_pModelInstance->SetReceivesLight( TFALSE );
		}

		TSceneObject* pSceneObject = m_pModelInstance->GetSceneObject();

		pSceneObject->GetSomeVector2().y = 1.0;
		pSceneObject->EnableUnknown1();
		m_eFlags |= FLAGS_CREATED;

		m_oToshiAnimInterface.SetSkeletonInstance( pSceneObject->GetSkeletonInstance() );
		m_oToshiAnimInterface.SetAnimationSet( a_pObjectType->GetDefaultAnimationSet() );

		if ( !( a_eFlags & CREATE_FLAGS_NO_ANIM_CONTROLLER ) )
		{
			// Create animation controllers
			T2_FOREACH( a_pObjectType->GetAnimControllerTypes(), it )
			{
				TClass* pControllerClass = it->GetControllerClass();
				TVALIDPTR( pControllerClass );

				AAnimController* pAnimController = TSTATICCAST( AAnimController, pControllerClass->CreateObject() );
				TVALIDPTR( pAnimController );

				TBOOL bCreated = pAnimController->Create( a_Unk1, this, it );
				TASSERT( TTRUE == bCreated );

				m_llAnimControllers.PushBack( pAnimController );
			}
		}

		a_pObjectType->OnAnimatableObjectCreated( this );
	}

	return m_eFlags & FLAGS_CREATED;
}

// $Barnyard: FUNCTION 0057d280
TBOOL AAnimatableObject::Update( TFLOAT a_fDeltaTime )
{
	if ( !HASANYFLAG( m_eFlags, FLAGS_CREATED ) )
		return TTRUE;

	AToshiAnimationInterface::AnimEventList animEvents;

	PlayQueuedAnimation();
	m_oToshiAnimInterface.Update( a_fDeltaTime, animEvents );

	// Update attached animation controllers
	UpdateAnimControllers( a_fDeltaTime );

	// Handle the animation events
	for ( TINT i = 0; i < animEvents.iNumEvents; i++ )
	{
		if ( animEvents.aEvents[ i ].eType == AToshiAnimationInterface::ANIMEVENT_TYPE_BREAKPOINT )
			TASSERT( !"Call the breakpoint" );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0057bf00
void AAnimatableObject::UpdateAnimControllers( TFLOAT a_fDeltaTime )
{
	T2_FOREACH( m_llAnimControllers, it )
	{
		if ( it->IsEnabled() )
			it->OnUpdate( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0057d130
TBOOL AAnimatableObject::PlayAnimation( const Toshi::TPString8& a_strName )
{
	if ( !StartAnimation( a_strName ) )
	{
		// Couldn't play animation right now, play it later
		TASSERT( m_vecQueuedAnims.Size() != m_vecQueuedAnims.CAPACITY );

		m_vecQueuedAnims.PushBack( a_strName );
		return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0057c8d0
TBOOL AAnimatableObject::StartAnimation( const TPString8& a_strName )
{
	ANamedAnimation* pAnimation = m_oToshiAnimInterface.GetAnimationSet()->GetNamedAnimation( a_strName );
	TVALIDPTR( pAnimation );

	if ( !pAnimation )
		return TTRUE;

	// Make sure this animation isn't skipped by any controller
	TBOOL bShouldPlay = TTRUE;
	T2_FOREACH( m_llAnimControllers, it )
	{
		if ( it->IsEnabled() )
		{
			bShouldPlay &= it->IsAnimationAllowed( pAnimation );
		}
	}

	if ( bShouldPlay )
		m_oToshiAnimInterface.PlayAnim( a_strName, -1.0f, TFALSE );

	return bShouldPlay;
}

// $Barnyard: FUNCTION 0057d0c0
void AAnimatableObject::PlayQueuedAnimation()
{
	T2_FOREACH( m_vecQueuedAnims, it )
	{
		if ( StartAnimation( *it ) )
		{
			// This animation has been started, remove it from the queue
			m_vecQueuedAnims.EraseFast( it );
			it--;
		}
	}
}

// $Barnyard: FUNCTION 0057c2f0
void AAnimatableObject::SetVisible( TBOOL a_bVisible )
{
	TVALIDPTR( m_pModelInstance );

	m_pModelInstance->SetVisible( a_bVisible );
}

// $Barnyard: FUNCTION 0057c210
void AAnimatableObject::SetSkeletonUpdating( TBOOL a_bUpdating, TBOOL a_bRecursive )
{
	TVALIDPTR( m_pModelInstance );

	m_pModelInstance->SetSkeletonUpdating( a_bUpdating );

	if ( a_bRecursive )
	{
		// Update settings of attached objects
		T2_FOREACH( m_llAttachedObjects, it )
		{
			it->SetSkeletonUpdating( a_bUpdating, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 0057d190
void AAnimatableObject::KillAllAnimations()
{
	m_vecQueuedAnims.Clear();
	m_oToshiAnimInterface.KillAllAnimations();
}
