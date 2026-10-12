#include "pch.h"
#include "ATierActRequirement.h"
#include "Quests/AQuestManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b0e80, CreateObject 005b0ee0
// NOTE: GetClass 005b0e80, CreateObject 005b0ee0, CreateObjectInPlace 005b0f40
TDEFINE_CLASS( ATierActRequirement );

TPSTRING8_DECLARE( act );
TPSTRING8_DECLARE( tier );

// $Barnyard: FUNCTION 005b0e90
// $Barnyard: FUNCTION 005b0fa0
ATierActRequirement::~ATierActRequirement()
{
}

// $Barnyard: FUNCTION 005b0ea0
TBOOL ATierActRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_iAct, TPS8( act ) );
	a_pProperties->GetOptionalPropertyValue( m_iTier, TPS8( tier ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b0fc0
void ATierActRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();
	TINT           iAct          = pQuestManager->GetAct();
	TBOOL          bPast         = iAct > m_iAct || ( iAct == m_iAct && pQuestManager->GetTier() > m_iTier );

	// NOTE: every role is set, "category" is ignored
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL bValue = ( m_bInvert == TTRUE ) ? !bPast : bPast;

		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bValue );
	}
}
