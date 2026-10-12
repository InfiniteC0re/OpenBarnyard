#include "pch.h"
#include "AQuestActivity.h"
#include "AUnit.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"
#include "Quests/AQuest.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bbbc0, CreateObject 005bbfa0, CreateObjectInPlace 005bc020
TDEFINE_CLASS( AQuestActivity );

TPSTRING8_DECLARE( name );
TPSTRING8_DECLARE( forcestate );
TPSTRING8_DECLARE( thank );
TPSTRING8_DECLARE( thankonbehalf );

// NOTE: inlined into CreateObject 005bbfa0 / CreateObjectInPlace 005bc020
AQuestActivity::AQuestActivity()
    : m_eFlags( FLAGS_NONE )
    , m_eForceState( FORCESTATE_NONE )
{
}

// $Barnyard: FUNCTION 005bbbf0
// $Barnyard: FUNCTION 005bc090
AQuestActivity::~AQuestActivity()
{
}

// $Barnyard: FUNCTION 005bbc60
TBOOL AQuestActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strName;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const PBPropertyValue* pValue = it->GetValue();
		strName                       = TPString8( it->GetName().GetString() );

		if ( strName == TPS8( name ) )
		{
			m_strName = pValue->GetTPString8();
		}
		else if ( strName == TPS8( forcestate ) )
		{
			const TCHAR* szState = pValue->GetString();

			if ( TStringManager::String8Compare( szState, "completed" ) == 0 )
				m_eForceState = FORCESTATE_COMPLETED;
			else if ( TStringManager::String8Compare( szState, "failed" ) == 0 )
				m_eForceState = FORCESTATE_FAILED;
		}
		else if ( strName == TPS8( thank ) )
		{
			if ( pValue->GetRaw() )
				m_eFlags |= FLAGS_THANK;
		}
		else if ( strName == TPS8( thankonbehalf ) )
		{
			if ( pValue->GetRaw() )
				m_eFlags |= FLAGS_THANK_ON_BEHALF;
		}
	}

	// The quest is given by the primary unit unless it's a thank you
	if ( ( m_eFlags & ( FLAGS_THANK | FLAGS_THANK_ON_BEHALF ) ) == 0 )
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bbe60
AInteractionActivity::RESULT AQuestActivity::Update( AInteractionSequence* a_pSequence )
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( m_eForceState == FORCESTATE_NONE )
	{
		// Assign the quest, given by the unit of the category
		if ( ( m_eFlags & ( FLAGS_THANK | FLAGS_THANK_ON_BEHALF ) ) == 0 )
			pQuestManager->AssignQuest( a_pSequence->GetUnit( m_eCategory ), m_strName );
	}
	else
	{
		AUnit* pUnit = TNULL;

		if ( m_eCategory != AInteractionSequence::CATEGORY_ALL )
			pUnit = a_pSequence->GetUnit( m_eCategory );

		// The last assigned quest called m_strName or given by the unit
		AQuestInstance* pInstance = TNULL;

		for ( TINT i = 0; i < pQuestManager->GetNumAssignedQuests(); i++ )
		{
			AQuestInstance* pAssigned = pQuestManager->GetAssignedQuest( i );

			if ( TStringManager::String8Compare( pAssigned->GetQuest()->GetName().GetString(), m_strName.GetString() ) == 0 ||
			     ( pUnit && pAssigned->GetQuestGiver() == pUnit ) )
			{
				pInstance = pAssigned;
			}
		}

		if ( pInstance )
		{
			pInstance->SetElementsState( m_eForceState );
			return RESULT_DONE;
		}
	}

	return RESULT_DONE;
}
