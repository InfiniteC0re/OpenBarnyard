#include "pch.h"
#include "AQuestElementActivity.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bc0b0, CreateObject 005bc260, CreateObjectInPlace 005bc2e0
TDEFINE_CLASS( AQuestElementActivity );

// NOTE: inlined into CreateObject 005bc260 / CreateObjectInPlace 005bc2e0
// NOTE: retail leaves m_bCompleted uninitialised
AQuestElementActivity::AQuestElementActivity()
    : m_iElement( 1 )
    , m_iQuestInstance( 1 )
    , m_bCompleted( TFALSE )
{
}

// $Barnyard: FUNCTION 005bc0f0
// $Barnyard: FUNCTION 005bc360
AQuestElementActivity::~AQuestElementActivity()
{
}

// $Barnyard: FUNCTION 005bc160
TBOOL AQuestElementActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pQuest = a_pProperties->GetProperty( "quest" );

	if ( pQuest )
		m_strQuest = pQuest->GetTPString8();

	const PBPropertyValue* pQuestInstance = a_pProperties->GetProperty( "questinstancenum" );

	if ( pQuestInstance )
		m_iQuestInstance = TINT( TREINTERPRETCAST( TUINTPTR, pQuestInstance->GetRaw() ) );

	const PBPropertyValue* pElement = a_pProperties->GetProperty( "elementnum" );

	if ( pElement )
		m_iElement = TINT( TREINTERPRETCAST( TUINTPTR, pElement->GetRaw() ) );

	const PBPropertyValue* pCompleted = a_pProperties->GetProperty( "completed" );

	if ( pCompleted )
		m_bCompleted = pCompleted->GetRaw() != TNULL;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bc0c0
AInteractionActivity::RESULT AQuestElementActivity::Update( AInteractionSequence* a_pSequence )
{
	AQuestInstance* pQuest = AQuestManager::GetSingleton()->FindAssignedQuest( m_strQuest, m_iQuestInstance );

	if ( pQuest )
		pQuest->SetElementState( m_iElement - 1, AQuestInstance::ELEMENTSTATE_COMPLETE, TTRUE );

	return RESULT_DONE;
}
