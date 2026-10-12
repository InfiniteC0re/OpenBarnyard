#include "pch.h"
#include "ACarryableUnitHelper.h"
#include "AObjectTraits.h"
#include "AAnimatedUnit.h"
#include "AUnitType.h"
#include "ARandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Helpers/AGameTimeManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( handle );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( putaway );

// $Barnyard: FUNCTION 0058bc80
ACarryableUnitHelper::ACarryableUnitHelper( AUnit* a_pUnit )
    : m_eFlags( FLAGS_NONE )
    , m_pHolder( TNULL )
    , m_pUnit( a_pUnit )
    , m_fDelayEndTime( 0.0f )
{
}

// $Barnyard: FUNCTION 0058bfd0
// $Barnyard: FUNCTION 0058c530
ACarryableUnitHelper::~ACarryableUnitHelper()
{
	DestroyFreeAttachedObjects();
}

// $Barnyard: FUNCTION 0058bb60
TBOOL ACarryableUnitHelper::Use()
{
	AObjectTraits::UseTrigger* pWhenUse = GetObjectTraits()->m_pWhenUse;

	if ( pWhenUse )
		return pWhenUse->Execute( m_pHolder, m_pUnit );

	return TFALSE;
}

// $Barnyard: FUNCTION 0058bbd0
TBOOL ACarryableUnitHelper::StartThrow()
{
	if ( !GetObjectTraits()->m_pWhenThrow )
		return TFALSE;

	AAnimatableObject* pAnimObject = TSTATICCAST( AAnimatedUnit, m_pHolder )->GetAnimatableObject();
	ASteer*            pSteer      = m_pHolder->GetSteer();

	if ( !pSteer )
		return TFALSE;

	if ( !pSteer->GetClass()->IsA( &TGetClass( APlayerSteer ) ) || !pAnimObject->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
		return TFALSE;

	TBOOL bResult = TSTATICCAST( APlayerSteer, pSteer )->SetState( APlayerSteer::STATE_THROW );
	Throw();

	return bResult;
}

// $Barnyard: FUNCTION 0058bcb0
void ACarryableUnitHelper::Throw()
{
	m_eFlags |= FLAGS_THROWN;

	AObjectTraits::ThrowTrigger* pWhenThrow = GetObjectTraits()->m_pWhenThrow;

	if ( pWhenThrow )
		pWhenThrow->Execute( m_pHolder, m_pUnit );
}

// $Barnyard: FUNCTION 0058bce0
TBOOL ACarryableUnitHelper::Equip()
{
	m_eFlags &= ~FLAGS_THROWN;

	AObjectTraits::EquipTrigger* pWhenEquip = GetObjectTraits()->m_pWhenEquip;

	if ( pWhenEquip )
		return pWhenEquip->Execute( m_pHolder, m_pUnit );

	return TFALSE;
}

// $Barnyard: FUNCTION 0058bfe0
TBOOL ACarryableUnitHelper::Carry( TBOOL a_bAttachUnit, TINT a_iUnused )
{
	TINT           iMaxCarry = GetObjectTraits()->m_iMaxCarry;
	AAnimatedUnit* pUnit     = TSTATICCAST( AAnimatedUnit, m_pUnit );
	AAnimatedUnit* pHolder   = TSTATICCAST( AAnimatedUnit, m_pHolder );

	if ( GetObjectTraits()->IsHeavy() )
	{
		// Heavy objects are held by their handle in front of the holder
		TFLOAT fSize = pUnit->GetSteer()->GetRadius();

		if ( a_bAttachUnit )
		{
			TPString8 strUnitBone;

			if ( pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( handle ) ) != -1 )
				strUnitBone = TPS8( handle );

			TFLOAT         fHoldScale = GetObjectTraits()->m_fHoldScale;
			AObjectTraits* pTraits    = GetObjectTraits();

			TVector4 vecOffset;
			vecOffset.Multiply( TVector4::VEC_POSZ, fSize );

			pHolder->AttachUnitWithOffset( pUnit, TPS8( cb_rlocator ), strUnitBone, vecOffset + pTraits->m_vecHoldOffset, fHoldScale );
		}
	}
	else
	{
		AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

		TINT iNumCarried = pAnimObjManager->GetNumAttachedObjectsRecursive( pHolder->GetAnimatableObject(), TPS8( cb_rlocator ) );
		DestroyFreeAttachedObjects();

		TPString8 strUnitBone;

		if ( pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( handle ) ) != -1 )
			strUnitBone = TPS8( handle );

		pHolder->AttachUnitWithOffset( pUnit, TPS8( cb_rlocator ), strUnitBone, TVector4::VEC_ZERO, GetObjectTraits()->m_fHoldScale );

		// Copies of the unit make the bunch the holder is carrying
		TINT iNumToCreate = iNumCarried;

		if ( a_bAttachUnit )
			iNumToCreate = iNumCarried - 1;

		TFLOAT fSpread = TFLOAT( iNumCarried + iNumToCreate ) * 0.05f;

		while ( iNumToCreate > 0 )
		{
			if ( m_vecAttachedObjects.Size() >= MAX_NUM_ATTACHED_OBJECTS ) break;
			if ( iMaxCarry <= pAnimObjManager->GetNumAttachedObjects( pHolder->GetAnimatableObject(), TPS8( cb_rlocator ) ) ) break;

			iNumToCreate--;

			AAnimatableObject* pObject = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( pUnit->GetType()->GetName() ), TNULL, 0 );
			m_vecAttachedObjects.PushBack( pObject );

			TFLOAT      fMinTilt = -0.1f - fSpread;
			TQuaternion quatRotation( TQuaternion::IDENTITY );
			TFLOAT      fMaxTilt = fSpread + 0.1f;

			TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
			quatRotation.RotateAroundAxis( TVector4::VEC_NEGX.AsVector3(), rRandom.GetFloatMinMax( fMinTilt, fMaxTilt ) );
			quatRotation.RotateAroundAxis( TVector4::VEC_NEGY.AsVector3(), rRandom.GetFloatMinMax( fMinTilt, fMaxTilt ) );
			quatRotation.RotateAroundAxis( TVector4::VEC_NEGZ.AsVector3(), rRandom.GetFloatMinMax( -TMath::PI, TMath::PI ) );

			pAnimObjManager->AttachObject( pObject, pUnit->GetAnimatableObject(), 0, 0, TVector4::VEC_ZERO, quatRotation, 1.0f );
			pObject->SetOpacity( 1.0f, TTRUE );
			pObject->SetSkeletonUpdating( TTRUE, TTRUE );
			pObject->SetAnimControllersDisabled( TTRUE );
		}
	}

	AObjectTraits::EquipTrigger* pWhenEquip = GetObjectTraits()->m_pWhenEquip;

	if ( pWhenEquip )
		pWhenEquip->Execute( m_pHolder, m_pUnit );

	return TTRUE;
}

// $Barnyard: FUNCTION 0058bb90
TBOOL ACarryableUnitHelper::UnEquip()
{
	if ( GetObjectTraits()->m_pWhenEquip )
		return AObjectTraits::EquipTrigger::UnEquip( m_pHolder, m_pUnit );

	return TFALSE;
}

// $Barnyard: FUNCTION 0058be20
TBOOL ACarryableUnitHelper::PutAway()
{
	AAnimatedUnit* pHolder = TSTATICCAST( AAnimatedUnit, m_pHolder );

	if ( !pHolder->GetAnimatableObject()->IsAnimationPlanned( TPS8( putaway ) ) && !pHolder->GetAnimatableObject()->IsAnimPlaying( TPS8( putaway ), 0.0f ) )
		DetachFromHolder();

	if ( GetObjectTraits()->m_pWhenEquip )
		AObjectTraits::EquipTrigger::UnEquip( m_pHolder, m_pUnit );

	return TTRUE;
}

// $Barnyard: FUNCTION 0058be80
TBOOL ACarryableUnitHelper::Update( TFLOAT a_fDeltaTime )
{
	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_pUnit );

	if ( !GetObjectTraits()->m_pWhenBreak ) return TFALSE;
	if ( !( m_eFlags & FLAGS_THROWN ) ) return TFALSE;
	if ( !( GetObjectTraits()->m_pWhenBreak->m_eFlags & AObjectTraits::BreakTrigger::FLAGS_BREAK_WHEN_THROWN ) ) return TFALSE;
	if ( !pUnit->IsVisible() ) return TFALSE;
	if ( m_eFlags & FLAGS_BROKEN ) return TFALSE;

	ASteer*  pSteer = pUnit->GetSteer();
	TVector4 vecPosition( *pSteer->GetPosition() );

	TFLOAT fGroundHeight = ATerrainInterface::GetSingleton()->GetGroundHeight( vecPosition.x, vecPosition.y, vecPosition.z, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	// NOTE: Y axis points down
	if ( pSteer->GetRadius() + 0.02f + vecPosition.y <= fGroundHeight )
		return TFALSE;

	m_eFlags |= FLAGS_BROKEN;
	return GetObjectTraits()->m_pWhenBreak->Execute( m_pHolder, pUnit );
}

// $Barnyard: FUNCTION 0058bf60
AObjectTraits* ACarryableUnitHelper::GetObjectTraits()
{
	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits )
		return pTraits;

	// Units without traits share the default ones
	static AObjectTraits s_oDefaultTraits;
	return &s_oDefaultTraits;
}

// $Barnyard: FUNCTION 0058bbc0
TINT ACarryableUnitHelper::GetMaxCarry()
{
	return GetObjectTraits()->m_iMaxCarry;
}

// $Barnyard: FUNCTION 0058bde0
void ACarryableUnitHelper::DetachFromHolder()
{
	AAnimatedUnit* pHolder = TSTATICCAST( AAnimatedUnit, m_pHolder );
	AUnit*         pUnit   = m_pUnit;

	if ( pHolder->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == pUnit )
	{
		pHolder->DetachUnit( pUnit, TTRUE, TFALSE );
		DestroyAttachedObjects();
	}
}

// $Barnyard: FUNCTION 0058bd70
void ACarryableUnitHelper::DestroyAttachedObjects()
{
	for ( TINT i = m_vecAttachedObjects.Size() - 1; i >= 0; i-- )
	{
		AAnimatableObject* pObject = m_vecAttachedObjects[ i ];

		if ( pObject->GetOwner() == m_pUnit )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( pObject );
			m_vecAttachedObjects.Erase( Toshi::T2Vector<AAnimatableObject*, MAX_NUM_ATTACHED_OBJECTS>::Iterator( i, &m_vecAttachedObjects ) );
		}
	}

	DestroyFreeAttachedObjects();
}

// $Barnyard: FUNCTION 0058bd10
void ACarryableUnitHelper::DestroyFreeAttachedObjects()
{
	for ( TINT i = m_vecAttachedObjects.Size() - 1; i >= 0; i-- )
	{
		AAnimatableObject* pObject = m_vecAttachedObjects[ i ];

		if ( !pObject->GetOwner() )
		{
			AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( pObject );
			m_vecAttachedObjects.Erase( Toshi::T2Vector<AAnimatableObject*, MAX_NUM_ATTACHED_OBJECTS>::Iterator( i, &m_vecAttachedObjects ) );
		}
	}
}

// $Barnyard: FUNCTION 0058bc40
void ACarryableUnitHelper::SetDelay( TFLOAT a_fTime )
{
	m_fDelayEndTime = a_fTime + AGameTimeManager::GetSingleton()->GetTotalTime();
}

// $Barnyard: FUNCTION 0058bc60
TBOOL ACarryableUnitHelper::IsDelayOver()
{
	return AGameTimeManager::GetSingleton()->GetTotalTime() > m_fDelayEndTime;
}
