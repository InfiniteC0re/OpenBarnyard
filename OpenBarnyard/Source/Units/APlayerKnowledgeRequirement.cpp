#include "pch.h"
#include "APlayerKnowledgeRequirement.h"
#include "AUnit.h"
#include "AUnitPlayerKnowledgeComponent.h"
#include "Player/APlayerKnowledgeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ade30, CreateObject 005ae320, CreateObjectInPlace 005ae3a0
TDEFINE_CLASS( APlayerKnowledgeRequirement );

// $Barnyard: FUNCTION 005adef0
// $Barnyard: FUNCTION 005ae410
APlayerKnowledgeRequirement::~APlayerKnowledgeRequirement()
{
}

// $Barnyard: FUNCTION 005adf90
TBOOL APlayerKnowledgeRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pQuestion = a_pProperties->GetProperty( "question" );

	if ( pQuestion )
		m_strQuestion = pQuestion->GetTPString8();

	APlayerKnowledgeManager* pKnowledgeManager = APlayerKnowledgeManager::GetSingleton();

	if ( pKnowledgeManager->FindKnowledge( m_strQuestion ) == -1 )
		pKnowledgeManager->AddKnowledge( m_strQuestion );

	const PBPropertyValue* pAnswer = a_pProperties->GetProperty( "answer" );

	if ( pAnswer )
		m_strAnswer = pAnswer->GetTPString8();

	if ( m_strAnswer == TPS8D( "any" ) )
		m_bAny = TTRUE;
	else if ( m_strAnswer == TPS8D( "none" ) )
		m_bNone = TTRUE;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ade40
TBOOL APlayerKnowledgeRequirement::HasAnswer( TPString8 a_strName )
{
	return !APlayerKnowledgeManager::GetSingleton()->GetKnowledgeValue( a_strName ).IsEmpty();
}

// $Barnyard: FUNCTION 005ae1b0
void APlayerKnowledgeRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	APlayerKnowledgeManager* pKnowledgeManager = APlayerKnowledgeManager::GetSingleton();

	TBOOL bAnswered = HasAnswer( m_strQuestion );
	TINT  iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit = a_pParticipants->Get( i );

		if ( m_bAny == TTRUE )
		{
			SetUnitAllowed( m_eCategory, i, bAnswered == TTRUE );
		}
		else if ( m_bNone == TTRUE )
		{
			SetUnitAllowed( m_eCategory, i, bAnswered == TFALSE );
		}
		else if ( bAnswered )
		{
			// The unit has to know the answer, and it has to be the right one
			TINT                           iKnowledge = pKnowledgeManager->FindKnowledge( m_strQuestion );
			AUnitPlayerKnowledgeComponent* pKnowledge = TSTATICCAST( AUnitPlayerKnowledgeComponent, pUnit->FindComponentExact( &TGetClass( AUnitPlayerKnowledgeComponent ) ) );
			TBOOL                          bAllowed   = TFALSE;

			if ( pKnowledge )
			{
				TBOOL bKnows      = pKnowledge->HasKnowledge( iKnowledge );
				TBOOL bRightValue = m_strAnswer == pKnowledgeManager->GetKnowledgeValue( m_strQuestion );
				bAllowed          = bKnows && bRightValue;
			}

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
		else
		{
			SetUnitAllowed( m_eCategory, i, TFALSE );
		}
	}
}
