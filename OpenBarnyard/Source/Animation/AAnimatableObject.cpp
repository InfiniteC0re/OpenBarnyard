#include "pch.h"
#include "AAnimatableObject.h"
#include "AModelRepos.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "Units/AAnimatedUnit.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( AAnimatableObject );

// DAT_00789988
AFunctionTable AAnimatableObject::ms_oFunctionTable( &TGetClass( AAnimatableObject ), TNULL );

// FSM fallback: resolves the animatable object of the unit
// $Barnyard: FUNCTION 0057b930
static TObject* __stdcall AAnimatableObjectFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	if ( a_pCaller )
	{
		TObject* pObject = a_pCaller->GetObject();

		if ( pObject && pObject->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
			return TSTATICCAST( AAnimatedUnit, pObject )->GetAnimatableObject();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057d1b0
void AAnimatableObject::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AAnimatableObjectFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AAnimatableObject, const TPString8&>::Create( "PlayAnimation", &AAnimatableObject::PlayAnimation ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AAnimatableObject, const TPString8&>::Create( "IsAnimationFinished", &AAnimatableObject::IsAnimationFinished ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AAnimatableObject, const TPString8&>::Create( "FinishAnimation", &AAnimatableObject::FinishAnimation ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AAnimatableObject, const TPString8&>::Create( "SwitchToNamedAnimationSet", &AAnimatableObject::SwitchToNamedAnimationSet ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AAnimatableObject>::Create( "IsInCameraFrustum", &AAnimatableObject::IsInCameraFrustum ) );
}

// $Barnyard: FUNCTION 0057b570
void AAnimatableObject::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0057ce20
TBOOL AAnimatableObject::SwitchToNamedAnimationSet( const TPString8& a_strName )
{
	ANamedAnimationSetRef pAnimationSet        = m_pObjectType->FindAnimationSetOrDefault( a_strName );
	ANamedAnimationSetRef pCurrentAnimationSet = m_oToshiAnimInterface.GetAnimationSet();

	if ( pAnimationSet.Get() != pCurrentAnimationSet.Get() )
	{
		m_oToshiAnimInterface.SetAnimationSet( pAnimationSet );
		m_oToshiAnimInterface.KillAllAnimations();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0057c200
TBOOL AAnimatableObject::IsInCameraFrustum()
{
	return m_pModelInstance.Get()->IsVisible();
}

// $Barnyard: FUNCTION 0057cff0
AAnimatableObject::AAnimatableObject()
    : m_pOwner( TNULL )
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

		g_pModelRepos->DestroyModelInstance( pModelInstance );
	}

	m_llAttachedObjects.Clear();
}

// $Barnyard: FUNCTION 0057c650
TBOOL AAnimatableObject::Create( AAnimatableObjectType* a_pObjectType, void* a_pOwner, CREATE_FLAGS a_eFlags )
{
	m_pOwner      = a_pOwner;
	m_pObjectType = a_pObjectType;

	// Instantiate a model
	m_pModelInstance = g_pModelRepos->InstantiateModel( a_pObjectType->GetModel() );

	if ( m_pModelInstance )
	{
		if ( !T2String8::CompareNoCase( "sunglasses", a_pObjectType->GetName(), 10 ) )
		{
			// Instantiating sunglasses model
			m_pModelInstance->SetReceivesLight( TFALSE );
		}

		TSceneObject* pSceneObject = m_pModelInstance->GetSceneObject();

		pSceneObject->GetFadeParams().y = 1.0;
		pSceneObject->EnableOpacity();
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

				TBOOL bCreated = pAnimController->Create( a_pOwner, this, it );
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
			animEvents.aEvents[ i ].pNextBreakpoint->Execute( this, animEvents.aEvents[ i ].fTimeFromBreakpoint );
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

// $Barnyard: FUNCTION 0057c280
void AAnimatableObject::SetOpacity( TFLOAT a_fOpacity, TBOOL a_bRecursive )
{
	m_pModelInstance->SetOpacity( a_fOpacity );

	if ( a_bRecursive == TTRUE )
	{
		T2_FOREACH( m_llAttachedObjects, it )
		{
			it->SetOpacity( a_fOpacity, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 0057c400
TBOOL AAnimatableObject::ApplyMaterial( TINT a_iSkin, TBOOL a_bSkinLighting )
{
	AAnimatableObjectType* pObjectType = m_pObjectType;

	const TCHAR* szMaterialName = pObjectType->GetMaterialName();
	const TCHAR* szTextureName  = pObjectType->GetSkinTexture( a_iSkin );
	const TCHAR* szSkinLight    = a_bSkinLighting ? pObjectType->GetSkinLighting( a_iSkin ) : TNULL;

	if ( szMaterialName && szTextureName )
	{
		GetModelInstance()->ApplyMaterial( szTextureName, szMaterialName, szSkinLight );
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0057c0a0
AAnimatableObject* AAnimatableObject::FindAttachedObject( const TPString8& a_rcBone )
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		const TCHAR* szParentBone = GetModelInstance()->GetBoneName( it->m_oAttachmentInfo.iParentBone );

		if ( TStringManager::String8CompareNoCase( a_rcBone.GetString(), szParentBone ) == 0 )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057c040
AAnimatableObject* AAnimatableObject::FindAttachedObjectOfType( const TPString8& a_rcTypeName )
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		if ( it->m_pObjectType->GetName() == a_rcTypeName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057cad0
void AAnimatableObject::SetAttachedSkeletonUpdatingOfType( const TPString8& a_rcTypeName, TBOOL a_bUpdating, TBOOL a_bRecursive )
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		if ( it->m_pObjectType->GetName() == a_rcTypeName )
			it->SetSkeletonUpdating( a_bUpdating, a_bRecursive );
	}
}

// $Barnyard: FUNCTION 0057cb40
void AAnimatableObject::SetAttachedSkeletonUpdatingOnBone( const TPString8& a_rcBone, TBOOL a_bUpdating, TBOOL a_bRecursive )
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		const TCHAR* szParentBone = GetModelInstance()->GetBoneName( it->m_oAttachmentInfo.iParentBone );

		if ( TStringManager::String8CompareNoCase( a_rcBone.GetString(), szParentBone ) == 0 )
			it->SetSkeletonUpdating( a_bUpdating, a_bRecursive );
	}
}

// $Barnyard: FUNCTION 0057b7f0
void AAnimatableObject::SetAttachmentTransform( const TVector4& a_rcTranslation, const TQuaternion& a_rcRotation )
{
	m_oAttachmentInfo.translation = a_rcTranslation;
	m_oAttachmentInfo.quaternion  = a_rcRotation;
}

// $Barnyard: FUNCTION 0057bfe0
void AAnimatableObject::SetAnimControllersDisabled( TBOOL a_bDisabled )
{
	T2_FOREACH( m_llAnimControllers, it )
	{
		it->SetDisabled( a_bDisabled );
	}
}

// $Barnyard: FUNCTION 0057bf70
AAnimController* AAnimatableObject::FindAnimController( TClass* a_pClass )
{
	T2_FOREACH( m_llAnimControllers, it )
	{
		if ( it->GetClass() == a_pClass )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057cac0
void AAnimatableObject::DestroyAnimControllers()
{
	m_llAnimControllers.DeleteAll();
}

// $Barnyard: FUNCTION 0057c850
TBOOL AAnimatableObject::HasAnimation( const TPString8& a_rcName )
{
	return m_oToshiAnimInterface.GetAnimationSet()->GetNamedAnimation( a_rcName ) != TNULL;
}

// $Barnyard: FUNCTION 0057bdb0
void AAnimatableObject::UpdateAttachedObjectTransforms()
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		it->UpdateAttachmentTransform();
	}
}

// $Barnyard: FUNCTION 0057bb90
void AAnimatableObject::UpdateAttachmentTransform()
{
	AAnimatableObject* pParent = m_oAttachmentInfo.pParentObject;

	TMatrix44 matParentBone;
	pParent->GetModelInstance()->GetScaledBoneTransform( m_oAttachmentInfo.iParentBone, matParentBone );

	TMatrix44 matParent;
	pParent->GetModelInstance()->GetLocalTransform( matParent );

	TMatrix44 matTransform;
	matTransform.Multiply( matParent, matParentBone );

	// Apply the attachment offset
	TMatrix44 matOffset;
	matOffset.SetFromQuaternion( m_oAttachmentInfo.quaternion );
	matOffset.GetTranslation() = m_oAttachmentInfo.translation;

	TMatrix44 matTemp = matTransform;
	matTransform.Multiply( matTemp, matOffset );

	if ( m_oAttachmentInfo.iBone != -1 )
	{
		// Make our bone the attachment point
		GetModelInstance()->GetScaledBoneTransform( m_oAttachmentInfo.iBone, matOffset );
		matOffset.InvertOrthogonal();

		matTemp = matTransform;
		matTransform.Multiply( matTemp, matOffset );
	}

	GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 0057b970
void AAnimatableObject::AttachToLocator( const TCHAR* a_szLocator )
{
	m_oAttachmentInfo.pLocator = ATerrainInterface::GetSingleton()->FindLocator( a_szLocator );
}

// $Barnyard: FUNCTION 0057be00
void AAnimatableObject::UpdateLocatorTransform()
{
	// Units in speech dialog mode are placed by the dialog
	ASteer* pSteer = TSTATICCAST( AUnit, m_pOwner )->GetSteer();

	if ( pSteer && pSteer->IsA( &TGetClass( AInteriorSteer ) ) && TSTATICCAST( AInteriorSteer, pSteer )->IsSpeechDialogMode() )
		return;

	TMatrix44 matLocator;
	m_oAttachmentInfo.pLocator->GetMatrix( matLocator );

	TMatrix44 matWorld;
	ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform().GetLocalMatrixImp( matWorld );

	TMatrix44 matTransform;
	matTransform.Multiply( matWorld, matLocator );

	GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 0057b630
TBOOL AAnimatableObject::IsAnimPlaying( const TPString8& a_strName, TFLOAT a_fMinWeight )
{
	AToshiAnimationInterface& rAnimInterface = m_oToshiAnimInterface;
	AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), a_strName );

	if ( a_fMinWeight <= 0.0f || !pAnimRef )
		return rAnimInterface.IsAnimPlaying( a_strName, TFALSE );

	TFLOAT      fWeight    = rAnimInterface.GetAnimWeight( pAnimRef );
	TAnimation* pAnimation = rAnimInterface.GetAnimation( *pAnimRef );

	return pAnimation->GetState() != TAnimation::STATE_BLENDING_OUT && a_fMinWeight <= fWeight;
}

// $Barnyard: FUNCTION 0057b5f0
TBOOL AAnimatableObject::FinishAnimation( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

		if ( !pAnimRef )
			return TFALSE;
	}

	return m_oToshiAnimInterface.StopAnim( *pAnimRef );
}

// $Barnyard: FUNCTION 0057b6b0
TBOOL AAnimatableObject::SetAnimWeight( const TPString8& a_strName, TFLOAT a_fWeight )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

		if ( !pAnimRef )
			return TFALSE;
	}

	return m_oToshiAnimInterface.SetAnimWeight( pAnimRef, a_fWeight );
}

// $Barnyard: FUNCTION 0057b6f0
TFLOAT AAnimatableObject::GetAnimationTime( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

		if ( !pAnimRef )
			return 0.0f;
	}

	return m_oToshiAnimInterface.GetAnimTime( pAnimRef );
}

// $Barnyard: FUNCTION 0057b7b0
TBOOL AAnimatableObject::SetAnimationTime( const TPString8& a_strName, TFLOAT a_fTime )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.SetAnimTime( pAnimRef, a_fTime );

	pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.SetAnimTime( pAnimRef, a_fTime );

	return TFALSE;
}

// $Barnyard: FUNCTION 0057ca80
TBOOL AAnimatableObject::IsAnimationFinished( const TPString8& a_strName )
{
	return !IsAnimationPlanned( a_strName ) && !IsAnimPlaying( a_strName, 0.0f );
}

// $Barnyard: FUNCTION 0057c9e0
TBOOL AAnimatableObject::IsAnimationPlanned( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = m_oToshiAnimInterface.GetAnim();

	// Is the current animation going to transit to it?
	if ( pAnimRef && m_oToshiAnimInterface.GetTransitionName( pAnimRef ) == a_strName )
		return TTRUE;

	// Is it queued?
	for ( TINT i = 0; i < m_vecQueuedAnims.Size(); i++ )
	{
		if ( m_vecQueuedAnims[ i ] == a_strName )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0057cc40
AAnimationBreakpointCallback* AAnimatableObject::GetAnimationEmitter( TPString8 a_strName )
{
	TINT iNumAnimationSets = m_pObjectType->GetNumAnimationSets();

	for ( TINT i = 0; i < iNumAnimationSets; i++ )
	{
		ANamedAnimationSetRef pAnimationSet  = m_pObjectType->GetAnimationSet( i );
		TINT                  iNumAnimations = pAnimationSet->GetNumNonOverlayAnimations();

		for ( TINT k = 0; k < iNumAnimations; k++ )
		{
			ANamedAnimation*                       pAnimation   = pAnimationSet->GetNonOverlayAnimation( k );
			ANamedAnimation::SortedBreakpointList& rBreakpoints = pAnimation->GetBreakpoints();

			for ( TINT j = 0; j < rBreakpoints.Size(); j++ )
			{
				AAnimationBreakpoint* pBreakpoint = rBreakpoints[ j ];

				if ( pBreakpoint->IsA( &TGetClass( AAnimationBreakpointCallback ) ) )
				{
					AAnimationBreakpointCallback* pCallback = TSTATICCAST( AAnimationBreakpointCallback, pBreakpoint );

					if ( pCallback->GetName() == a_strName )
						return pCallback;
				}
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057b730
TFLOAT AAnimatableObject::GetAnimationProgress( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.GetAnimProgress( pAnimRef );

	pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.GetAnimProgress( pAnimRef );

	return 0.0f;
}

// $Barnyard: FUNCTION 0057b8a0
TBOOL AAnimatableObject::SetAnimationSpeed( const TPString8& a_strName, TFLOAT a_fSpeed )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.SetAnimSpeed( pAnimRef, a_fSpeed );

	pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.SetAnimSpeed( pAnimRef, a_fSpeed );

	return TFALSE;
}

// $Barnyard: FUNCTION 0057b830
TBOOL AAnimatableObject::IsAnimContinuing( const TPString8& a_strName, TFLOAT a_fDeltaTime )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

		if ( !pAnimRef )
			return TFALSE;
	}

	if ( m_oToshiAnimInterface.IsAnimLooping( pAnimRef ) )
		return !m_oToshiAnimInterface.IsAnimLoopEnding( pAnimRef, a_fDeltaTime );

	return !m_oToshiAnimInterface.IsAnimFinishing( pAnimRef, a_fDeltaTime );
}

// $Barnyard: FUNCTION 0057b8e0
TFLOAT AAnimatableObject::GetAnimationSpeed( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.GetAnimSpeed( pAnimRef );

	pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

	if ( pAnimRef )
		return m_oToshiAnimInterface.GetAnimSpeed( pAnimRef );

	return -1.0f;
}

// $Barnyard: FUNCTION 0057b770
TFLOAT AAnimatableObject::GetAnimationDuration( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetAnimRefs(), a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = AToshiAnimationInterface::GetAnimationRef( m_oToshiAnimInterface.GetOverlayAnimRefs(), a_strName );

		if ( !pAnimRef )
			return 0.0f;
	}

	return m_oToshiAnimInterface.GetAnimDuration( pAnimRef );
}

// $Barnyard: FUNCTION 0057c1b0
AAnimatableObject* AAnimatableObject::FindAttachedObject( TINT a_iParentBone )
{
	T2_FOREACH( m_llAttachedObjects, it )
	{
		if ( a_iParentBone == it->m_oAttachmentInfo.iParentBone )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057c300
void AAnimatableObject::SetGlow( TBOOL a_bGlow )
{
	TModel* pModel = m_pModelInstance->GetSceneObject()->GetModelRef()->GetModel();

	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
			rLOD.ppMeshes[ k ]->GetMaterial()->SetFlags( TMaterial::FLAGS_GLOW, a_bGlow );
	}

	m_pModelInstance->SetGlow( a_bGlow );
}
