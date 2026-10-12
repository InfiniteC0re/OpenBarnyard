#include "pch.h"
#include "AGameTimeActivity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c4da0, CreateObject 005c4dc0, CreateObjectInPlace 005c4e40
TDEFINE_CLASS( AGameTimeActivity );

TPSTRING8_DECLARE( dawn );
TPSTRING8_DECLARE( morning );
TPSTRING8_DECLARE( afternoon );
TPSTRING8_DECLARE( dusk );
TPSTRING8_DECLARE( night );

// Returns the day phase named a_strName, -1 if there's none.
// NOTE: retail has this as an AGameTimeManager method (00546ad0, called on AGameTimeManager::ms_pInstance)
// that isn't reconstructed yet. The names are in the table at 0x007741d8.
static AGameTimeManager::DAYPHASE GetDayPhaseFromName( TPString8 a_strName )
{
	const TPString8* aDayPhaseNames[ AGameTimeManager::DAYPHASE_NUMOF ] = {
		&TPS8( dawn ),
		&TPS8( morning ),
		&TPS8( afternoon ),
		&TPS8( dusk ),
		&TPS8( night ),
	};

	for ( TINT i = 0; i < AGameTimeManager::DAYPHASE_NUMOF; i++ )
	{
		if ( *aDayPhaseNames[ i ] == a_strName )
			return i;
	}

	return -1;
}

// NOTE: inlined in CreateObject
AGameTimeActivity::AGameTimeActivity()
    : m_uiFlags( FLAGS_NONE )
    , m_eDayPhase( AGameTimeManager::DAYPHASE_DAWN )
{
}

// $Barnyard: FUNCTION 005c4db0
// $Barnyard: FUNCTION 005c4eb0
AGameTimeActivity::~AGameTimeActivity()
{
}

// $Barnyard: FUNCTION 005c4f00
TBOOL AGameTimeActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strValue;
	a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "action" ) );

	if ( strValue == TPS8D( "wait" ) )
		m_uiFlags |= FLAGS_WAIT;
	else if ( strValue == TPS8D( "setto" ) )
		m_uiFlags |= FLAGS_SET_TO;
	else if ( strValue == TPS8D( "advancephase" ) )
		m_uiFlags |= FLAGS_ADVANCE_PHASE;

	// NOTE: retail reads "phase" into the same string, the action stays if there's no "phase"
	a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "phase" ) );
	m_eDayPhase = GetDayPhaseFromName( strValue );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c4ed0
AInteractionActivity::RESULT AGameTimeActivity::Update( AInteractionSequence* a_pSequence )
{
	// Waits for the day phase
	if ( m_uiFlags & FLAGS_WAIT )
		return ( m_eDayPhase != AGameTimeManager::GetSingleton()->GetCurrentDayPhase() ) ? RESULT_RUNNING : RESULT_DONE;

	return RESULT_DONE;
}
