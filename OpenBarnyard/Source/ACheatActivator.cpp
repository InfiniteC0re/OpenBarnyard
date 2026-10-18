#include "pch.h"
#include "ACheatActivator.h"
#include "Units/AUnit.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASimState.h"
#include "GameInterface/AFrontEndMainMenuState2.h"
#include "GameInterface/AFrontEndMiniGameState2.h"
#include "MiniGame/AMiniGameState.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2MessageBoxState.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/ABikeManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Units/AInteractionComponent.h"
#include "Units/AInteractionRequirement.h"
#include "Units/AInventoryGroupComponent.h"
#include "MiniGame/ABarnExploreMinigameState.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( apple );
TPSTRING8_DECLARE( pumpkin );
TPSTRING8_DECLARE( melon );
TPSTRING8_DECLARE( lemon );
TPSTRING8_DECLARE( walnut );
TPSTRING8_DECLARE( berrys );
TPSTRING8_DECLARE( egg );
TPSTRING8_DECLARE( honeycomb );
TPSTRING8_DECLARE( carrot );
TPSTRING8_DECLARE( cream );
TPSTRING8_DECLARE( butter );
TPSTRING8_DECLARE( milkcarton );
TPSTRING8_DECLARE( strawberry );
TPSTRING8_DECLARE( cabbage );
TPSTRING8_DECLARE( flour );
TPSTRING8_DECLARE( chocolate );
TPSTRING8_DECLARE( ice );
TPSTRING8_DECLARE( snowdrops );
TPSTRING8_DECLARE( bluebells );
TPSTRING8_DECLARE( foxgloves );
TPSTRING8_DECLARE( cownip );
TPSTRING8_DECLARE( oxeyeddaisy );
TPSTRING8_DECLARE( treasurebells );

// NOTE: GetClass 00421c20, CreateObject 004231a0, CreateObjectInPlace 00423200 (the constructor is inlined in both)
TDEFINE_CLASS( ACheatActivator );

// Text of the button of the cheat popup
static TWCHAR s_wszCheatText[ 256 ];

// DAT_00719100: the keys that are recorded while the cheat key is down
const TINT ACheatActivator::ms_aCheatKeys[ 28 ] = {
	TInputDeviceKeyboard::KEY_A,
	TInputDeviceKeyboard::KEY_B,
	TInputDeviceKeyboard::KEY_C,
	TInputDeviceKeyboard::KEY_D,
	TInputDeviceKeyboard::KEY_E,
	TInputDeviceKeyboard::KEY_F,
	TInputDeviceKeyboard::KEY_G,
	TInputDeviceKeyboard::KEY_H,
	TInputDeviceKeyboard::KEY_I,
	TInputDeviceKeyboard::KEY_J,
	TInputDeviceKeyboard::KEY_K,
	TInputDeviceKeyboard::KEY_L,
	TInputDeviceKeyboard::KEY_M,
	TInputDeviceKeyboard::KEY_N,
	TInputDeviceKeyboard::KEY_O,
	TInputDeviceKeyboard::KEY_P,
	TInputDeviceKeyboard::KEY_Q,
	TInputDeviceKeyboard::KEY_R,
	TInputDeviceKeyboard::KEY_S,
	TInputDeviceKeyboard::KEY_T,
	TInputDeviceKeyboard::KEY_U,
	TInputDeviceKeyboard::KEY_V,
	TInputDeviceKeyboard::KEY_W,
	TInputDeviceKeyboard::KEY_X,
	TInputDeviceKeyboard::KEY_Y,
	TInputDeviceKeyboard::KEY_Z,
	TInputDeviceKeyboard::KEY_RIGHT,
	TInputDeviceKeyboard::KEY_LEFT,
};

#define CHEAT_KEY( LETTER ) TInputDeviceKeyboard::KEY_##LETTER

// DAT_00719178
const ACheatActivator::CheatCodeRef ACheatActivator::ms_aCheats[ CHEAT_NUMOF ] = {
	// TIMETOGOTOWORK
	{ { CHEAT_KEY( T ), CHEAT_KEY( I ), CHEAT_KEY( M ), CHEAT_KEY( E ), CHEAT_KEY( T ), CHEAT_KEY( O ), CHEAT_KEY( G ), CHEAT_KEY( O ), CHEAT_KEY( T ), CHEAT_KEY( O ), CHEAT_KEY( W ), CHEAT_KEY( O ), CHEAT_KEY( R ), CHEAT_KEY( K ) }, 14, 167, STATEMASK_MAIN_MENU, TTRUE },
	// DOGSPLAYINGPOOL
	{ { CHEAT_KEY( D ), CHEAT_KEY( O ), CHEAT_KEY( G ), CHEAT_KEY( S ), CHEAT_KEY( P ), CHEAT_KEY( L ), CHEAT_KEY( A ), CHEAT_KEY( Y ), CHEAT_KEY( I ), CHEAT_KEY( N ), CHEAT_KEY( G ), CHEAT_KEY( P ), CHEAT_KEY( O ), CHEAT_KEY( O ), CHEAT_KEY( L ) }, 15, 167, STATEMASK_MAIN_MENU, TTRUE },
	// COWSONBIKES
	{ { CHEAT_KEY( C ), CHEAT_KEY( O ), CHEAT_KEY( W ), CHEAT_KEY( S ), CHEAT_KEY( O ), CHEAT_KEY( N ), CHEAT_KEY( B ), CHEAT_KEY( I ), CHEAT_KEY( K ), CHEAT_KEY( E ), CHEAT_KEY( S ) }, 11, 167, STATEMASK_MAIN_MENU, TTRUE },
	// HANGINGWITHOTIS
	{ { CHEAT_KEY( H ), CHEAT_KEY( A ), CHEAT_KEY( N ), CHEAT_KEY( G ), CHEAT_KEY( I ), CHEAT_KEY( N ), CHEAT_KEY( G ), CHEAT_KEY( W ), CHEAT_KEY( I ), CHEAT_KEY( T ), CHEAT_KEY( H ), CHEAT_KEY( O ), CHEAT_KEY( T ), CHEAT_KEY( I ), CHEAT_KEY( S ) }, 15, 167, STATEMASK_MAIN_MENU, TTRUE },
	// ILOVEPUMPKINS
	{ { CHEAT_KEY( I ), CHEAT_KEY( L ), CHEAT_KEY( O ), CHEAT_KEY( V ), CHEAT_KEY( E ), CHEAT_KEY( P ), CHEAT_KEY( U ), CHEAT_KEY( M ), CHEAT_KEY( P ), CHEAT_KEY( K ), CHEAT_KEY( I ), CHEAT_KEY( N ), CHEAT_KEY( S ) }, 13, 167, STATEMASK_MAIN_MENU, TTRUE },
	// THATSCALLEDCOWTIPPING
	{ { CHEAT_KEY( T ), CHEAT_KEY( H ), CHEAT_KEY( A ), CHEAT_KEY( T ), CHEAT_KEY( S ), CHEAT_KEY( C ), CHEAT_KEY( A ), CHEAT_KEY( L ), CHEAT_KEY( L ), CHEAT_KEY( E ), CHEAT_KEY( D ), CHEAT_KEY( C ), CHEAT_KEY( O ), CHEAT_KEY( W ), CHEAT_KEY( T ), CHEAT_KEY( I ), CHEAT_KEY( P ), CHEAT_KEY( P ), CHEAT_KEY( I ), CHEAT_KEY( N ), CHEAT_KEY( G ) }, 21, 167, STATEMASK_MAIN_MENU, TTRUE },
	// OHMILKME
	// NOTE: retail stores the length 21 (copied from the entry above) for the 8 keys, the other keys are 0 and
	// are never recorded, so this cheat can't be entered. FUNANDGAMES unlocks BarnTender as well.
	{ { CHEAT_KEY( O ), CHEAT_KEY( H ), CHEAT_KEY( M ), CHEAT_KEY( I ), CHEAT_KEY( L ), CHEAT_KEY( K ), CHEAT_KEY( M ), CHEAT_KEY( E ) }, 21, 167, STATEMASK_MAIN_MENU, TTRUE },
	// GIMMESOMELOVE
	{ { CHEAT_KEY( G ), CHEAT_KEY( I ), CHEAT_KEY( M ), CHEAT_KEY( M ), CHEAT_KEY( E ), CHEAT_KEY( S ), CHEAT_KEY( O ), CHEAT_KEY( M ), CHEAT_KEY( E ), CHEAT_KEY( L ), CHEAT_KEY( O ), CHEAT_KEY( V ), CHEAT_KEY( E ) }, 13, 1684, STATEMASK_SIM, TTRUE },
	// IWANTSOME
	{ { CHEAT_KEY( I ), CHEAT_KEY( W ), CHEAT_KEY( A ), CHEAT_KEY( N ), CHEAT_KEY( T ), CHEAT_KEY( S ), CHEAT_KEY( O ), CHEAT_KEY( M ), CHEAT_KEY( E ) }, 9, 1685, STATEMASK_SIM, TTRUE },
	// SHOWMESTUFF
	{ { CHEAT_KEY( S ), CHEAT_KEY( H ), CHEAT_KEY( O ), CHEAT_KEY( W ), CHEAT_KEY( M ), CHEAT_KEY( E ), CHEAT_KEY( S ), CHEAT_KEY( T ), CHEAT_KEY( U ), CHEAT_KEY( F ), CHEAT_KEY( F ) }, 11, 167, STATEMASK_BONUS | STATEMASK_ANTICS | STATEMASK_MAIN_MENU, TTRUE },
	// FUNANDGAMES
	{ { CHEAT_KEY( F ), CHEAT_KEY( U ), CHEAT_KEY( N ), CHEAT_KEY( A ), CHEAT_KEY( N ), CHEAT_KEY( D ), CHEAT_KEY( G ), CHEAT_KEY( A ), CHEAT_KEY( M ), CHEAT_KEY( E ), CHEAT_KEY( S ) }, 11, 167, STATEMASK_BONUS | STATEMASK_ANTICS | STATEMASK_MAIN_MENU, TTRUE },
};

#undef CHEAT_KEY

// $Barnyard: FUNCTION 00423010
ACheatActivator::ACheatActivator()
{
	m_iFirstKey = 0;
	m_iNumKeys  = 0;
	m_iMaxKeys  = MAX_CODE_LENGTH;
	m_pKeys     = new TINT[ MAX_CODE_LENGTH ];
	m_iAct      = 1;
	m_iTier     = 1;
	m_bUnk60    = TTRUE;
	m_iUnk04    = 0;
	m_iUnk08    = 0;
	m_bUnk0C    = TFALSE;
	m_bUnk0D    = TFALSE;
	m_bUnk0E    = TFALSE;

	m_oInteractionListener.Connect( this, OnInteractionManagerEventCallback );

	m_iNumTierRequirements = AQuestManager::GetSingleton()->GetNumTiers() + 3;
	m_paiTierRequirements  = new TINT[ m_iNumTierRequirements ];
}

// $Barnyard: FUNCTION 00422c50
// $Barnyard: FUNCTION 00423100
ACheatActivator::~ACheatActivator()
{
	m_oInteractionListener.Disconnect();
	delete[] m_paiTierRequirements;
	delete[] m_pKeys;
}

// $Barnyard: FUNCTION 00423240
TBOOL ACheatActivator::ProcessCheatCodeInput( const TInputInterface::InputEvent* a_pEvent )
{
	if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
		return TFALSE;

	a_pEvent->GetSource()->StopRepeat( a_pEvent->GetDoodad() );

	for ( TINT i = 0; i < TARRAYSIZE( ms_aCheatKeys ); i++ )
	{
		if ( ms_aCheatKeys[ i ] == a_pEvent->GetDoodad() )
		{
			AddCheatCodeInput( a_pEvent->GetDoodad() );
			break;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00421d00
TBOOL ACheatActivator::IsCheatKeyDown()
{
	TInputDeviceKeyboard* pKeyboard = AInputHandler::GetSingleton()->GetKeyboardDevice();

	if ( pKeyboard )
		return pKeyboard->IsDown( TInputDeviceKeyboard::KEY_TAB );

	return TFALSE;
}

// $Barnyard: FUNCTION 00421c50
TBOOL ACheatActivator::ResetCheatCodeInput()
{
	m_iNumKeys  = 0;
	m_iFirstKey = 0;
	return TFALSE;
}

// $Barnyard: FUNCTION 00421c60
TBOOL ACheatActivator::Reset()
{
	m_iNumKeys  = 0;
	m_iFirstKey = 0;
	m_iAct      = 1;
	m_iTier     = 1;
	return TTRUE;
}

// $Barnyard: FUNCTION 00422000
void ACheatActivator::UpdateQuestTier()
{
	m_iAct  = AQuestManager::GetSingleton()->GetAct();
	m_iTier = AQuestManager::GetSingleton()->GetTier();
}

// $Barnyard: FUNCTION 00421e80
void ACheatActivator::UpdateTierRequirements()
{
	AInteractionManager* pInteractionManager = AInteractionManager::GetSingleton();
	TINT                 iTier               = 1;

	for ( TINT iAct = 1; iAct <= m_iAct; iAct++ )
	{
		TINT iEndTier = ( iAct < m_iAct ) ? AQuestManager::GetSingleton()->GetNumTiersInAct( iAct - 1 ) + 1 : m_iTier;

		if ( iEndTier > 1 )
		{
			do
			{
				AQuestManager* pQuestManager = AQuestManager::GetSingleton();
				TINT           iIndex        = -1;

				if ( iAct > 0 && iTier > 0 )
					iIndex = pQuestManager->GetAbsQuestTier( iAct, iTier ) - 1;
				else if ( iTier == -1 )
					iIndex = pQuestManager->GetAbsQuestTier( 3, pQuestManager->GetNumTiersInAct( 2 ) ) + iAct - 1;

				pInteractionManager->GetRequirement( m_paiTierRequirements[ iIndex ] )->SetForcedState( AInteractionRequirement::FORCEDSTATE_NOT_SATISFIED );
				iTier++;
			} while ( iTier < iEndTier );

			iTier = 1;
		}

		if ( iAct < m_iAct )
		{
			AQuestManager* pQuestManager = AQuestManager::GetSingleton();
			TINT           iIndex        = pQuestManager->GetAbsQuestTier( 3, pQuestManager->GetNumTiersInAct( 2 ) ) + iAct - 1;

			AInteractionManager::GetSingleton()->GetRequirement( m_paiTierRequirements[ iIndex ] )->SetForcedState( AInteractionRequirement::FORCEDSTATE_NOT_SATISFIED );
		}
	}

	if ( m_iAct != 1 && m_iTier != 1 )
		AInteractionManager::GetSingleton()->GetRequirement( m_aiRequirements[ REQUIREMENT_ACT_1_TIER_0_COMPLETE ] )->SetForcedState( AInteractionRequirement::FORCEDSTATE_NOT_SATISFIED );

	if ( m_iTier == 1 && m_iAct != 1 )
	{
		AQuestManager* pQuestManager = AQuestManager::GetSingleton();
		TINT           iIndex        = pQuestManager->GetAbsQuestTier( 3, pQuestManager->GetNumTiersInAct( 2 ) ) + m_iAct - 2;

		AInteractionManager::GetSingleton()->GetRequirement( m_paiTierRequirements[ iIndex ] )->SetForcedState( AInteractionRequirement::FORCEDSTATE_SATISFIED );
	}
	else if ( m_iAct != 1 || m_iTier != 1 )
	{
		AInteractionManager::GetSingleton()->GetRequirement( GetTierRequirement( m_iAct, m_iTier - 1 ) )->SetForcedState( AInteractionRequirement::FORCEDSTATE_SATISFIED );
	}
}

// $Barnyard: FUNCTION 00421e10
TINT ACheatActivator::GetTierRequirement( TINT a_iAct, TINT a_iTier )
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( a_iAct > 0 && a_iTier > 0 )
		return m_paiTierRequirements[ pQuestManager->GetAbsQuestTier( a_iAct, a_iTier ) - 1 ];

	if ( a_iTier == -1 )
		return m_paiTierRequirements[ pQuestManager->GetAbsQuestTier( 3, pQuestManager->GetNumTiersInAct( 2 ) ) + a_iAct - 1 ];

	// NOTE: retail reads the element before the array here
	return m_paiTierRequirements[ -1 ];
}

// $Barnyard: FUNCTION 00421db0
TBOOL ACheatActivator::SetRequirementSatisfied( TINT a_iRequirement, TBOOL a_bSatisfied )
{
	AInteractionRequirement* pRequirement = AInteractionManager::GetSingleton()->GetRequirement( a_iRequirement );

	if ( !pRequirement )
		return TFALSE;

	pRequirement->SetForcedState( a_bSatisfied ? AInteractionRequirement::FORCEDSTATE_SATISFIED : AInteractionRequirement::FORCEDSTATE_NOT_SATISFIED );
	return TTRUE;
}

// $Barnyard: FUNCTION 00421de0
TBOOL ACheatActivator::ResetRequirement( TINT a_iRequirement )
{
	AInteractionRequirement* pRequirement = AInteractionManager::GetSingleton()->GetRequirement( a_iRequirement );

	if ( !pRequirement )
		return TFALSE;

	pRequirement->SetForcedState( AInteractionRequirement::FORCEDSTATE_NONE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00423120
TBOOL ACheatActivator::AddCheatCodeInput( TINT a_iKey )
{
	if ( m_iNumKeys >= m_iMaxKeys )
	{
		m_pKeys[ m_iFirstKey ] = a_iKey;
		m_iFirstKey            = ( m_iFirstKey + 1 ) % m_iMaxKeys;
	}
	else
	{
		m_pKeys[ ( m_iNumKeys + m_iFirstKey ) % m_iMaxKeys ] = a_iKey;
		m_iNumKeys++;
	}

	for ( TINT i = 0; i < CHEAT_NUMOF; i++ )
	{
		if ( IsMatchCheatCode( ms_aCheats[ i ].aKeys, ms_aCheats[ i ].iLength ) )
		{
			TriggerCheat( CHEAT( i ) );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00421c80
TBOOL ACheatActivator::IsMatchCheatCode( const TINT* a_pKeys, TINT a_iLength )
{
	if ( m_iNumKeys >= a_iLength && a_iLength > 0 )
	{
		// Look for the code anywhere in the recorded keys
		for ( TINT iStart = 0; iStart <= m_iNumKeys - a_iLength; iStart++ )
		{
			TINT i = 0;

			while ( a_pKeys[ i ] == m_pKeys[ ( iStart + m_iFirstKey + i ) % m_iMaxKeys ] )
			{
				if ( ++i >= a_iLength )
					return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00422d10
void ACheatActivator::TriggerCheat( CHEAT a_eCheat )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( pPlayer )
	{
		AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, pPlayer->FindComponent( &TGetClass( AInteractionComponent ) ) );

		// No cheats during interaction sequences
		if ( pInteraction && pInteraction->IsInSequence() == TTRUE )
			return;
	}

	m_iLastCheat = a_eCheat;

	if ( !( GetCurrentStateMask() & ms_aCheats[ a_eCheat ].eAllowedStates ) )
	{
		m_iNumKeys  = 0;
		m_iFirstKey = 0;
		return;
	}

	ActivateCheat( a_eCheat );

	if ( ms_aCheats[ m_iLastCheat ].bShowPopup )
	{
		// Release Tab and Shift so they don't stay down while the popup is on
		TInputDeviceKeyboard* pKeyboard = AInputHandler::GetSingleton()->GetKeyboardDevice();
		TInputInterface::GetSingleton()->SendEvent( pKeyboard, TInputDeviceKeyboard::KEY_TAB, TInputInterface::EVENT_TYPE_GONE_UP );
		TInputInterface::GetSingleton()->SendEvent( pKeyboard, TInputDeviceKeyboard::KEY_LSHIFT, TInputInterface::EVENT_TYPE_GONE_UP );

		T2GUIElement* pRootElement = AGUI2::GetRootElement();
		TFLOAT        fWidth       = pRootElement->GetWidth() * 0.4f;
		TFLOAT        fHeight      = pRootElement->GetHeight() * 0.5f;

		AGUI2MessageBoxState* pMessageBox = new AGUI2MessageBoxState();
		pMessageBox->Create( fWidth, fHeight, "rekord26", "rekord26", "Border_Top_Left", "Border_Horizontal", "Border_Top_Right", "Border_Vertical", "Border_Middle", "Border_Vertical", "Border_Bottom_Left", "Border_Horizontal", "Border_Bottom_Right", TFALSE );
		pMessageBox->GetTextBox()->SetScale( 0.8f );

		// "Cheat activated!"
		pMessageBox->SetText( ALocaleManager::Interface()->GetString( 1675 ) );

		const TWCHAR* wszText = ALocaleManager::Interface()->GetString( ms_aCheats[ m_iLastCheat ].iMessage );

		// 167 is "Press OK to continue" with an input tag
		if ( ms_aCheats[ m_iLastCheat ].iMessage == 167 )
			ALocale::FormatInputTags( s_wszCheatText, TARRAYSIZE( s_wszCheatText ), wszText );
		else
			TStringManager::String16Format( s_wszCheatText, TARRAYSIZE( s_wszCheatText ), L"%s", wszText );

		pMessageBox->AddButton( s_wszCheatText );
		pMessageBox->SetPadding( 60.0f, 10.0f );
		pMessageBox->SetAutoSize( TTRUE );
		pMessageBox->SetTextColour( 0xffffefb4 );
		pMessageBox->SetButtonScales( 0.8f, 0.8f );
		pMessageBox->SetDialogColour( 0xffffffff );
		pMessageBox->SetButtonColours( 0xffffffff, 0xff74482b );
		pMessageBox->SetTextShadowEnabled( TTRUE );
		pMessageBox->SetTextShadowColour( 0xa0000000 );
		pMessageBox->SetTextShadowAlpha( 0.5f );
		pMessageBox->SetTextShadowOffset( 5.0f, 5.0f );
		pMessageBox->SetPadding( 64.0f, 128.0f );

		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_CONFIRM );
		AGameStateController::GetSingleton()->PushState( pMessageBox );
	}
}

// $Barnyard: FUNCTION 00422020
void ACheatActivator::ActivateCheat( CHEAT a_eCheat )
{
	switch ( a_eCheat )
	{
		case CHEAT_UNLOCK_MUD_JUMP:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_MUD_JUMP );
			break;
		case CHEAT_UNLOCK_BARN_POOL:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_BARN_POOL );
			break;
		case CHEAT_UNLOCK_BIKE_RACE:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_BIKE_RACE );
			break;
		case CHEAT_UNLOCK_CHASING_CHICKS:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_CHASING_CHICKS );
			break;
		case CHEAT_UNLOCK_VEGGIE_PATCH:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_VEGGIE_PATCH );
			break;
		case CHEAT_UNLOCK_COW_TIPPING:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_COW_TIPPING );
			break;
		case CHEAT_UNLOCK_BARNTENDER:
			g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_BARNTENDER );
			break;
		case CHEAT_MONEY:
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetPickups( 9999, TFALSE, TTRUE );
			m_iNumKeys  = 0;
			m_iFirstKey = 0;
			return;
		case CHEAT_ITEMS:
		{
			AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

			if ( !pInventory )
			{
				// The player is on the bike, use the inventory of the bike's rider
				AUnit* pRider = ABikeManager::GetSingleton()->GetRider();

				if ( !pRider )
					return;

				pInventory = TSTATICCAST( AInventoryGroupComponent, pRider->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

				if ( !pInventory )
					return;
			}

			ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

			if ( pPickUpElement )
				pPickUpElement->m_bUnk1 = TFALSE;

			// Top up every item to 10
			const TPString8* apItems[] = {
				&TPS8( apple ),
				&TPS8( pumpkin ),
				&TPS8( melon ),
				&TPS8( lemon ),
				&TPS8( walnut ),
				&TPS8( berrys ),
				&TPS8( egg ),
				&TPS8( honeycomb ),
				&TPS8( carrot ),
				&TPS8( cream ),
				&TPS8( butter ),
				&TPS8( milkcarton ),
				&TPS8( strawberry ),
				&TPS8( cabbage ),
				&TPS8( flour ),
				&TPS8( chocolate ),
				&TPS8( ice ),
				&TPS8( snowdrops ),
				&TPS8( bluebells ),
				&TPS8( foxgloves ),
				&TPS8( cownip ),
				&TPS8( oxeyeddaisy ),
				&TPS8( treasurebells ),
			};

			for ( TINT i = 0; i < TARRAYSIZE( apItems ); i++ )
			{
				TINT iNumMissing = 10 - pInventory->GetNumItemsOfType( *apItems[ i ] );
				pInventory->AddItems( *apItems[ i ], TMath::Max( iNumMissing, 0 ), AInventoryComponentInterface::ms_strDefaultInventory );
			}

			if ( !pPickUpElement )
				break;

			pPickUpElement->m_bUnk1 = TTRUE;
			m_iNumKeys              = 0;
			m_iFirstKey             = 0;
			return;
		}
		case CHEAT_UNLOCK_ALL_BONUS_ITEMS:
			// The two galleries of nine bonus items
			for ( TINT i = 0; i < 9; i++ )
			{
				g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_ART0 << i );
				g_pPlayerProgress->UnlockFlags( APlayerProgress::UNLOCKFLAGS_MOVIE0 << i );
			}
			break;
		case CHEAT_UNLOCK_ALL_ANTICS:
			for ( TINT i = 4; i <= 10; i++ )
				g_pPlayerProgress->UnlockFlags( BITFLAG( i ) );
			break;
	}

	m_iNumKeys  = 0;
	m_iFirstKey = 0;
}

// $Barnyard: FUNCTION 00421d20
ACheatActivator::STATEMASK ACheatActivator::GetCurrentStateMask()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	// NOTE: these states aren't reconstructed yet, TClass::Find gives TNULL until they are
	static const TClass* s_pFrontEndBonusState      = TClass::Find( "AFrontEndBonusState" );
	static const TClass* s_pFrontEndArtGalleryState = TClass::Find( "AFrontEndArtGalleryState" );

	if ( pState->GetClass() == &TGetClass( ASimState ) || pState->GetClass() == &TGetClass( ABarnExploreMinigameState ) )
		return STATEMASK_ANY | STATEMASK_SIM;

	if ( pState->GetClass() == &TGetClass( AFrontEndMainMenuState2 ) )
		return STATEMASK_ANY | STATEMASK_MAIN_MENU;

	if ( pState->GetClass() == s_pFrontEndBonusState || pState->GetClass() == s_pFrontEndArtGalleryState )
		return STATEMASK_ANY | STATEMASK_BONUS;

	if ( pState->GetClass() == &TGetClass( AFrontEndMiniGameState2 ) )
		return STATEMASK_ANY | STATEMASK_ANTICS;

	return pState->IsA( &TGetClass( AMiniGameState ) ) ? STATEMASK_ANY | STATEMASK_MINIGAME : STATEMASK_ANY | STATEMASK_OTHER;
}

// $Barnyard: FUNCTION 00422940
void ACheatActivator::FindTierRequirements()
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iNumTiers     = 0;
	TCHAR          szName[ 128 ];

	for ( TINT iAct = 1; iAct <= 3; iAct++ )
	{
		for ( TINT iTier = 1; iTier <= pQuestManager->GetNumTiersInAct( iAct - 1 ); iTier++ )
		{
			T2String8::Format( szName, "Act %d Tier %d - Complete", iAct, iTier );

			TINT iRequirement = AInteractionManager::GetSingleton()->GetRequirementIndex( TPString8( szName ) );

			m_paiTierRequirements[ pQuestManager->GetAbsQuestTier( iAct, iTier ) - 1 ] = iRequirement;
			iNumTiers++;
		}
	}

	for ( TINT i = 0; i < 3; i++ )
	{
		T2String8::Format( szName, "Act %d - Complete", i + 1 );
		m_paiTierRequirements[ iNumTiers + i ] = AInteractionManager::GetSingleton()->GetRequirementIndex( TPString8( szName ) );
	}
}

// $Barnyard: FUNCTION 00422b50
void ACheatActivator::OnInteractionManagerEvent( const AInteractionManagerEvent& a_rcEvent )
{
	if ( a_rcEvent.GetType() & AInteractionManagerEvent::TYPE_INTERACTIONS_LOADED )
	{
		FindTierRequirements();

		const TCHAR* aRequirements[ REQUIREMENT_NUMOF ] = {
			"Cooking Cheat Available",
			"Duke (Night) Barn Pool Given",
			"Act 3 - Complete",
			"Act 1 Tier 0 - Complete",
			"Player has Gate Key",
			"secondary_has_bike_key",
			"instance_rock_close_to_secondary",
		};

		for ( TINT i = 0; i < REQUIREMENT_NUMOF; i++ )
			m_aiRequirements[ i ] = AInteractionManager::GetSingleton()->GetRequirementIndex( TPString8( aRequirements[ i ] ) );
	}
}

// $Barnyard: FUNCTION 00422ff0
void ACheatActivator::OnInteractionManagerEventCallback( void* a_pReceiver, const AInteractionManagerEvent& a_rcEvent )
{
	TSTATICCAST( ACheatActivator, a_pReceiver )->OnInteractionManagerEvent( a_rcEvent );
}

#ifdef BARNYARD_COMMUNITY_PATCH

static TBOOL s_bDevCheatEntered = TFALSE;

#  ifndef TOSHI_FINAL

void ACheatActivator::Dev_TriggerCheat( CHEAT a_eCheat )
{
	if ( !IsSingletonCreated() )
		return;

	if ( GetCurrentStateMask() & ms_aCheats[ a_eCheat ].eAllowedStates )
	{
		TINFO( "[debugmenu] cheat %d\n", a_eCheat );
		GetSingleton()->TriggerCheat( a_eCheat );
	}
	else if ( !( ms_aCheats[ a_eCheat ].eAllowedStates & STATEMASK_SIM ) )
	{
		// The unlocks are typed in the menus of the front end, where the debug menu doesn't open: applied without the popup
		TINFO( "[debugmenu] cheat %d (front end cheat, no popup)\n", a_eCheat );
		GetSingleton()->ActivateCheat( a_eCheat );
	}
	else
	{
		TWARN( "[debugmenu] cheat %d only works in the sim\n", a_eCheat );
	}
}

#  endif

TBOOL ACheatActivator::Dev_IsPending()
{
	return !s_bDevCheatEntered && TStringManager::String8FindString( GetCommandLineA(), "-devcheat " ) != TNULL;
}

void ACheatActivator::Dev_Update()
{
	static DWORD s_uiSimStart = 0;

	if ( s_bDevCheatEntered || !IsSingletonCreated() )
		return;

	const TCHAR* szArg = TStringManager::String8FindString( GetCommandLineA(), "-devcheat " );
	if ( !szArg )
		return;

	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( !pState || !pState->IsA( &TGetClass( ASimState ) ) )
	{
		s_uiSimStart = 0;
		return;
	}

	if ( s_uiSimStart == 0 )
		s_uiSimStart = GetTickCount();

	// Give the sim a few seconds to settle
	if ( GetTickCount() - s_uiSimStart < 20000 )
		return;

	s_bDevCheatEntered = TTRUE;
	szArg += TStringManager::String8Length( "-devcheat " );

	TINFO( "-devcheat: entering the cheat\n" );

	for ( ; *szArg != '\0' && *szArg != ' '; szArg++ )
	{
		TCHAR chLetter = *szArg;

		if ( chLetter >= 'a' && chLetter <= 'z' )
			chLetter -= 'a' - 'A';

		if ( chLetter >= 'A' && chLetter <= 'Z' )
			GetSingleton()->AddCheatCodeInput( ms_aCheatKeys[ chLetter - 'A' ] );
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
