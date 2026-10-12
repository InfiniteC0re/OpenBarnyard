#include "pch.h"
#include "AAnimatedUnit.h"
#include "AUnitType.h"
#include "AUnitTransform.h"
#include "AUnitModelStaticData.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Cameras/ACamera.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "FX/AAniEffectManager.h"

#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers it with size 0x98 and alignment 1
TDEFINE_CLASS_INIT_NORUNTIME( AAnimatedUnit, 1, 1 );

AFunctionTable AAnimatedUnit::ms_oFunctionTable( &TGetClass( AAnimatedUnit ), &AVisibleUnit::ms_oFunctionTable );

// $Barnyard: FUNCTION 00416530
void AAnimatedUnit::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	// NOTE: SetVisible, AddUnitToParent and RemoveFromParentAttachedUnit are virtual, so
	// retail binds the compiler's vcall thunks (00415b60 is +0xb4, 00415b50 is +0xd0)
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimatedUnit, TBOOL>::Create( "SetVisible", &AAnimatedUnit::SetVisible ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AAnimatedUnit, AUnit*, const TPString8&>::Create( "AddUnitToParent", &AAnimatedUnit::AddUnitToParent ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AAnimatedUnit>::Create( "RemoveFromParentAttachedUnit", &AAnimatedUnit::RemoveFromParentAttachedUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AAnimatedUnit, AUnit*>::Create( "IsAttachedToParent", &AAnimatedUnit::IsAttachedToParent ) );
}

// $Barnyard: FUNCTION 00415a60
void AAnimatedUnit::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00415a70
AAnimatedUnit::AAnimatedUnit()
    : m_pAnimatableObject( TNULL )
{
}

// NOTE: 00534e30 is the scalar deleting destructor folded between AAnimatedUnit subclasses with an empty destructor
// (APrecisionScarecrow, APrecisionTargetItem, ACoinPickupCoinItem, ...); it only calls this destructor
// $Barnyard: FUNCTION 00415b80
// $Barnyard: FUNCTION 00415aa0
// $Barnyard: FUNCTION 00534e30
AAnimatedUnit::~AAnimatedUnit()
{
	// NOTE: m_pAnimatableObject is destroyed in OnDestroy
}

// $Barnyard: FUNCTION 00414370
AFunctionTable* AAnimatedUnit::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00416900
void AAnimatedUnit::SetSize( TFLOAT a_fSize )
{
	m_pAnimatableObject->GetModelInstance()->SetScale( TVector4( a_fSize, a_fSize, a_fSize, 1.0f ) );
}

// $Barnyard: FUNCTION 00416820
TBOOL AAnimatedUnit::Create( AUnitTransform* a_pTransform )
{
	if ( !CreateAnimatableObject( a_pTransform->m_iModelIndex ) )
		return TFALSE;

	m_eFlags |= FLAGS_EXTERNAL_UPDATE;
	return AVisibleUnit::Create( a_pTransform );
}

// $Barnyard: FUNCTION 00416670
TBOOL AAnimatedUnit::CreateAnimatableObject( TINT a_iModelIndex )
{
	// NOTE: retail also constructs (and never uses) an empty TString8 here
	TPString8 strModelName;
	strModelName = GetModelName( a_iModelIndex );

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pAnimObjType    = pAnimObjManager->FindType( strModelName );

	if ( pAnimObjType )
		m_pAnimatableObject = pAnimObjManager->CreateAnimatableObject( pAnimObjType, this, 0 );
	else
		m_pAnimatableObject = TNULL;

	if ( m_pAnimatableObject )
	{
		m_pAnimatableObject->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pAnimatableObject->GetModelInstance()->GetSceneObject()->DisableOpacity();

		TSceneObject* pSceneObject         = m_pAnimatableObject->GetModelInstance()->GetSceneObject();
		pSceneObject->GetFadeDistances().x = 50.0f;
		pSceneObject->GetFadeDistances().y = 50.1f;
		pSceneObject->GetFadeParams().x    = 10.000153f;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004165c0
TPString8 AAnimatedUnit::GetModelName( TINT a_iModelIndex )
{
	AUnitModelStaticData* pModelData = TSTATICCAST( AUnitModelStaticData, m_pType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) );

	TINT iIndex = ( a_iModelIndex < 0 ) ? 0 : a_iModelIndex % pModelData->GetNumModels();
	return pModelData->GetModelName( iIndex );
}

// $Barnyard: FUNCTION 00415ba0
void AAnimatedUnit::OnDestroy()
{
	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pAnimatableObject );
	m_pAnimatableObject = TNULL;

	AVisibleUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00415ae0
void AAnimatedUnit::UpdateComponents( TFLOAT a_fDeltaTime )
{
	// NOTE: retail calls an empty AAnimatableObject method here first:
	// m_pAnimatableObject->0054df00( a_fDeltaTime, this ), a folded DOES_NOTHING stub (in the ignorelist)
	AVisibleUnit::UpdateComponents( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00415ac0
void AAnimatedUnit::SetVisible( TBOOL a_bVisible )
{
	// NOTE: AAnimatableObject::SetSkeletonUpdating toggles AModelInstance flag 1, which
	// is what IsVisible reads back
	m_pAnimatableObject->SetSkeletonUpdating( a_bVisible, TTRUE );
}

// $Barnyard: FUNCTION 004164e0
TBOOL AAnimatedUnit::IsVisible()
{
	// NOTE: retail reads the model instance pointer without taking a reference
	AModelInstanceRef pModelInstance = m_pAnimatableObject->GetModelInstance();

	if ( pModelInstance )
		return pModelInstance->IsUpdatingSkeleton();

	return TFALSE;
}

// $Barnyard: FUNCTION 00416040
void AAnimatedUnit::SetOpacity( TFLOAT a_fOpacity )
{
	m_pAnimatableObject->SetOpacity( a_fOpacity, TTRUE );
}

// $Barnyard: FUNCTION 00416850
void AAnimatedUnit::UpdateBoundingSphere()
{
	TVector4 vecPosition;
	m_pSteer->CopyPosition( vecPosition );

	TFLOAT fRadius = m_pAnimatableObject->GetModelInstance()->GetModel()->GetManagedModel()->GetModel()->GetRenderDistance();
	m_oBoundingSphere.Set( vecPosition.x, vecPosition.y, vecPosition.z, fRadius );
}

// $Barnyard: FUNCTION 00416a10
AModelInstance* AAnimatedUnit::GetModelInstance()
{
	// NOTE: the animatable object keeps its own reference, the temporary one is released here
	return m_pAnimatableObject->GetModelInstance().Get();
}

// $Barnyard: FUNCTION 00415bd0
void AAnimatedUnit::AddUnitToParent( AUnit* a_pParent, const TPString8& a_rcParentBone )
{
	TSTATICCAST( AAnimatedUnit, a_pParent )->AttachUnit( this, a_rcParentBone );
}

// $Barnyard: FUNCTION 00415c50
void AAnimatedUnit::AttachUnitWithOffset( AUnit* a_pUnit, const TPString8& a_rcParentBone, const TPString8& a_rcUnitBone, const TVector4& a_rcOffset, TFLOAT a_fUnk )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	AAnimatableObjectManager::GetSingleton()->AttachObject( pUnit->m_pAnimatableObject, m_pAnimatableObject, a_rcUnitBone, a_rcParentBone, a_rcOffset, TQuaternion::IDENTITY, a_fUnk );

	pUnit->SetVisible( TTRUE );

	if ( AAniEffectManager::GetSingleton() )
		AAniEffectManager::GetSingleton()->RemoveUnitEffects( pUnit );
}

// $Barnyard: FUNCTION 00415bf0
void AAnimatedUnit::AttachUnitToBone( AUnit* a_pUnit, const TPString8& a_rcParentBone, const TPString8& a_rcUnitBone )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	AAnimatableObjectManager::GetSingleton()->AttachObject( pUnit->m_pAnimatableObject, m_pAnimatableObject, a_rcUnitBone, a_rcParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	pUnit->SetVisible( TTRUE );

	if ( AAniEffectManager::GetSingleton() )
		AAniEffectManager::GetSingleton()->RemoveUnitEffects( pUnit );
}

// $Barnyard: FUNCTION 00416950
void AAnimatedUnit::AttachUnit( AUnit* a_pUnit, const TPString8& a_rcParentBone )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	AAnimatableObjectManager::GetSingleton()->AttachObject( pUnit->m_pAnimatableObject, m_pAnimatableObject, TPString8(), a_rcParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

	pUnit->SetVisible( TTRUE );

	if ( AAniEffectManager::GetSingleton() )
		AAniEffectManager::GetSingleton()->RemoveUnitEffects( pUnit );
}

// $Barnyard: FUNCTION 00415d40
AUnit* AAnimatedUnit::GetAttachedUnitByBoneId( TINT a_iBone, TINT a_iIndex )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( a_iIndex >= 0 )
	{
		AAnimatableObject* pAttached = pAnimObjManager->GetAttachedObject( m_pAnimatableObject, a_iBone, a_iIndex );
		return pAttached ? TSTATICCAST( AUnit, pAttached->GetOwner() ) : TNULL;
	}

	TINT iNumAttached = pAnimObjManager->GetNumAttachedObjects( m_pAnimatableObject, a_iBone );

	for ( TINT i = 0; i < iNumAttached; i++ )
	{
		AAnimatableObject* pAttached = pAnimObjManager->GetAttachedObject( m_pAnimatableObject, a_iBone, i );

		if ( pAttached->GetOwner() )
			return TSTATICCAST( AUnit, pAttached->GetOwner() );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00415cb0
AUnit* AAnimatedUnit::GetAttachedUnit( const TPString8& a_rcBone, TINT a_iIndex )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	if ( a_iIndex >= 0 )
	{
		AAnimatableObject* pAttached = pAnimObjManager->GetAttachedObject( m_pAnimatableObject, a_rcBone, a_iIndex );
		return pAttached ? TSTATICCAST( AUnit, pAttached->GetOwner() ) : TNULL;
	}

	TINT iNumAttached = pAnimObjManager->GetNumAttachedObjects( m_pAnimatableObject, a_rcBone );

	for ( TINT i = 0; i < iNumAttached; i++ )
	{
		AAnimatableObject* pAttached = pAnimObjManager->GetAttachedObjectByBoneName( m_pAnimatableObject, a_rcBone, i );

		if ( pAttached->GetOwner() )
			return TSTATICCAST( AUnit, pAttached->GetOwner() );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00415dd0
void AAnimatedUnit::DetachUnit( AUnit* a_pUnit, TBOOL a_bReactivate, TBOOL a_bKeepUpright )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	pUnit->m_pAnimatableObject->UpdateAttachmentTransform();

	TMatrix44 oBoneTransform;
	pUnit->GetModelInstance()->GetBoneTransform( 0, oBoneTransform );

	TVector3 vecPosition( oBoneTransform.m_f41, oBoneTransform.m_f42, oBoneTransform.m_f43 );
	TVector4 vecDirection( -oBoneTransform.m_f21, -oBoneTransform.m_f23, -oBoneTransform.m_f22, 1.0f );

	if ( a_bKeepUpright )
		vecDirection.y = 0.0f;

	vecDirection.Normalise();
	pUnit->m_pSteer->SetPosition3( vecPosition );

	TVector4 vecDown;
	vecDown.Multiply( ACamera::sm_vWorldUp, -1.0f );

	// Can't face straight up or down
	if ( !vecDirection.IsEqual( vecDown ) && !vecDirection.IsEqual( ACamera::sm_vWorldUp ) )
		pUnit->m_pSteer->SetDirection( vecDirection );

	AAnimatableObjectManager::GetSingleton()->DetachObject( pUnit->m_pAnimatableObject, m_pAnimatableObject );

	if ( a_bReactivate )
	{
		pUnit->SetMovement( TTRUE );
		pUnit->SetAIEnabled( TTRUE );
		pUnit->SetHashed( TTRUE );
		pUnit->SetVisible( TTRUE );
		pUnit->SetNoQuery( TFALSE );
		pUnit->m_pAnimatableObject->SetOpacity( 1.0f, TTRUE );
		pUnit->m_pAnimatableObject->SetAnimControllersDisabled( TFALSE );
	}
}

// $Barnyard: FUNCTION 00415f70
void AAnimatedUnit::RemoveFromParentAttachedUnit()
{
	AAnimatableObject* pParent = m_pAnimatableObject->GetParentObject();

	if ( pParent )
		AAnimatableObjectManager::GetSingleton()->DetachObject( m_pAnimatableObject, pParent );
}

// $Barnyard: FUNCTION 00415ff0
TBOOL AAnimatedUnit::IsAttachedToParent( AUnit* a_pParent )
{
	return m_pAnimatableObject->GetParentObject() == TSTATICCAST( AAnimatedUnit, a_pParent )->m_pAnimatableObject;
}

// $Barnyard: FUNCTION 00415f90
void AAnimatedUnit::DetachObjectsAtBone( const TPString8& a_rcBone )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObject*        pAnimObj        = m_pAnimatableObject;

	for ( AAnimatableObject* pAttached = pAnimObj->FindAttachedObject( a_rcBone ); pAttached; pAttached = pAnimObj->FindAttachedObject( a_rcBone ) )
	{
		pAnimObjManager->DetachObject( pAttached, pAnimObj );
		pAttached->SetSkeletonUpdating( TFALSE, TTRUE );
	}
}

// $Barnyard: FUNCTION 00415b10
TBOOL AAnimatedUnit::ApplyMaterial( TINT a_iSkin, TBOOL a_bSkinLighting )
{
	if ( !m_pAnimatableObject )
		return TFALSE;

	return m_pAnimatableObject->ApplyMaterial( a_iSkin, a_bSkinLighting );
}

// $Barnyard: FUNCTION 00415fe0
TINT AAnimatedUnit::GetNumSkins()
{
	return m_pAnimatableObject->GetObjectType()->GetNumSkins();
}

// $Barnyard: FUNCTION 00415b40
AAnimatableObjectType* AAnimatedUnit::GetAnimatableObjectType()
{
	return m_pAnimatableObject->GetObjectType();
}

// $Barnyard: FUNCTION 00416010
void AAnimatedUnit::ReplaceAnimatableObject( AAnimatableObject* a_pAnimatableObject )
{
	if ( m_pAnimatableObject )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pAnimatableObject );
		m_pAnimatableObject = a_pAnimatableObject;
	}
}
