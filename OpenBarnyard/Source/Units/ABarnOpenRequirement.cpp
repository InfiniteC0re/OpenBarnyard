#include "pch.h"
#include "ABarnOpenRequirement.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Quests/AQuestManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a8080, CreateObject 005a80a0
// NOTE: GetClass 005a8080, CreateObject 005a80a0, CreateObjectInPlace 005a8100
TDEFINE_CLASS( ABarnOpenRequirement );

// $Barnyard: FUNCTION 005a8090
// $Barnyard: FUNCTION 005a8150
ABarnOpenRequirement::~ABarnOpenRequirement()
{
}

// $Barnyard: FUNCTION 005a8310
void ABarnOpenRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	// NOTE: only the weight is set
	TFLOAT fWeight = IsBarnOpen( -1 ) ? 1.0f : 0.0f;

	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}

// $Barnyard: FUNCTION 005a8170
TBOOL ABarnOpenRequirement::IsBarnOpenAtNight( TINT a_iAbsTier )
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	TINT iQuestAct;
	TINT iQuestTier;
	pQuestManager->FindQuestTier( AQuestManager::GetBarnDetailingQuest(), &iQuestAct, &iQuestTier );

	TINT iBarnDetailingTier = pQuestManager->GetAbsQuestTier( iQuestAct, iQuestTier );
	TINT iAbsTier           = pQuestManager->GetAbsQuestTier( pQuestManager->GetAct(), pQuestManager->GetTier() );

	if ( a_iAbsTier != -1 )
		iAbsTier = a_iAbsTier;

	if ( iAbsTier <= iBarnDetailingTier )
		return TFALSE;

	pQuestManager->FindQuestTier( AQuestManager::GetFlowersForBenQuest(), &iQuestAct, &iQuestTier );

	if ( iAbsTier >= pQuestManager->GetAbsQuestTier( iQuestAct, iQuestTier ) )
	{
		pQuestManager->FindQuestTier( AQuestManager::GetRoundUpChicksQuest(), &iQuestAct, &iQuestTier );

		if ( iAbsTier <= pQuestManager->GetAbsQuestTier( iQuestAct, iQuestTier ) )
			return TFALSE;
	}

	iQuestAct  = -1;
	iQuestTier = -1;
	pQuestManager->FindQuestTier( AQuestManager::GetCoyoteTippingQuest(), &iQuestAct, &iQuestTier );

	return iAbsTier != pQuestManager->GetAbsQuestTier( iQuestAct, iQuestTier );
}

// $Barnyard: FUNCTION 005a82a0
TBOOL ABarnOpenRequirement::IsBarnOpen( TINT a_iAbsTier )
{
	AGameState* pState      = AGameStateController::GetSingleton()->GetCurrentState();
	TBOOL       bCoinPickup = pState->GetClass()->IsA( &TGetClass( ACoinPickupMicroGame ) );

	if ( AGameLoader::IsBarnExploreMiniGame() )
		return !bCoinPickup;

	return !bCoinPickup && ( AGameTimeManager::GetSingleton()->GetCurrentDayPhase() != AGameTimeManager::DAYPHASE_NIGHT || IsBarnOpenAtNight( a_iAbsTier ) );
}
