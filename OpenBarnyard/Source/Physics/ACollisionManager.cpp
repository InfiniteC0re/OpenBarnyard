#include "pch.h"
#include "ACollisionManager.h"
#include "AVehicleManager.h"
#include "ACollisionObjectMesh.h"
#include "ACollisionObjectSimpleDummy.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AVisibleUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitType.h"
#include "Units/AUnitCollisionComponentData.h"
#include "Player/APlayerManager.h"

#include <Math/TMath.h>

#include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00616070, CreateObject 00616610, CreateObjectInPlace 006166b0
TDEFINE_CLASS( ACollisionManager );

// NOTE: retail uses a plain sqrt here (TMath::Sqrt asserts on zero, which is valid for vertical rays)
static TFLOAT MagnitudeXZ( const TVector4& a_rcVector )
{
	return sqrtf( a_rcVector.x * a_rcVector.x + a_rcVector.z * a_rcVector.z );
}

// NOTE: the lists of the rigid bodies and of the vehicles, the contact pool and the settings of the physics are in AVehicleManager

// $Barnyard: FUNCTION 006166b0
ACollisionManager::ACollisionManager()
{
	field2_0x28             = 0;
	m_fVehiclesUpdateTime   = 0.0f;
	m_fVehiclesUpdateRate   = 1.0f / 60.0f;
	m_bEnableVehiclesUpdate = TTRUE;
}

// $Barnyard: FUNCTION 00616740
// $Barnyard: FUNCTION 00616190
ACollisionManager::~ACollisionManager()
{
}

// $Barnyard: FUNCTION 00616140
TBOOL ACollisionManager::Reset()
{
	return BaseClass::Reset();
}

// $Barnyard: FUNCTION 00616080
TBOOL ACollisionManager::OnCreate()
{
	CreateStatic();
	m_fVehiclesUpdateTime = 0.0f;
	m_fVehiclesUpdateRate = 1.0f / 60.0f;

	return BaseClass::OnCreate();
}

// $Barnyard: FUNCTION 00616760
TBOOL ACollisionManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Moves the collision objects with their owners (ACollisionObjectUnit follows its unit)
	T2_FOREACH( m_oCollisionObjects, it )
	{
		// NOTE: retail passes the frame time as the argument
		it->Update( *TREINTERPRETCAST( void**, &a_fDeltaTime ) );
	}

	if ( m_bEnableVehiclesUpdate )
		UpdateVehicles( a_fDeltaTime );

	if ( ms_bDebugDrawUsedBBox )
	{
		ms_bDebugDrawUsedBBox = TFALSE;
		AObjectHashMain::GetSingleton()->Debug_DrawUsedBBox();
	}

	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006160b0
void ACollisionManager::OnDestroy()
{
	DestroyStatic();

	BaseClass::OnDestroy();
}

// $Barnyard: FUNCTION 006160c0
void ACollisionManager::OnActivate()
{
	m_fVehiclesUpdateTime = 0.0f;

	BaseClass::OnActivate();
}

// NOTE: folded with ACollisionManager::OnActivate (006160c0) in retail, the TTask base methods are both empty
void ACollisionManager::OnDeactivate()
{
	m_fVehiclesUpdateTime = 0.0f;

	BaseClass::OnDeactivate();
}

// $Barnyard: FUNCTION 00616290
void ACollisionManager::DestroyObjectHashMain()
{
	if ( AObjectHashMain::IsSingletonCreated() )
		delete AObjectHashMain::GetSingleton();
}

// $Barnyard: FUNCTION 006161b0
AObjectHashMain* ACollisionManager::CreateObjectHashMain( HashType a_eHashType )
{
	TINT   iNumCellsY;
	TINT   iNumCellsX;
	TFLOAT fMaxY;
	TFLOAT fMaxX;
	TFLOAT fMinY;
	TFLOAT fMinX;

	if ( a_eHashType == HashType_UNK4 )
	{
		AObjectHashMain::CreateSingleton( 6500 );

		iNumCellsY = 24;
		iNumCellsX = 24;
		fMaxY      = 1950.0f;
		fMaxX      = 1150.0f;
		fMinY      = -1000.0f;
		fMinX      = -1250.0f;
	}
	else
	{
		AObjectHashMain::CreateSingleton( 6500 );

		iNumCellsY = 18;
		iNumCellsX = 18;
		fMaxY      = 200.0f;
		fMaxX      = 400.0f;
		fMinY      = -200.0f;
		fMinX      = -400.0f;
	}

	AObjectHashMain::GetSingleton()->InitialiseCache( fMinX, fMinY, fMaxX, fMaxY, iNumCellsX, iNumCellsY );

	return AObjectHashMain::GetSingleton();
}

// $Barnyard: FUNCTION 006160d0
void ACollisionManager::UpdateVehicles( TFLOAT a_fDeltaTime )
{
	m_fVehiclesUpdateTime += a_fDeltaTime;

	if ( m_fVehiclesUpdateTime < m_fVehiclesUpdateRate )
		return;

	TINT iNumUpdateSteps = TMath::FloorToInt( m_fVehiclesUpdateTime / m_fVehiclesUpdateRate );

	for ( TINT i = 0; i < iNumUpdateSteps; i++ )
		AVehicleManager::UpdatePhysics( m_fVehiclesUpdateRate );

	m_fVehiclesUpdateTime -= m_fVehiclesUpdateRate * iNumUpdateSteps;
}

// $Barnyard: FUNCTION 0065a2a0
void ACollisionManager::CreateStatic()
{
	TASSERT( ms_pRayHits == TNULL );

	ms_pRayHits = new T2DynamicVector<ACollisionHit>( GetGlobalAllocator(), 64, 64 );

	AVehicleManager::ms_vecBodies.Clear();
	AVehicleManager::ms_vecVehicles.Clear();

	AVehicleManager::ms_iNumUsedContacts   = 0;
	AVehicleManager::ms_iMaxNumContacts    = AVehicleManager::MAX_NUM_CONTACTS;
	AVehicleManager::ms_pContacts          = new ARigidBodyContact[ AVehicleManager::MAX_NUM_CONTACTS ];
	AVehicleManager::ms_bUnk7cd02c         = TFALSE;
	AVehicleManager::ms_bSlideOnObjects    = TFALSE;
	AVehicleManager::ms_bUseTriangleNormal = TFALSE;
	AVehicleManager::ms_iNumIterations     = 1;
}

// $Barnyard: FUNCTION 0065d900
void ACollisionManager::DestroyStatic()
{
	AVehicleManager::ms_iNumUsedContacts = 0;

	TASSERT( AVehicleManager::ms_pContacts != TNULL );
	delete[] AVehicleManager::ms_pContacts;
	AVehicleManager::ms_pContacts = TNULL;

	AVehicleManager::ms_vecBodies.Clear();
	AVehicleManager::ms_vecVehicles.Clear();

	TASSERT( ms_pRayHits != TNULL );
	ms_pRayHits->Clear();
	delete ms_pRayHits;
	ms_pRayHits = TNULL;
}

// $Barnyard: FUNCTION 00616800
TBOOL ACollisionManager::IsObjectCollidable( ACollisionObject* a_pObject, TUINT a_eIgnoreTypes )
{
	if ( ( a_eIgnoreTypes & a_pObject->GetType() ) != 0 )
		return TFALSE;

	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, a_pObject->GetOwnerUnit() );

	// Skip objects of dying, hidden or not queryable units
	if ( pUnit && ( ( pUnit->GetFlags() & AUnit::FLAGS_DYING ) || ( pUnit->GetFlags() & AUnit::FLAGS_NO_QUERY ) || !pUnit->IsVisible() ) )
		return TFALSE;

	if ( !a_pObject->IsA( &TGetClass( ACollisionObjectMesh ) ) )
		return TTRUE;

	return TSTATICCAST( ACollisionObjectMesh, a_pObject )->GetSceneObject()->IsSkeletonUpdateEnabled();
}

// $Barnyard: FUNCTION 006162b0
TBOOL ACollisionManager::CollideRayWithTerrain( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, TFLOAT& a_rfDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	a_rfDistance = a_rcRay.fLength;
	return ATerrainInterface::GetSingleton()->CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, a_rfDistance, TNULL, a_eIgnoreGroups, TFALSE );
}

// $Barnyard: FUNCTION 006162e0
TBOOL ACollisionManager::CollideRayWithTerrain( const ARayData& a_rcRay, TFLOAT* a_pOutDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TFLOAT fDistance = a_rcRay.fLength;

	if ( ATerrainInterface::GetSingleton()->CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, fDistance, TNULL, a_eIgnoreGroups, TFALSE ) )
	{
		*a_pOutDistance = fDistance;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00616ae0
TBOOL ACollisionManager::CollideRay( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, TFLOAT& a_rfDistance, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TUINT a_eIgnoreTypes )
{
	TFLOAT fDistance = a_rcRay.fLength;
	TBOOL  bHit      = ATerrainInterface::GetSingleton()->CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, fDistance, TNULL, a_eIgnoreGroups, TFALSE ) != TFALSE;

	if ( fDistance > 0.0f )
	{
		// The objects only have to be closer than the terrain
		ARayData oRay( a_rcRay.vOrigin, a_rcRay.vDirection, fDistance );

		TFLOAT fRadius = ( MagnitudeXZ( a_rcRay.vDirection ) * 0.5f + 0.001f ) * fDistance;

		TVector4 vecCenter;
		vecCenter.Multiply( a_rcRay.vDirection, fRadius );
		vecCenter.Set( vecCenter.x + a_rcRay.vOrigin.x, vecCenter.y + a_rcRay.vOrigin.y, vecCenter.z + a_rcRay.vOrigin.z, a_rcRay.vOrigin.w );

		TINT                      iNumObjects;
		AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vecCenter, fRadius, &iNumObjects, TFALSE, AObjectHash::MAX_RESULTS );

		for ( TINT i = 0; i < iNumObjects; i++ )
		{
			ACollisionObject* pObject = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

			if ( IsObjectCollidable( pObject, a_eIgnoreTypes ) )
			{
				ACollisionHit oHit;
				oHit.vNormal = TVector4::VEC_ZERO;

				if ( pObject->CollideRay( oRay, oHit ) == TTRUE &&
				     ( oHit.pObject == TNULL || oHit.pObject->GetOwnerUnit() == TNULL || oHit.pObject->GetOwnerUnit() != a_pIgnoreUnit ) )
				{
					oRay.fLength = oHit.fDistance;
					fDistance    = oHit.fDistance;
					bHit         = TTRUE;
				}
			}
		}
	}

	a_rfDistance = fDistance;
	return bHit;
}

// $Barnyard: FUNCTION 00616890
TBOOL ACollisionManager::CollideRayObjects( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, T2DynamicVector<ACollisionHit>* a_pHits, ACollisionHit& a_rClosestHit, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TUINT a_eIgnoreTypes )
{
	a_rClosestHit.fDistance = a_rcRay.fLength;

	TBOOL  bHit    = TFALSE;
	TFLOAT fRadius = ( MagnitudeXZ( a_rcRay.vDirection ) * 0.5f + 0.001f ) * a_rcRay.fLength;

	TVector4 vecCenter;
	vecCenter.Multiply( a_rcRay.vDirection, fRadius );
	vecCenter.Set( vecCenter.x + a_rcRay.vOrigin.x, vecCenter.y + a_rcRay.vOrigin.y, vecCenter.z + a_rcRay.vOrigin.z, a_rcRay.vOrigin.w );

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vecCenter, fRadius, &iNumObjects, TFALSE, AObjectHash::MAX_RESULTS );

	for ( TINT i = 0; i < iNumObjects; i++ )
	{
		ACollisionObject* pObject = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

		if ( ( a_pIgnoreUnit == TNULL || pObject->GetOwnerUnit() != a_pIgnoreUnit ) && IsObjectCollidable( pObject, a_eIgnoreTypes ) )
		{
			ACollisionHit oHit;
			oHit.vNormal = TVector4::VEC_ZERO;

			if ( pObject->CollideRay( a_rcRay, oHit ) == TTRUE )
			{
				bHit = TTRUE;

				if ( oHit.fDistance < a_rClosestHit.fDistance && &a_rClosestHit != &oHit )
				{
					// NOTE: the collision group isn't copied
					a_rClosestHit.fDistance = oHit.fDistance;
					a_rClosestHit.pObject   = oHit.pObject;
					a_rClosestHit.vNormal   = oHit.vNormal;
				}

				a_pHits->PushBack( oHit );
			}
		}
	}

	// Check if the terrain is closer
	TFLOAT fTerrainDistance = a_rcRay.fLength;

	if ( bHit == TTRUE &&
	     ATerrainInterface::GetSingleton()->CollideRay( a_rcRay.vOrigin, a_rcRay.vDirection, fTerrainDistance, TNULL, a_eIgnoreGroups, TFALSE ) == TTRUE &&
	     fTerrainDistance < a_rClosestHit.fDistance )
	{
		return TFALSE;
	}

	return bHit;
}

// $Barnyard: FUNCTION 00616d80
ACollisionObject* ACollisionManager::CollideSweptSphereObjects( ACollisionObject* a_pObject, TVector4& a_rNormal, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TUINT a_eIgnoreTypes, TFLOAT a_fMaxStepHeight, TFLOAT* a_pStepHeight, TINT a_iUnused )
{
	if ( !a_pObject )
		return TNULL;

	TFLOAT   fOriginalDistance = a_rfDistance;
	TFLOAT   fDeepestDistance  = 1.0f;
	TFLOAT   fStepHeight       = -1.0f;
	TVector4 vecDeepestNormal;

	if ( a_pStepHeight )
		fStepHeight = *a_pStepHeight;

	// Circle around the movement on the XZ plane
	TFLOAT   fHalfDistance = a_rfDistance * 0.5f;
	TVector4 vecHalfMovement( a_rcDirection.x, a_rcDirection.y, a_rcDirection.z, a_rcDirection.w );
	TFLOAT   fExtraRadius = MagnitudeXZ( vecHalfMovement ) * fHalfDistance;

	vecHalfMovement.y = 0.0f;
	vecHalfMovement.Multiply( fHalfDistance );

	vecHalfMovement.x = vecHalfMovement.x + a_rcCenter.x;
	vecHalfMovement.y = vecHalfMovement.y + a_rcCenter.y;
	vecHalfMovement.z = vecHalfMovement.z + a_rcCenter.z;

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vecHalfMovement, a_pObject->GetObjectRadius() + fExtraRadius, &iNumObjects, TFALSE, AObjectHash::MAX_RESULTS );

	ACollisionObject* pHitObject = TNULL;

	if ( iNumObjects != 0 )
	{
		for ( TINT i = 0; i < iNumObjects; i++ )
		{
			ACollisionObject* pOther = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

			if ( pOther != a_pObject &&
			     IsObjectCollidable( pOther, a_eIgnoreTypes ) &&
			     pOther->GetClass() != &TGetClass( ACollisionObjectSimpleDummy ) &&
			     pOther->ReciprocateSweptSphere( a_pObject, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, fStepHeight ) == TTRUE )
			{
				pHitObject = pOther;

				// Remember the deepest penetration and restore the distance
				if ( a_rfDistance <= 0.0f )
				{
					if ( a_rfDistance < fDeepestDistance )
					{
						fDeepestDistance = a_rfDistance;
						vecDeepestNormal = a_rNormal;
					}

					a_rfDistance = fOriginalDistance;
				}
			}
		}

		if ( fDeepestDistance <= 0.0f )
		{
			a_rfDistance = fDeepestDistance;
			a_rNormal    = vecDeepestNormal;
		}
	}

	if ( a_pStepHeight && fStepHeight != -1.0f )
		*a_pStepHeight = fStepHeight;

	return pHitObject;
}

// Returns TTRUE if a_pUnit is one of the a_uiNumIgnore units of a_ppIgnore (inlined in the unit queries)
static TBOOL IsUnitIgnored( AUnit* a_pUnit, TUINT a_uiNumIgnore, AUnit* const* a_ppIgnore )
{
	for ( TUINT i = 0; i < a_uiNumIgnore; i++ )
	{
		if ( a_pUnit == a_ppIgnore[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00618440
TBOOL ACollisionManager::CollideRayUnits( const ARayData& a_rcRay, TFLOAT a_fRadius, TRef<AUnitProxy>& a_rHitUnit, ACollisionHit& a_rHit, TUINT a_uiNumIgnore, AUnit* const* a_ppIgnore )
{
	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( a_rcRay.vOrigin, a_fRadius, &iNumFound, TFALSE, 420, TFALSE, TNULL );
	TBOOL                   bHit     = TFALSE;

	for ( TUINT i = 0; i < TUINT( iNumFound ) && !bHit; i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( IsUnitIgnored( pUnit, a_uiNumIgnore, a_ppIgnore ) )
			continue;

		if ( !pUnit->GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) )
			continue;

		if ( !pUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) || !TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			continue;

		ACollisionObject* pObject = TSTATICCAST( AVisibleUnit, pUnit )->GetCollisionObject();

		if ( pObject && pObject->CollideRay( a_rcRay, a_rHit ) )
		{
			bHit       = TTRUE;
			a_rHitUnit = pUnit->GetSelfProxy();
		}
	}

	if ( bHit )
		return TTRUE;

	// Then the player
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( IsUnitIgnored( pPlayer, a_uiNumIgnore, a_ppIgnore ) )
		return TFALSE;

	ACollisionObject* pObject = TSTATICCAST( AVisibleUnit, pPlayer )->GetCollisionObject();

	if ( pObject && pObject->CollideRay( a_rcRay, a_rHit ) )
	{
		a_rHitUnit = pPlayer->GetSelfProxy();
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00618260
TBOOL ACollisionManager::CollideSphereUnits( const TVector4& a_rcCenter, TFLOAT a_fRadius, TRef<AUnitProxy>& a_rHitUnit, TUINT a_uiNumIgnore, AUnit* const* a_ppIgnore )
{
	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( a_rcCenter, a_fRadius * 4.0f, &iNumFound, TFALSE, 420, TFALSE, TNULL );
	TBOOL                   bHit      = TFALSE;

	for ( TUINT i = 0; i < TUINT( iNumFound ) && !bHit; i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( IsUnitIgnored( pUnit, a_uiNumIgnore, a_ppIgnore ) )
			continue;

		if ( !pUnit->GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) )
			continue;

		if ( !pUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) || !TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			continue;

		ACollisionObject* pObject = TSTATICCAST( AVisibleUnit, pUnit )->GetCollisionObject();

		if ( pObject )
		{
			const TVector4& rcPosition = pObject->GetObjectPosition();
			TVector4        vToObject( rcPosition.x - a_rcCenter.x, rcPosition.y - a_rcCenter.y, rcPosition.z - a_rcCenter.z, rcPosition.w );

			if ( vToObject.Magnitude() < a_fRadius + a_fRadius )
			{
				bHit       = TTRUE;
				a_rHitUnit = pUnit->GetSelfProxy();
			}
		}
	}

	return bHit;
}

// $Barnyard: FUNCTION 006187d0
ACollisionObject* ACollisionManager::CollideSweptSphereObjects( const TVector4& a_rcCenter, const TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT& a_rfDistance, TVector4& a_rNormal, TUINT a_eIgnoreTypes, TFLOAT a_fMaxStepHeight, TFLOAT* a_pStepHeight, TINT a_iUnused )
{
	ACollisionObjectSimpleDummy oSphere;
	oSphere.Create( a_rcCenter, a_fRadius, a_rfDistance, 0.0f, 0, TNULL );

	return CollideSweptSphereObjects( &oSphere, a_rNormal, a_rfDistance, a_rcDirection, a_rcCenter, a_eIgnoreTypes, a_fMaxStepHeight, a_pStepHeight, a_iUnused );
}

// $Barnyard: FUNCTION 00616fa0
TUINT ACollisionManager::CollideSweptSphere( const TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TINT a_iUnused, TUINT a_eFlags, ACollisionObject** a_ppHitObject, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TUINT             uiResult   = 0;
	ACollisionObject* pHitObject = CollideSweptSphereObjects( a_pObject, a_rNormal, a_rfDistance, a_rcDirection, a_rcCenter, a_eFlags, a_fMaxStepHeight, &a_rfStepHeight, 0 );

	if ( pHitObject )
	{
		uiResult = pHitObject->GetType();

		if ( a_ppHitObject )
			*a_ppHitObject = pHitObject;
	}

	if ( ( a_eFlags & ACollisionObject::TYPE_TERRAIN ) == 0 && a_rfDistance > 0.0f &&
	     ATerrainInterface::GetSingleton()->CollideSweptSphere( a_rcCenter, a_fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, a_rfPenetration, a_iUnused, a_eIgnoreGroups ) )
	{
		return uiResult | ACollisionObject::TYPE_TERRAIN;
	}

	return uiResult;
}

// $Barnyard: FUNCTION 00617290
TBOOL ACollisionManager::SweepAndSlide( const TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, TVector4& a_rOutPosition, TVector4& a_rOutDirection, TUINT a_eFlags, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bSlopeSlowdown, TBOOL* a_pUnk, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope, TFLOAT a_fMaxSlideNormalY )
{
	// NOTE: retail passes the address of a stack variable as a_iUnused of CollideSweptSphere, it's never read
	a_rfStepHeight = -1.0f;

	if ( a_pUnk )
		*a_pUnk = TFALSE;

	TVector4 vecCenter = a_rcCenter;

	TBOOL bTouchingTerrain = IsSphereTouchingTerrain( a_rcCenter, a_fRadius, a_rcDirection, a_eFlags, TINT( TUINTPTR( a_pUnk ) ), a_eIgnoreGroups );

	TFLOAT   fTotalDistance = a_rfDistance;
	TVector4 vecDirection   = a_rcDirection;
	TFLOAT   fPenetration   = 0.0f;

	ACollisionObject* pHitObject = TNULL;
	TVector4          vecNormal;

	TUINT uiHit = CollideSweptSphere( vecCenter, a_fRadius, a_pObject, vecDirection, a_rfDistance, vecNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, 0, a_eFlags, &pHitObject, a_eIgnoreGroups );

	if ( uiHit == 0 )
	{
		// Nothing was hit, move the whole way
		TVector4 vecMovement;
		vecMovement.Multiply( a_rcDirection, a_rfDistance );

		a_rOutPosition.Set( vecMovement.x + vecCenter.x, vecMovement.y + vecCenter.y, vecMovement.z + vecCenter.z, vecCenter.w );
		return bTouchingTerrain;
	}

	if ( a_rfDistance < 0.0f )
	{
		// The sphere is embedded in something
		if ( uiHit & ACollisionObject::TYPE_SIMPLE )
		{
			// Embedded in an object, push the sphere out along the normal ignoring the object
			TFLOAT            fPushDistance   = 0.05f - a_rfDistance;
			ACollisionObject* pEmbeddedObject = pHitObject;
			pHitObject                        = TNULL;

			a_rfDistance       = fPushDistance;
			TFLOAT fStepHeight = a_rfStepHeight;
			pEmbeddedObject->SetType( pEmbeddedObject->GetType() | ACollisionObject::TYPE_IGNORED );

			TVector4 vecPushDirection = vecNormal;
			vecPushDirection.Normalise();

			CollideSweptSphere( vecCenter, a_fRadius, a_pObject, vecPushDirection, a_rfDistance, vecNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags | ACollisionObject::TYPE_IGNORED, &pHitObject, a_eIgnoreGroups );
			pEmbeddedObject->SetType( pEmbeddedObject->GetType() & ~ACollisionObject::TYPE_IGNORED );

			TVector4 vecStart = vecCenter;
			TVector4 vecTemp;

			if ( a_rfDistance < 0.0f )
			{
				vecTemp.Substract( vecNormal, vecPushDirection );

				if ( vecTemp.IsZero4( 0.001f ) )
				{
					// Embedded in another object in the push direction, push out of that one too
					ACollisionObject* pEmbeddedObject2 = pHitObject;

					if ( pEmbeddedObject2 )
						pEmbeddedObject2->SetType( pEmbeddedObject2->GetType() | ACollisionObject::TYPE_IGNORED );

					TVector4 vecPushDirection2 = vecNormal;
					a_rfDistance               = 0.05f - a_rfDistance;

					CollideSweptSphere( vecCenter, a_fRadius, a_pObject, vecPushDirection2, a_rfDistance, vecNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags | ACollisionObject::TYPE_IGNORED, TNULL, a_eIgnoreGroups );

					if ( pEmbeddedObject2 )
						pEmbeddedObject2->SetType( pEmbeddedObject2->GetType() & ~ACollisionObject::TYPE_IGNORED );

					if ( a_rfDistance <= 0.0f )
					{
						vecTemp.Substract( vecNormal, vecPushDirection2 );

						if ( vecTemp.IsZero4( 0.001f ) )
						{
							// Stuck
							a_rOutPosition = vecCenter;
							a_rfDistance   = 0.0f;
							a_rOutPosition = vecCenter;

							if ( a_pUnk )
								*a_pUnk = TTRUE;

							return TTRUE;
						}
					}

					if ( a_rfDistance < 0.0f )
						a_rfDistance = -a_rfDistance;

					TVector4 vecOffset;
					vecOffset.Multiply( vecPushDirection2, a_rfDistance );
					vecStart.Set( vecCenter.x + vecOffset.x, vecCenter.y + vecOffset.y, vecCenter.z + vecOffset.z, vecCenter.w );

					// Try the first push again from there
					a_rfDistance = fPushDistance;
					CollideSweptSphere( vecStart, a_fRadius, a_pObject, vecPushDirection, a_rfDistance, vecNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags, TNULL, a_eIgnoreGroups );

					if ( a_rfDistance <= 0.0f )
					{
						a_rOutPosition = vecCenter;
						a_rfDistance   = 0.0f;
						a_rOutPosition = vecCenter;

						if ( a_pUnk )
							*a_pUnk = TTRUE;

						return TTRUE;
					}
				}
			}

			if ( a_rfDistance == 0.0f )
			{
				a_rOutPosition = vecCenter;
			}
			else
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecPushDirection, a_rfDistance );

				TVector4 vecPushed( vecStart.x + vecOffset.x, vecStart.y + vecOffset.y, vecStart.z + vecOffset.z, vecStart.w );
				a_rOutPosition = vecPushed;

				// NOTE: compares a value with itself, so the position is never reset here
				if ( a_rOutPosition.y < vecPushed.y )
				{
					a_rOutPosition = vecCenter;
					a_rfDistance   = 0.0f;
				}
			}

			if ( a_pUnk )
				*a_pUnk = TTRUE;

			return TTRUE;
		}

		// Embedded in the terrain, try moving back
		TVector4 vecBack;
		vecBack.Negate3( vecDirection );
		TFLOAT   fBackDistance = -a_rfDistance;
		TVector4 vecBackNormal;

		if ( CollideSweptSphere( vecCenter, a_fRadius, a_pObject, vecBack, fBackDistance, vecBackNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, 0, a_eFlags, TNULL, a_eIgnoreGroups ) )
		{
			if ( fBackDistance >= 0.0f )
			{
				TVector4 vecOffset;
				vecOffset.Multiply( vecBack, fBackDistance );
				a_rOutPosition.Set( vecCenter.x + vecOffset.x, vecCenter.y + vecOffset.y, vecCenter.z + vecOffset.z, vecCenter.w );
			}
			else
			{
				a_rOutPosition = vecCenter;
			}

			return TTRUE;
		}
	}

	//-----------------------------------------------------------------------------
	// Slide along what was hit
	//-----------------------------------------------------------------------------
	TVector4 vecSlideDirection = vecDirection;
	TFLOAT   fSlideDistance    = a_rfDistance;

	TFLOAT fBackOff = TVector4::DotProduct3( vecNormal, vecSlideDirection ) * 0.0001f;
	fBackOff        = ( fBackOff >= 0.0f ) ? 0.0f : fBackOff;

	if ( fSlideDistance < 0.0f )
	{
		fSlideDistance = -fSlideDistance;
		vecSlideDirection.Negate3();
		fTotalDistance = fSlideDistance + fTotalDistance;
	}

	TFLOAT fHitDistance = fSlideDistance;

	TVector4 vecOffset;
	vecOffset.Multiply( vecSlideDirection, fSlideDistance );

	TVector4 vecPosition( vecOffset.x + vecCenter.x, vecOffset.y + vecCenter.y, vecOffset.z + vecCenter.z, vecCenter.w );

	if ( vecNormal.y <= a_fMaxSlideNormalY )
	{
		TMath::SlideAlongPlane( vecDirection, vecNormal, a_fSpeedScale, fTotalDistance, fHitDistance + fBackOff, fSlideDistance, vecSlideDirection, 0.1f, 0.004f, 1.0f );
	}
	else
	{
		// Wall, reflect the direction in the XZ plane
		TVector4 vecReflect;
		vecReflect.Multiply( vecNormal, TVector4::DotProduct3( vecDirection, vecNormal ) * -2.0f );

		vecDirection.x = vecDirection.x + vecReflect.x;
		vecDirection.y = 0.0f;
		vecDirection.z = vecDirection.z + vecReflect.z;
		vecDirection.Normalise();

		fSlideDistance = fTotalDistance - ( fHitDistance + fHitDistance );
	}

	if ( a_bSlopeSlowdown && vecDirection.y > 0.9f )
		ScaleSpeedBySlope( fSlideDistance, vecSlideDirection, a_fMinSlope, a_fMaxSlope );

	fTotalDistance                 = fSlideDistance;
	TVector4 vecPrevSlideDirection = vecSlideDirection;
	TINT     iNumIterations        = 0;
	fHitDistance                   = fSlideDistance;

	while ( fHitDistance > TMath::TFLOAT_EPSILON && fSlideDistance > TMath::TFLOAT_EPSILON && TVector4::DotProduct3( vecPrevSlideDirection, vecSlideDirection ) > TMath::TFLOAT_EPSILON && iNumIterations < 15 )
	{
		TFLOAT            fStepHeight = -1.0f;
		ACollisionObject* pSlideHit   = TNULL;

		TUINT uiSlideHit = CollideSweptSphere( vecPosition, a_fRadius, a_pObject, vecSlideDirection, fHitDistance, vecNormal, a_fMaxStepHeight, fStepHeight, fPenetration, 0, a_eFlags, &pSlideHit, a_eIgnoreGroups );

		if ( uiSlideHit != 0 )
		{
			if ( fStepHeight != -1.0f && fStepHeight < a_rfStepHeight )
				a_rfStepHeight = fStepHeight;

			TVector4 vecMovement;
			vecMovement.Multiply( vecSlideDirection, fHitDistance );
			vecPosition.x = vecMovement.x + vecPosition.x;
			vecPosition.y = vecMovement.y + vecPosition.y;
			vecPosition.z = vecMovement.z + vecPosition.z;

			if ( fHitDistance > 0.0f )
			{
				vecPrevSlideDirection = vecSlideDirection;

				if ( vecNormal.y <= a_fMaxSlideNormalY )
				{
					TMath::SlideAlongPlane( vecDirection, vecNormal, a_fSpeedScale, fTotalDistance, fHitDistance + fBackOff, fSlideDistance, vecSlideDirection, 0.1f, 0.004f, 1.0f );
				}
				else
				{
					TVector4 vecReflect;
					vecReflect.Multiply( vecNormal, TVector4::DotProduct3( vecDirection, vecNormal ) * -2.0f );

					vecDirection.x = vecReflect.x + vecDirection.x;
					vecDirection.y = 0.0f;
					vecDirection.z = vecReflect.z + vecDirection.z;
					vecDirection.Normalise();

					fSlideDistance = fTotalDistance - ( fHitDistance + fHitDistance );
				}

				if ( a_bSlopeSlowdown && vecDirection.y > 0.5f )
					ScaleSpeedBySlope( fSlideDistance, vecSlideDirection, a_fMinSlope, a_fMaxSlope );

				fTotalDistance = fSlideDistance;
				fHitDistance   = fSlideDistance;
			}

			iNumIterations++;
		}
		else
		{
			// Nothing in the way, move the rest of the distance
			TVector4 vecMovement;
			vecMovement.Multiply( vecSlideDirection, fHitDistance );
			vecPosition.x = vecMovement.x + vecPosition.x;
			vecPosition.y = vecMovement.y + vecPosition.y;
			vecPosition.z = vecMovement.z + vecPosition.z;

			iNumIterations++;
			break;
		}
	}

	if ( iNumIterations == 15 )
	{
		// Couldn't resolve the movement
		a_rOutPosition = vecCenter;
		a_rOutDirection.Multiply( -1.0f );
	}
	else
	{
		a_rOutPosition    = vecPosition;
		a_rOutDirection   = vecDirection;
		a_rOutDirection.y = 0.0f;
		a_rOutDirection.Normalise();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00617040
ACollisionObject* ACollisionManager::CollideSphereObjects( const TVector4& a_rcCenter, ACollisionObject* a_pObject, TFLOAT& a_rfDistance, TVector4& a_rNormal, TUINT a_eIgnoreTypes )
{
	if ( !a_pObject )
		return TNULL;

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( a_rcCenter, a_pObject->GetObjectRadius(), &iNumObjects, TFALSE, AObjectHash::MAX_RESULTS );

	ACollisionObject* pHitObject = TNULL;

	for ( TINT i = 0; i < iNumObjects; i++ )
	{
		ACollisionObject* pOther = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

		if ( pOther != a_pObject && IsObjectCollidable( pOther, a_eIgnoreTypes ) && a_pObject->CollideSphere( pOther, a_rcCenter, a_rfDistance, a_rNormal ) == TTRUE )
		{
			a_rNormal.Normalise();
			pHitObject = pOther;
		}
	}

	return pHitObject;
}

// $Barnyard: FUNCTION 006170f0
ACollisionObject* ACollisionManager::CollideShapeObjects( void* a_pShape, ACollisionObject* a_pObject, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, TVector4* a_pNormal, TUINT a_eIgnoreTypes )
{
	if ( !a_pObject )
		return TNULL;

	const TVector4& rcPosition = *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TCHAR, a_pShape ) + 0x30 );
	TFLOAT          fRadius    = *TREINTERPRETCAST( const TFLOAT*, TSTATICCAST( TCHAR, a_pShape ) + 0x50 );

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( rcPosition, fRadius, &iNumObjects, TFALSE, AObjectHash::MAX_RESULTS );

	ACollisionObject* pHitObject = TNULL;

	for ( TINT i = 0; i < iNumObjects; i++ )
	{
		ACollisionObject* pOther = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

		if ( pOther != a_pObject && IsObjectCollidable( pOther, a_eIgnoreTypes ) && a_pObject->CollideShape( pOther, a_pShape, a_pUnk3, a_pUnk4, a_pUnk5, a_pNormal ) == TTRUE )
		{
			a_pNormal->Normalise();
			pHitObject = pOther;
		}
	}

	return pHitObject;
}

// $Barnyard: FUNCTION 00616370
TBOOL ACollisionManager::CollideBoxWithTerrain( const TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	return ATerrainInterface::GetSingleton()->CollideBox( a_rcBox, a_pHitList, -1, a_eIgnoreGroups );
}

// $Barnyard: FUNCTION 006171c0
TBOOL ACollisionManager::CollideSphereWithWorld( const TVector4& a_rcCentre, ACollisionObject* a_pObject, TFLOAT& a_rfDistance, TVector4& a_rNormal, TUINT a_eIgnoreTypes, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TBOOL bHitObjects = ACollisionManager::GetSingleton()->CollideSphereObjects( a_rcCentre, a_pObject, a_rfDistance, a_rNormal, a_eIgnoreTypes ) != TNULL;
	TBOOL bHitTerrain = TFALSE;

	if ( !( a_eIgnoreTypes & ACollisionObject::TYPE_TERRAIN ) )
	{
		TVector4 vSphere = a_rcCentre;
		vSphere.w        = a_pObject->GetObjectRadius();

		TFLOAT fDistance = a_rfDistance;
		bHitTerrain      = ATerrainInterface::GetSingleton()->CollideSphere( vSphere, fDistance, a_rNormal, -1, a_eIgnoreGroups );

		if ( a_pObject->GetObjectRadius() > fDistance )
			a_rfDistance = fDistance;
	}

	return bHitObjects || bHitTerrain;
}

// $Barnyard: FUNCTION 00616420
TBOOL ACollisionManager::IsSphereTouchingTerrain( const TVector4& a_rcCenter, TFLOAT a_fRadius, const TVector4& a_rcDirection, TUINT a_eFlags, TINT a_iUnused, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	if ( a_eFlags & ACollisionObject::TYPE_TERRAIN )
		return TFALSE;

	TVector4 vecCenter = a_rcCenter;
	TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, a_fRadius );

	ACollisionModelRayHitList oHitList;
	TBOOL                     bHit = ATerrainInterface::GetSingleton()->CollideSphere( vecSphere, &oHitList, -1, a_eIgnoreGroups );

	if ( bHit != TTRUE )
		return bHit;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		// NOTE: retail subtracts the center from its own copy, so this is always ( 0, 0, 1 ) after normalising
		TVector4 vecToHit( vecCenter.x - a_rcCenter.x, vecCenter.y - a_rcCenter.y, vecCenter.z - a_rcCenter.z, vecCenter.w );
		vecToHit.Normalise();

		// NOTE: the normal of the hit triangle is calculated but not used
		oHitList.aHits[ i ].GetNormal();

		if ( TVector4::DotProduct3( a_rcDirection, vecToHit ) < 0.0f )
			return bHit;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00616390
void ACollisionManager::ScaleSpeedBySlope( TFLOAT& a_rfSpeed, const TVector4& a_rcNormal, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope )
{
	if ( a_rcNormal.y > a_fMinSlope )
	{
		if ( a_rcNormal.y > a_fMaxSlope )
		{
			TFLOAT fScale = TMath::Pow( a_rcNormal.y - a_fMaxSlope, 0.4f ) * ( 1.0f / ( 1.0f - a_fMaxSlope ) );
			a_rfSpeed     = a_rfSpeed * TMath::Min( fScale, 1.0f );
		}
		else
		{
			a_rfSpeed = 0.0f;
		}
	}
}

// $Barnyard: FUNCTION 00616320
TBOOL ACollisionManager::PushOutOfTerrain( const TVector4& a_rcCenter, TFLOAT a_fRadius, TVector4& a_rOutPosition, TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups )
{
	TVector4 vecSphere( a_rcCenter.x, a_rcCenter.y, a_rcCenter.z, a_fRadius );
	return ATerrainInterface::GetSingleton()->PushOutSphere( vecSphere, a_rOutPosition, -1, a_eIgnoreGroups );
}
