#include "pch.h"
#include "ASimpleProjectileManager.h"
#include "Helpers/AHighScoreManager.h"
#include "AProjectile.h"
#include "AProjectileWithUnit.h"
#include "ASimMilkProjectile.h"
#include "AVeggieDefenderMilkProjectile.h"
#include "AVeggieThrowProjectile.h"
#include "AChasingChicksMilkProjectile.h"
#include "AChasingChicksThrowProjectile.h"
#include "AMilkPrecisionProjectile.h"
#include "AChickenCoopDefenderMilkProjectile.h"
#include "AChickenCoopDefenderThrowProjectile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0066cbf0, CreateObject 0066d510, CreateObjectInPlace 0066d570
TDEFINE_CLASS_INIT( ASimpleProjectileManager );

// DAT_007cd370
AFunctionTable ASimpleProjectileManager::ms_oFunctionTable( &TGetClass( ASimpleProjectileManager ), TNULL );

// FSM fallback: natives of ASimpleProjectileManager are always called on the singleton.
// $Barnyard: FUNCTION 0066d170
static TObject* __stdcall ASimpleProjectileManagerFunctionFallback( void* a_pUnk1, const AFunctionCaller* a_pCaller )
{
	return ASimpleProjectileManager::GetSingleton();
}

// $Barnyard: FUNCTION 0066d240
void ASimpleProjectileManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ASimpleProjectileManagerFunctionFallback ) );
}

// $Barnyard: FUNCTION 0066cc00
void ASimpleProjectileManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0066d180
ASimpleProjectileManager::ASimpleProjectileManager()
{
	for ( TINT i = NUM_PROJECTILE_TYPES; i != 0; i-- )
	{
		m_aiNumUsed[ i - 1 ]   = 0;
		m_aiPoolSizes[ i - 1 ] = 0;
		m_apPools[ i - 1 ]     = TNULL;
	}

	m_bNewRecord      = TFALSE;
	m_bTracking       = TFALSE;
	m_fRecordDistance = 40.0f;
	m_iTrackedRecord  = -1;
}

// $Barnyard: FUNCTION 0066d740
// $Barnyard: FUNCTION 0066d5b0
ASimpleProjectileManager::~ASimpleProjectileManager()
{
	DestroyAllPools();
}

// $Barnyard: FUNCTION 0066d630
TBOOL ASimpleProjectileManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TBOOL bHasNewRecord = TFALSE;

	for ( TINT iType = NUM_PROJECTILE_TYPES - 1; iType >= 0; iType-- )
	{
		for ( TINT i = m_aiNumUsed[ iType ]; i != 0; )
		{
			AProjectile* pProjectile = m_apPools[ iType ][ --i ];

			if ( pProjectile->GetPoolIndex() == -1 )
				continue;

			pProjectile->Update( a_fDeltaTime );

			if ( m_bTracking && m_apPools[ iType ][ i ]->GetAge() > m_fTrackedAge )
			{
				m_fTrackedAge      = m_apPools[ iType ][ i ]->GetAge();
				m_vTrackedVelocity = m_apPools[ iType ][ i ]->GetVelocity();
				m_vTrackedPosition = m_apPools[ iType ][ i ]->GetPosition();
				bHasNewRecord      = TTRUE;
			}
		}
	}

	if ( m_bTracking )
		UpdateTracking( a_fDeltaTime, bHasNewRecord );

	return TTRUE;
}

// $Barnyard: FUNCTION 0066d230
AFunctionTable* ASimpleProjectileManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0066d320
// $Barnyard: FUNCTION 0066d2d0
void ASimpleProjectileManager::DestroyAllPools()
{
	for ( TINT iType = NUM_PROJECTILE_TYPES; iType != 0; )
		DestroyPool( --iType );

	m_bTracking  = TFALSE;
	m_bNewRecord = TFALSE;

	m_oHUDElement1.SetVisible( TFALSE );
	m_oStarRating.SetVisible( TFALSE );
}

// $Barnyard: FUNCTION 0066d4e0
TBOOL ASimpleProjectileManager::CreateDefaultPools()
{
	TBOOL bCreated1 = CreatePool( 10, TYPE_WITH_UNIT );
	TBOOL bCreated2 = CreatePool( 25, TYPE_SIM_MILK );

	return bCreated1 && bCreated2;
}

// $Barnyard: FUNCTION 0066d250
TBOOL ASimpleProjectileManager::CreatePool( TINT a_iSize, TINT a_iType )
{
	if ( m_apPools[ a_iType ] )
		DestroyPool( a_iType );

	m_apPools[ a_iType ]     = new AProjectile*[ a_iSize ];
	m_aiPoolSizes[ a_iType ] = a_iSize;
	m_aiNumUsed[ a_iType ]   = 0;

	for ( TINT i = a_iSize; i != 0; )
	{
		i--;
		m_apPools[ a_iType ][ i ] = CreateProjectile( a_iType );
		m_apPools[ a_iType ][ i ]->SetPoolIndex( -1 );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0066cc70
void ASimpleProjectileManager::DestroyPool( TINT a_iType )
{
	for ( TINT i = m_aiPoolSizes[ a_iType ]; i != 0; )
	{
		AProjectile* pProjectile = m_apPools[ a_iType ][ --i ];

		if ( pProjectile )
			delete pProjectile;

		m_apPools[ a_iType ][ i ] = TNULL;
	}

	m_aiNumUsed[ a_iType ]   = 0;
	m_aiPoolSizes[ a_iType ] = 0;

	if ( m_apPools[ a_iType ] )
	{
		delete[] m_apPools[ a_iType ];
		m_apPools[ a_iType ] = TNULL;
	}
}

// $Barnyard: FUNCTION 0066ccf0
AProjectile* ASimpleProjectileManager::GetProjectile( TINT a_iType )
{
	for ( TINT i = 0; i < m_aiPoolSizes[ a_iType ]; i++ )
	{
		AProjectile* pProjectile = m_apPools[ a_iType ][ i ];

		if ( pProjectile->GetPoolIndex() == -1 )
		{
			pProjectile->SetPoolIndex( i );
			m_apPools[ a_iType ][ i ]->Reset();

			if ( m_aiNumUsed[ a_iType ] <= i )
				m_aiNumUsed[ a_iType ] = i + 1;

			return m_apPools[ a_iType ][ i ];
		}
	}

	// Every projectile is in use, reuse the first one
	m_apPools[ a_iType ][ 0 ]->Reset();
	m_apPools[ a_iType ][ 0 ]->SetPoolIndex( 0 );

	return m_apPools[ a_iType ][ 0 ];
}

// $Barnyard: FUNCTION 0066cc10
void ASimpleProjectileManager::ReleaseProjectile( AProjectile* a_pProjectile )
{
	TINT iIndex = a_pProjectile->GetPoolIndex();
	TINT iType  = a_pProjectile->GetType();

	if ( iIndex >= 0 )
	{
		if ( m_aiNumUsed[ iType ] <= iIndex + 1 )
			m_aiNumUsed[ iType ]--;

		a_pProjectile->Reset();
		m_apPools[ iType ][ iIndex ]->SetPoolIndex( -1 );
	}
}

// $Barnyard: FUNCTION 0066d040
AProjectile* ASimpleProjectileManager::FindClosestProjectile( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance )
{
	TFLOAT       fClosestDistanceSq = a_fMaxDistance * a_fMaxDistance;
	AProjectile* pClosest           = TNULL;

	for ( TINT iType = NUM_PROJECTILE_TYPES - 1; iType >= 0; iType-- )
	{
		for ( TINT i = m_aiNumUsed[ iType ]; i != 0; )
		{
			i--;

			if ( m_apPools[ iType ][ i ]->GetPoolIndex() == -1 )
				continue;

			AProjectile* pProjectile = m_apPools[ iType ][ i ];

			TVector4 vStart = pProjectile->GetPosition();
			TVector4 vEnd( pProjectile->GetPosition().x + pProjectile->GetVelocity().x, pProjectile->GetVelocity().y + pProjectile->GetPosition().y, pProjectile->GetVelocity().z + pProjectile->GetPosition().z, pProjectile->GetPosition().w );

			// NOTE: DistancePointToLineSq (006b8ba0, AWhackARaccMiniGameState.cpp) wraps TMath::DistancePointToLineSq
			TFLOAT fT;
			TFLOAT fDistanceSq = TMath::DistancePointToLineSq( a_rcPoint, vStart, vEnd, fT );

			if ( fDistanceSq < fClosestDistanceSq )
			{
				fClosestDistanceSq = fDistanceSq;
				pClosest           = pProjectile;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 0066d460
void ASimpleProjectileManager::StartTracking( TINT a_iRecord )
{
	m_bTracking = TTRUE;

	m_oHUDElement1.Create( TNULL );
	m_oStarRating.Create( 0.5f, 0.2f, 0.1f );
	m_oStarRating.SetVisible( TFALSE );
	m_oHUDElement1.SetVisible( TFALSE );

	m_iTrackedRecord = a_iRecord;

	// NOTE: a_iRecord is the high score record of the distance, m_fRecordDistance is its best score
	m_fRecordDistance = AHighScoreManager::GetRecord( a_iRecord )->aScores[ 0 ];
}

// $Barnyard: FUNCTION 0066d010
void ASimpleProjectileManager::StopTracking()
{
	m_bTracking = TFALSE;

	m_oStarRating.SetVisible( TFALSE );
	m_oHUDElement1.SetVisible( TFALSE );

	m_iTrackedRecord = -1;
}

// $Barnyard: FUNCTION 0066cda0
AProjectile* ASimpleProjectileManager::CreateProjectile( TINT a_iType )
{
	switch ( a_iType )
	{
		case TYPE_SIM_MILK:
			return new ASimMilkProjectile();
		case TYPE_VEGGIE_DEFENDER_MILK:
			return new AVeggieDefenderMilkProjectile();
		case TYPE_WITH_UNIT:
			return new AProjectileWithUnit();
		case TYPE_VEGGIE_THROW:
			return new AVeggieThrowProjectile();
		case TYPE_CHASING_CHICKS_THROW:
			return new AChasingChicksThrowProjectile();
		case TYPE_CHICKEN_COOP_DEFENDER_THROW:
			return new AChickenCoopDefenderThrowProjectile();
		case TYPE_MILK_PRECISION:
			return new AMilkPrecisionProjectile();
		case TYPE_CHICKEN_COOP_DEFENDER_MILK:
			return new AChickenCoopDefenderMilkProjectile();
		case TYPE_CHASING_CHICKS_MILK:
			return new AChasingChicksMilkProjectile();
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0066d330
void ASimpleProjectileManager::UpdateTracking( TFLOAT a_fDeltaTime, TBOOL a_bHasNewRecord )
{
	if ( !a_bHasNewRecord )
	{
		ms_fTrackingTimer += a_fDeltaTime;
	}
	else
	{
		ms_fTrackingTimer  = 0.0f;
		m_fTrackedDistance = m_vTrackedVelocity.MagnitudeXZ() * m_fTrackedAge;

		m_oHUDElement1.SetDistance( m_vTrackedPosition, m_fTrackedDistance );
		m_oHUDElement1.SetVisible( m_fTrackedDistance > 16.0f );

		if ( m_oStarRating.SetDistance( m_fTrackedDistance, m_fRecordDistance ) )
			m_bNewRecord = TTRUE;

		if ( m_oStarRating.GetNumStars() != 0 )
			AHighScoreManager::GetSingleton()->SetPendingScore( m_iTrackedRecord, m_fTrackedDistance, -2 );

		m_oStarRating.SetVisible( TTRUE );
	}

	if ( ms_fTrackingTimer > 2.0f )
	{
		ms_fTrackingTimer = 0.0f;
		m_fTrackedAge     = 0.0f;

		m_oHUDElement1.SetVisible( TFALSE );
		m_oStarRating.SetNumStars( 0 );
	}

	m_oHUDElement1.Update( a_fDeltaTime );
	m_oStarRating.Update( a_fDeltaTime );
}
