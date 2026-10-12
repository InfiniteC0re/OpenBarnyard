#include "pch.h"
#include "AGopherOpenRequirement.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameTimeManager.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ab980, CreateObject 005ab9a0
// NOTE: GetClass 005ab980, CreateObject 005ab9a0, CreateObjectInPlace 005aba00
TDEFINE_CLASS( AGopherOpenRequirement );

// $Barnyard: FUNCTION 005ab990
// $Barnyard: FUNCTION 005aba50
AGopherOpenRequirement::~AGopherOpenRequirement()
{
}

// $Barnyard: FUNCTION 005abbd0
void AGopherOpenRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	// NOTE: only the weight is set
	if ( IsGopherShopOpen() )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}

// $Barnyard: FUNCTION 005aba70
TBOOL AGopherOpenRequirement::IsGopherShopOpen()
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( pState->GetClass()->IsA( &TGetClass( ACoinPickupMicroGame ) ) || AGameTimeManager::GetSingleton()->GetCurrentDayPhase() != AGameTimeManager::DAYPHASE_NIGHT )
		return TFALSE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	TINT iQuestAct;
	TINT iQuestTier;
	pQuestManager->FindQuestTier( AQuestManager::GetBarnDetailingQuest(), &iQuestAct, &iQuestTier );

	TINT iQuestAbsTier = pQuestManager->GetAbsQuestTier( iQuestAct, iQuestTier );
	TINT iCurAbsTier   = pQuestManager->GetAbsQuestTier( pQuestManager->GetAct(), pQuestManager->GetTier() );

	AQuestInstance* pInstance = pQuestManager->FindAssignedQuest( TPString8( AQuestManager::GetBarnDetailingQuest() ), 1 );

	// Open once the first element of the quest is done or its tier is over
	if ( pInstance )
		return pInstance->IsElementDone( 0 );

	return iCurAbsTier > iQuestAbsTier;
}
