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
	m_UnkList.DeleteAll();

	if ( m_pModelInstance )
	{
		AModelInstance* pModelInstance = m_pModelInstance;
		m_pModelInstance               = TNULL;

		AModelRepos::GetSingleton()->DestroyModelInstance( pModelInstance );
	}

	m_llAttachedObjects.Clear();
}

// $Barnyard: FUNCTION 0057c650
TBOOL AAnimatableObject::Create( AAnimatableObjectType* a_pObjectType, void* a_Unk1, TUINT a_eFlags )
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

		if ( !( a_eFlags & 1 ) )
		{
			TASSERT( !"It's time to implement this *something*" );
		}

		a_pObjectType->OnAnimatableObjectCreated( this );
	}

	return m_eFlags & FLAGS_CREATED;
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
	m_vecPlayingAnims.Clear();
	m_oToshiAnimInterface.KillAllAnimations();
}
