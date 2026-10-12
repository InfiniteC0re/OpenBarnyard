#include "pch.h"
#include "ACoinTrailActivity.h"
#include "AUnit.h"
#include "ACoinPickupCoinItem.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Cameras/ACamera.h"
#include "GameInterface/AGameStateController.h"
#include "HUD/AHudManager.h"
#include "ARandom.h"

#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b4580, CreateObject 005b4a10
// NOTE: GetClass 005b4580, CreateObject 005b4a10, CreateObjectInPlace 005b4a70
TDEFINE_CLASS( ACoinTrailActivity );

TPSTRING8_DECLARE( time );
TPSTRING8_DECLARE( count );
TPSTRING8_DECLARE( precisioncoin );

// Sound of a collected coin
static constexpr ASoundId SOUND_COIN_COLLECTED = 0xa5;

// $Barnyard: FUNCTION 005b4610
ACoinTrailActivity::ACoinTrailActivity()
    : m_iNumCoins( 0 )
    , m_iNumCoinsLeft( 0 )
    , m_iCount( 0 )
    , m_fTimeLeft( 0.0f )
    , m_fTime( 0.0f )
    , m_uiHUDFlags( TUINT( -1 ) )
{
	m_oSplineTRB.Load( "Data\\treasure\\coinspline1.trb", 0 );

	m_aSplines[ 0 ].Load( m_oSplineTRB, "coinspline1" );

	m_oSplineTRB.DeleteSymbolTable();
}

// $Barnyard: FUNCTION 005b4590
// $Barnyard: FUNCTION 005b46e0
ACoinTrailActivity::~ACoinTrailActivity()
{
}

// $Barnyard: FUNCTION 005b4700
TBOOL ACoinTrailActivity::Create( const PBProperties* a_pProperties )
{
	if ( a_pProperties->GetOptionalPropertyValue( m_fTime, TPS8( time ) ) )
		m_fTime = TMath::Max( m_fTime, 0.0f );

	if ( a_pProperties->GetOptionalPropertyValue( m_iCount, TPS8( count ) ) )
		TMath::Clip( m_iCount, 0, MAX_NUM_COINS );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b4770
AInteractionActivity::RESULT ACoinTrailActivity::OnStart( AInteractionSequence* a_pSequence )
{
	m_fTimeLeft = m_fTime;

	// Hide most of the HUD and show the time left
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();
	m_uiHUDFlags       = pState->GetHUDParams().m_uiFlags;
	pState->GetHUDParams().SetFlags( AHudElement::MASK_UNK2 | AHudElement::MASK_UNK4 | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK11 | AHudElement::MASK_GAME_TIME | AHudElement::MASK_ITEM_COUNT | AHudElement::MASK_TERRAIN_SECTION | AHudElement::MASK_PLAYER_MAIN );

	AHudManager::GetSingleton()->AddElement( &m_oTimeLeftHud, TFALSE, TTRUE, TFALSE );
	m_oTimeLeftHud.SetForceHide( TTRUE );
	m_oTimeLeftHud.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oTimeLeftHud.Setup( m_fTime );
	m_oTimeLeftHud.SetTime( m_fTimeLeft );

	// The coins are spread evenly along the spline
	TVector4 avecPositions[ MAX_NUM_COINS ];

	for ( TINT i = 0; i < m_iCount; i++ )
		m_aSplines[ 0 ].GetPosition( TFLOAT( i ) / TFLOAT( m_iCount ), avecPositions[ i ] );

	for ( TINT i = 0; i < m_iCount; i++ )
	{
		// Random rotation
		TVector3 vecDirection( ACamera::sm_vInitialLookDirection.x, ACamera::sm_vInitialLookDirection.y, ACamera::sm_vInitialLookDirection.z );
		vecDirection.RotateY( TFLOAT( ARandom::GetSingleton()->m_oRandom.RandRaw() >> 1 ) * 4.656613e-10f * TMath::TWO_PI );

		TVector4 vecDirection4( vecDirection.x, vecDirection.y, vecDirection.z, ACamera::sm_vInitialLookDirection.w );

		AUnitTransform oTransform( avecPositions[ i ], vecDirection4, 0, -1, -1, TTRUE );
		oTransform.m_iModelIndex = 3;

		ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, AUnitManager::GetSingleton()->SpawnUnit( TPS8( precisioncoin ), &oTransform ) );
		pCoin->ChangeSteer( &TGetClass( AThrowItemSteer ) );
		pCoin->SetValue( 1 );
		pCoin->SetMovement( TTRUE );
		pCoin->SetStartPosition( avecPositions[ i ] );

		m_apCoins[ m_iNumCoins++ ] = pCoin;
		m_iNumCoinsLeft++;
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005b4ab0
AInteractionActivity::RESULT ACoinTrailActivity::Update( AInteractionSequence* a_pSequence )
{
	// NOTE: retail counts down by a fixed 1/30 s per update
	m_fTimeLeft -= 1.0f / 30.0f;

	if ( m_fTimeLeft <= 0.0f )
	{
		m_fTimeLeft = 0.0f;
		Finish();
		return RESULT_DONE;
	}

	m_oTimeLeftHud.SetForceHide( TFALSE );
	m_oTimeLeftHud.SetTime( m_fTimeLeft );

	// Collect the coins the player touches
	TVector4 vPlayerPosition = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();

	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		ACoinPickupCoinItem* pCoin = TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] );

		if ( !pCoin->IsCollected() && TMath::SphereVsSphere( *pCoin->GetSteer()->GetPosition(), 1.0f, vPlayerPosition, 1.0f ) )
		{
			pCoin->SetCollected( TTRUE );
			m_iNumCoinsLeft--;
			ASoundManager::GetSingleton()->PlayCue( SOUND_COIN_COLLECTED );
		}
	}

	if ( m_iNumCoinsLeft > 0 )
		return RESULT_RUNNING;

	// Wait for the collected coins to fade out
	TINT iNumVisible = 0;

	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		if ( TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] )->IsVisible() == TTRUE )
			iNumVisible++;
	}

	if ( m_iNumCoins > 0 && iNumVisible > 0 )
		return RESULT_RUNNING;

	Finish();
	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005b4990
void ACoinTrailActivity::Finish()
{

	for ( TINT i = 0; i < m_iNumCoins; i++ )
	{
		TSTATICCAST( ACoinPickupCoinItem, m_apCoins[ i ] )->Destroy();
	}

	for ( TINT i = 0; i < m_iNumCoins; i++ )
		m_apCoins[ i ] = TNULL;

	m_iNumCoins = 0;

	m_oTimeLeftHud.Detach();
	AGameStateController::GetSingleton()->GetCurrentState()->GetHUDParams().SetFlags( m_uiHUDFlags );
}
