#include "pch.h"
#include "APropagateKnowledgeActivity.h"
#include "AUnitPlayerKnowledgeComponent.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "Player/APlayerKnowledgeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bb8c0, CreateObject 005bbac0, CreateObjectInPlace 005bbb30
TDEFINE_CLASS( APropagateKnowledgeActivity );

// NOTE: inlined into CreateObject 005bbac0 / CreateObjectInPlace 005bbb30
APropagateKnowledgeActivity::APropagateKnowledgeActivity()
{
}

// $Barnyard: FUNCTION 005bb8d0
// $Barnyard: FUNCTION 005bbba0
APropagateKnowledgeActivity::~APropagateKnowledgeActivity()
{
}

// $Barnyard: FUNCTION 005bb940
TBOOL APropagateKnowledgeActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pQuestion = a_pProperties->GetProperty( "question" );

	if ( pQuestion )
		m_strQuestion = pQuestion->GetTPString8();

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bba00
AInteractionActivity::RESULT APropagateKnowledgeActivity::Update( AInteractionSequence* a_pSequence )
{
	TINT   iKnowledge = APlayerKnowledgeManager::GetSingleton()->FindKnowledge( m_strQuestion );
	AUnit* pUnit      = a_pSequence->GetUnit( 1 );

	AUnitPlayerKnowledgeComponent* pKnowledge = TSTATICCAST( AUnitPlayerKnowledgeComponent, pUnit->FindComponentExact( &TGetClass( AUnitPlayerKnowledgeComponent ) ) );

	if ( pKnowledge )
		pKnowledge->SetKnowledge( iKnowledge );

	for ( auto it = a_pSequence->GetCategoryBegin( AInteractionSequence::CATEGORY_AUXILIARY ); it != a_pSequence->GetCategoryEnd( AInteractionSequence::CATEGORY_AUXILIARY ); it++ )
	{
		pUnit = it->GetOwner();

		if ( pUnit )
		{
			pKnowledge = TSTATICCAST( AUnitPlayerKnowledgeComponent, pUnit->FindComponentExact( &TGetClass( AUnitPlayerKnowledgeComponent ) ) );

			if ( pKnowledge )
				pKnowledge->SetKnowledge( iKnowledge );
		}
	}

	return RESULT_DONE;
}
