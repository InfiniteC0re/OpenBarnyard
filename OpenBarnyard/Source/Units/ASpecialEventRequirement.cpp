#include "pch.h"
#include "ASpecialEventRequirement.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/ASimPersistData.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AHintManager.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"
#include "MiniGame/ABarnExploreMinigameState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005afc80, CreateObject 005afca0
// NOTE: GetClass 005afc80, CreateObject 005afca0, CreateObjectInPlace 005afd10
TDEFINE_CLASS( ASpecialEventRequirement );

// $Barnyard: FUNCTION 005afc90
// $Barnyard: FUNCTION 005afd80
ASpecialEventRequirement::~ASpecialEventRequirement()
{
}

// $Barnyard: FUNCTION 005afda0
TBOOL ASpecialEventRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strType;
	a_pProperties->GetOptionalPropertyValue( strType, TPS8D( "type" ) );

	if ( strType == TPS8D( "player_in_barn" ) )
		m_eEvent = EVENT_PLAYER_IN_BARN;
	else if ( strType == TPS8D( "radio_groundfloor_on" ) )
		m_eEvent = EVENT_RADIO_GROUNDFLOOR_ON;
	else if ( strType == TPS8D( "radio_basement_on" ) )
		m_eEvent = EVENT_RADIO_BASEMENT_ON;
	else if ( strType == TPS8D( "got_apple_pie_rock" ) )
		m_eEvent = EVENT_GOT_APPLE_PIE_ROCK;
	else if ( strType == TPS8D( "super_squirt_available" ) )
		m_eEvent = EVENT_SUPER_SQUIRT_AVAILABLE;
	else if ( strType == TPS8D( "can_enter_kitchen" ) )
		m_eEvent = EVENT_CAN_ENTER_KITCHEN;
	else if ( strType == TPS8D( "player_is_champion" ) )
		m_eEvent = EVENT_PLAYER_IS_CHAMPION;
	else if ( strType == TPS8D( "player_is_hero" ) )
		m_eEvent = EVENT_PLAYER_IS_HERO;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b02e0
void ASpecialEventRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bAllowed;

	switch ( m_eEvent )
	{
		case EVENT_RADIO_GROUNDFLOOR_ON:
			// The music of the barn's ground floor (the wurlitzer)
			bAllowed = ABarnExploreMinigameState::ms_bWurlitzerOn;
			break;
		case EVENT_RADIO_BASEMENT_ON:
			bAllowed = ABarnExploreMinigameState::ms_bRadioOn;
			break;
		case EVENT_PLAYER_IN_BARN:
			bAllowed = AGameLoader::IsBarnExploreMiniGame();
			break;
		case EVENT_GOT_APPLE_PIE_ROCK:
			// NOTE: retail calls a one-line getter of the first recipe (0041cfe0, in the ignorelist)
			bAllowed = AHintManager::GetRecipes()[ 0 ];
			break;
		case EVENT_CAN_ENTER_KITCHEN:
			bAllowed = CanEnterKitchen();
			break;
		case EVENT_SUPER_SQUIRT_AVAILABLE:
			bAllowed = IsSuperSquirtAvailable();
			break;
		case EVENT_PLAYER_IS_CHAMPION:
			bAllowed = ASimPersistData::GetSingleton()->GetRank() == 1;
			break;
		case EVENT_PLAYER_IS_HERO:
			bAllowed = APlayerProgress::GetSingleton()->IsHero();
			break;
		default:
			bAllowed = TFALSE;
			break;
	}

	// NOTE: every role is set, "category" is ignored
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL bValue = ( m_bInvert == TTRUE ) ? !bAllowed : bAllowed;

		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bValue );
	}
}

// $Barnyard: FUNCTION 005b0050
TBOOL ASpecialEventRequirement::CanEnterKitchen()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pState->GetClass()->IsA( &TGetClass( ACoinPickupMicroGame ) ) || AGameTimeManager::GetSingleton()->IsItNight() )
		return TFALSE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TPString8      strQuest( "Pig Apple Pie" );

	if ( pQuestManager->IsQuestComplete( strQuest, 1 ) )
		return TTRUE;

	TINT iQuestTier = -1;
	TINT iQuestAct  = -1;
	pQuestManager->FindQuestTier( "Pig Apple Pie", &iQuestAct, &iQuestTier );

	TINT iAct = pQuestManager->GetAct();

	if ( iAct < iQuestAct || ( iAct == iQuestAct && pQuestManager->GetTier() <= iQuestTier ) )
	{
		// The quest's tier isn't over yet, the first three elements have to be done
		AQuestInstance* pInstance = pQuestManager->FindAssignedQuest( strQuest, 1 );
		return pInstance && pInstance->IsElementDone( 0 ) && pInstance->IsElementDone( 1 ) && pInstance->IsElementDone( 2 );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005b0210
TBOOL ASpecialEventRequirement::IsSuperSquirtAvailable()
{
	// The act and the tier after which the super squirt is always available (0x00775214, 0x00775218)
	static TINT s_iAct  = 2;
	static TINT s_iTier = 3;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iAbsTier      = pQuestManager->GetAbsQuestTier( s_iAct, s_iTier );
	TINT           iCurAbsTier   = pQuestManager->GetAbsQuestTier( pQuestManager->GetAct(), pQuestManager->GetTier() );

	return pQuestManager->FindAssignedQuest( TPS8D( "Otis Super Squirt" ), 1 ) != TNULL || iCurAbsTier > iAbsTier;
}
