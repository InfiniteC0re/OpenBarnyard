#include "pch.h"
#include <Render/TCameraObject.h>
#include "ASuperSquirtMicroGame.h"
#include "FX/AMilkManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASimState.h"
#include "GUI/AGUI2.h"
#include "GUI/AHighScoreChart.h"
#include "GUI/T2GUIFontManager.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Helpers/AHighScoreManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitPlayer.h"
#include "Units/AInteractionComponent.h"
#include "Units/AInventoryComponentInterface.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( sunglasses );

// NOTE: GetClass, the create and the create-in-place functions of the TClass
// $Barnyard: FUNCTION 00538450
// $Barnyard: FUNCTION 00538dd0
// $Barnyard: FUNCTION 00538e30
TDEFINE_CLASS( ASuperSquirtMicroGame );

// $Barnyard: FUNCTION 00538460
ASuperSquirtMicroGame::ASuperSquirtMicroGame()
{
	SetInputMap( TPS8( NormalBarnyard ) );
}

// $Barnyard: FUNCTION 00538540
// $Barnyard: FUNCTION 00538520
ASuperSquirtMicroGame::~ASuperSquirtMicroGame()
{
}

// $Barnyard: FUNCTION 005385e0
TBOOL ASuperSquirtMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// No input until the player is ready
	if ( m_eState != STATE_PLAYING )
		return TTRUE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
			AGameStateController::GetSingleton()->StartMenuIfPossible();
			return TTRUE;
		case AInputCommand_Interact:
			if ( m_bReady )
				return TTRUE;

			// Without the sunglasses Interact leaves the game
			Remove();
			return TTRUE;
		case AInputCommand_Switch:
			return TTRUE;
		case AInputCommand_Inventory:
			Remove();
			return TTRUE;
		default:
			return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents ) != TFALSE;
	}
}

// $Barnyard: FUNCTION 00539070
TBOOL ASuperSquirtMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	AMilkManager::GetSingleton()->Update( a_fDeltaTime );

	switch ( m_eState )
	{
		case STATE_PUT_AWAY_HAND_ITEM:
			if ( !AInventoryGroupComponent::PutAwayHandItem() )
			{
				AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
				AInventoryComponent*      pHand      = pInventory->GetCategory( TPS8( Hand ) );

				if ( pHand->GetNumItems() == 0 )
					m_eState = STATE_TAKE_OUT_SUNGLASSES;
			}
			break;
		case STATE_TAKE_OUT_SUNGLASSES:
		{
			AInventoryComponentInterface* pInventory  = AInventoryComponentInterface::GetInventory( m_pPlayer );
			AInventoryItem*               pSunglasses = pInventory->FindItemByType( TPS8( sunglasses ) );

			if ( !pSunglasses )
			{
				// The player has no sunglasses
				m_bReady = TFALSE;
				ALocale::FormatInputTags( m_wszHint, TARRAYSIZE( m_wszHint ), 181 );
				m_eState = STATE_PLAYING;
				break;
			}

			AInventoryGroupComponent::TakeOutItem( pSunglasses, AGameStateController::GetSingleton()->GetCurrentState() );
			m_bReady = TTRUE;
			ALocale::FormatInputTags( m_wszHint, TARRAYSIZE( m_wszHint ), 180 );
			m_eState = STATE_WAIT_FOR_TOOL;
			break;
		}
		case STATE_WAIT_FOR_TOOL:
		{
			APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

			if ( pControl->IsCarryingTool() )
				m_eState = STATE_SELECT_LAUNCHER;
			break;
		}
		case STATE_SELECT_LAUNCHER:
		{
			AUnitPlayer*         pPlayer   = TSTATICCAST( AUnitPlayer, m_pPlayer );
			AProjectileLauncher* pLauncher = pPlayer->GetSecondaryLauncher();

			if ( pLauncher )
			{
				pPlayer->SelectLauncher( TTRUE );

				if ( pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
				{
					AAimedSquirtProjectileLauncher* pAimedLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher );
					pAimedLauncher->SetCursorInput( TTRUE );
					pAimedLauncher->SetMoveAimsCursor( TTRUE );
				}
			}

			AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, m_pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) );

			if ( pInteraction )
				pInteraction->SetInteractionsDisabled( TTRUE );

			ALocale::FormatInputTags( m_wszHint, TARRAYSIZE( m_wszHint ), 180 );
			m_eState = STATE_PLAYING;
			break;
		}
		default:
			break;
	}

	if ( m_eState != STATE_PLAYING || !m_bReady )
		return TTRUE;

	// NOTE: the state updates the active launcher (the milk squirt) itself
	AProjectileLauncher* pLauncher = TSTATICCAST( AUnitPlayer, m_pPlayer )->GetActiveLauncher();

	if ( pLauncher )
	{
		pLauncher->Update( a_fDeltaTime );

		if ( m_pMainHud )
			m_pMainHud->UpdateMilkAmmo( TSTATICCAST( AUnitPlayer, m_pPlayer )->GetActiveLauncher() );
	}

	TINT iRank = AHighScoreManager::PopLastRank();

	if ( iRank >= 0 )
	{
		APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

		if ( pProfile )
		{
			// A new score is in the chart, show the new best score
			CreateBestScorePanel();

			AHighScoreManager::Record* pRecord       = AHighScoreManager::GetRecord( m_iRecord );
			TINT                       iPreviousRank = AHighScoreManager::PopLastPreviousRank();

			// The player gets the distance as pickups if the score improved
			if ( iPreviousRank == -1 || iPreviousRank > iRank )
				pProfile->AddPickups( TINT( pRecord->aScores[ iRank ] ) );
		}
	}

	UpdateReticle( a_fDeltaTime );
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00538f50
void ASuperSquirtMicroGame::OnInsertion()
{
	BaseClass::OnInsertion();
	Start();

	m_pMainHud = AHudManager::GetSingleton()->FindElement<ASimPlayerMainHudElement>();

	// Hide the mini map while playing
	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

	if ( pSimState )
	{
		ASimMiniMap& rMiniMap = pSimState->GetMiniMap();
		m_bMiniMapForceHide   = rMiniMap.IsForceHidden();
		rMiniMap.SetForceHide( TTRUE );
	}

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "rekord18" );
	ALocale::FormatInputTags( m_wszHint, TARRAYSIZE( m_wszHint ), 180 );

	m_oHint.Create( pFont, pFont->GetTextWidth( m_wszHint ) );
	m_oHint.SetText( m_wszHint );
	m_oHint.SetShadow( TTRUE, 0xa0000000 );
	m_oHint.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oHint );

	CreateBestScorePanel();
	AGUI2::GetRootElement()->AddChildHead( m_oPanelRoot );
}

// $Barnyard: FUNCTION 005386e0
void ASuperSquirtMicroGame::OnRemoval()
{
	m_pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	AUnitPlayer*         pPlayer   = TSTATICCAST( AUnitPlayer, m_pPlayer );
	AProjectileLauncher* pLauncher = pPlayer->GetSecondaryLauncher();

	if ( pLauncher )
	{
		pPlayer->SelectLauncher( TFALSE );

		if ( pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		{
			AAimedSquirtProjectileLauncher* pAimedLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher );
			pAimedLauncher->SetCursorInput( TFALSE );
			pAimedLauncher->SetMoveAimsCursor( TFALSE );
		}
	}

	AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, m_pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pInteraction )
		pInteraction->SetInteractionsDisabled( TFALSE );

	APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pControl )
		pControl->SetMovementEnabled( TTRUE );

	ASimpleProjectileManager::GetSingleton()->StopTracking();

	m_oHint.RemoveSelf();
	m_oScore.RemoveSelf();
	m_oName.RemoveSelf();
	m_oFace.RemoveSelf();
	m_oPanel.RemoveSelf();
	m_oPanelRoot.RemoveSelf();
	m_pMainHud = TNULL;

	// Restore the mini map
	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

	if ( pSimState )
		pSimState->GetMiniMap().SetForceHide( m_bMiniMapForceHide );

	BaseClass::OnRemoval();
}

// $Barnyard: FUNCTION 00539040
void ASuperSquirtMicroGame::OnActivate()
{
	Start();
	m_oPanelRoot.Show();
	m_oHint.Show();

	BaseClass::OnActivate();
}

// $Barnyard: FUNCTION 005386b0
void ASuperSquirtMicroGame::OnDeactivate()
{
	m_oPanelRoot.Hide();
	m_oHint.Hide();
	ASimpleProjectileManager::GetSingleton()->StopTracking();

	BaseClass::OnDeactivate();
}

// $Barnyard: FUNCTION 00538e70
void ASuperSquirtMicroGame::Start()
{
	m_pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	AUnitPlayer*         pPlayer   = TSTATICCAST( AUnitPlayer, m_pPlayer );
	AProjectileLauncher* pLauncher = pPlayer->GetSecondaryLauncher();

	if ( pLauncher )
	{
		pPlayer->SelectLauncher( TTRUE );

		if ( pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
		{
			AAimedSquirtProjectileLauncher* pAimedLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher );
			pAimedLauncher->SetCursorInput( TTRUE );
			pAimedLauncher->SetMoveAimsCursor( TTRUE );
		}
	}

	m_iRecord = FindClosestRecord();

	APlayerControlComponent* pControl = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pControl )
		pControl->SetMovementEnabled( TFALSE );

	// NOTE: the original doesn't check pControl here
	m_bReady = pControl->IsCarryingTool();
	m_eState = STATE_PUT_AWAY_HAND_ITEM;

	if ( m_bReady )
		m_eState = STATE_SELECT_LAUNCHER;

	ASimpleProjectileManager::GetSingleton()->StartTracking( m_iRecord );
	m_HUDParams.SetFlags( AHudElement::MASK_UNK4 | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK8 | AHudElement::MASK_MINIMAP | AHudElement::MASK_UNK11 | AHudElement::MASK_GAME_TIME | AHudElement::MASK_ITEM_COUNT | AHudElement::MASK_TERRAIN_SECTION | AHudElement::MASK_PLAYER_MAIN );
}

// $Barnyard: FUNCTION 00538820
void ASuperSquirtMicroGame::CreateBestScorePanel()
{
	AHighScoreManager::Record* pRecord     = AHighScoreManager::GetRecord( m_iRecord );
	TINT                       iBestPlayer = pRecord->aPlayers[ 0 ];
	T2GUIFont*                 pFont       = T2GUIFontManager::FindFont( "rekord18" );

	// Name of the best player, the human players have the name of the profile
	m_oName.Create( pFont, 10.0f );
	m_oName.SetColour( 0xffffffff );
	m_oName.SetShadow( TTRUE, 0xa0000000 );
	m_oName.SetScale( 1.2f );

	const TWCHAR* wszName;

	if ( iBestPlayer < 0 )
		wszName = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString();
	else
		wszName = g_pLocaleManager->GetString( iBestPlayer );

	m_oName.SetTextAndFitWidth( wszName );

	// The best score, Italian and Spanish use a decimal comma
	T2Locale::Lang eLanguage = g_pLocaleManager->GetLanguage();

	if ( eLanguage == ALocaleManager::Lang_Spanish || eLanguage == ALocaleManager::Lang_Italian )
	{
		TINT iWhole    = TMath::FloorToInt( pRecord->aScores[ 0 ] );
		TINT iFraction = TMath::FloorToInt( ( pRecord->aScores[ 0 ] - TFLOAT( iWhole ) ) * 100.0f );
		TStringManager::String16Format( m_wszScore, TARRAYSIZE( m_wszScore ), L" %d,%02d %s", iWhole, iFraction, g_pLocaleManager->GetString( 1182 ) );
	}
	else
	{
		TStringManager::String16Format( m_wszScore, TARRAYSIZE( m_wszScore ), L" %.2f %s", pRecord->aScores[ 0 ], g_pLocaleManager->GetString( 1182 ) );
	}

	m_oScore.Create( pFont, 10.0f );
	m_oScore.SetShadowEnabled( TTRUE );
	m_oScore.SetColour( 0xffffc800 );
	m_oScore.SetShadowColour( 0xa0000000 );
	m_oScore.SetScale( 1.2f );
	m_oScore.SetTextAndFitWidth( m_wszScore );

	// Face of the best player
	T2GUITextureSection* pFace = AHighScoreChart::GetPlayerTexture( iBestPlayer );

	if ( pFace )
	{
		m_oFace.SetTextureSection( *pFace );
		m_oFace.SetAlpha( 1.0f );
	}
	else
	{
		m_oFace.T2GUIElement::SetDimensions( 1.0f, 1.0f );
		m_oFace.SetAlpha( 0.0f );
	}

	TFLOAT fFaceWidth  = m_oFace.GetWidth();
	TFLOAT fPanelWidth = TMath::Max( m_oName.GetWidth(), m_oScore.GetWidth() ) + fFaceWidth + 4.0f;
	TFLOAT fTextHeight = m_oName.GetHeight() + m_oScore.GetHeight();

#ifdef BARNYARD_COMMUNITY_PATCH
	TFLOAT fPanelHeight = TMath::Max( m_oFace.GetHeight(), fTextHeight ) + 4.0f;
#else
	// NOTE: the original uses the width of the face when it's higher than the texts
	TFLOAT fPanelHeight = ( ( m_oFace.GetHeight() < fTextHeight ) ? fTextHeight : m_oFace.GetWidth() ) + 4.0f;
#endif

	m_oPanel.T2GUIElement::SetDimensions( fPanelWidth, fPanelHeight );
	m_oPanel.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Panel_01" ) );

	m_oName.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPLEFT );
	m_oName.SetTransform( 4.0f, 0.0f );
	m_oFace.AddChildTail( m_oName );

	m_oScore.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oScore.SetTransform( 0.0f, 3.0f );
	m_oName.AddChildTail( m_oScore );

	m_oFace.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oPanel.AddChildTail( m_oFace );

	m_oPanelRoot.T2GUIElement::SetDimensions( m_oPanel.GetWidth(), m_oPanel.GetHeight() );
	m_oPanelRoot.SetAlpha( 0.0f );
	m_oPanel.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oPanelRoot.AddChildTail( m_oPanel );

	m_oPanelRoot.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oPanelRoot.SetTransform( 0.0f, T2GUIFontManager::FindFont( "rekord18" )->GetLineHeight() * -1.5f );
}

// $Barnyard: FUNCTION 00538ba0
void ASuperSquirtMicroGame::UpdateReticle( TFLOAT a_fDeltaTime )
{
	AProjectileLauncher* pLauncher = TSTATICCAST( AUnitPlayer, m_pPlayer )->GetActiveLauncher();

	if ( pLauncher && pLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) ) )
	{
		AAimedSquirtProjectileLauncher* pAimedLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pLauncher );
		ATargetReticle&                 rReticle       = pAimedLauncher->GetReticle();

		pAimedLauncher->LockOn( TNULL );
		pAimedLauncher->UpdateLockedCursor();
		rReticle.SetMode( ATargetReticle::MODE_IDLE );
		rReticle.UpdateRectangles();
	}
}

// $Barnyard: FUNCTION 00538c00
TINT ASuperSquirtMicroGame::FindClosestRecord()
{
	TVector4 vecPlayerPos     = *m_pPlayer->GetSteer()->GetPosition();
	TFLOAT   fClosestDistance = 99999.99f;
	TINT     iClosest         = -1;

	for ( TINT i = 1; i < NUM_HIGHSQUIRT_LOCATORS + 1; i++ )
	{
		TCHAR szLocator[ 20 ];
		TStringManager::String8Format( szLocator, sizeof( szLocator ), "highsquirt_%02d", i );

		TVector4 vecPosition;
		TVector4 vecDirection;

		if ( ATerrainInterface::GetSingleton()->GetLocatorPosAndDir( szLocator, vecPosition, vecDirection ) )
		{
			TFLOAT fDistance = TVector4::Distance( vecPlayerPos, vecPosition );

			if ( fDistance < fClosestDistance )
			{
				fClosestDistance = fDistance;
				iClosest         = i;
			}
		}
	}

	// The spots 1-12 use the records 0-11, the others the record 0
	if ( iClosest >= 1 && iClosest <= 12 )
		return iClosest - 1;

	return 0;
}
