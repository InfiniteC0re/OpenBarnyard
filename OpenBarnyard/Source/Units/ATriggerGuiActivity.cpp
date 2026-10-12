#include "pch.h"
#include "ATriggerGuiActivity.h"
#include "AInteractionComponent.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c2590, CreateObject 005c25b0, CreateObjectInPlace 005c2630
TDEFINE_CLASS( ATriggerGuiActivity );

TPSTRING8_DECLARE( forceanswertime );

// NOTE: inlined in CreateObject
ATriggerGuiActivity::ATriggerGuiActivity()
    : m_eBehaviour( BEHAVIOUR_ACTIVATE )
    , m_fForceAnswerTime( 0.0f )
{
}

// $Barnyard: FUNCTION 005c25a0
// $Barnyard: FUNCTION 005c26a0
ATriggerGuiActivity::~ATriggerGuiActivity()
{
}

// $Barnyard: FUNCTION 005c2710
TBOOL ATriggerGuiActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pBehaviour = a_pProperties->GetProperty( "behaviour" );

	if ( pBehaviour )
	{
		if ( TStringManager::String8CompareNoCase( pBehaviour->GetTPString8().GetString(), "activate" ) == 0 )
			m_eBehaviour = BEHAVIOUR_ACTIVATE;
		else if ( TStringManager::String8CompareNoCase( pBehaviour->GetTPString8().GetString(), "deactivate" ) == 0 )
			m_eBehaviour = BEHAVIOUR_DEACTIVATE;
	}

	// NOTE: retail calls the TPString8 overload (006b1f50)
	a_pProperties->GetOptionalFloatOrIntValue( m_fForceAnswerTime, TPS8( forceanswertime ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c2840
AInteractionActivity::RESULT ATriggerGuiActivity::Update( AInteractionSequence* a_pSequence )
{
	auto it = a_pSequence->GetCategoryBegin( m_eCategory );

	if ( it != a_pSequence->GetCategoryEnd( m_eCategory ) )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			ATriggerGuiEvent oEvent;
			oEvent.iUnk0            = 0;
			oEvent.eBehaviour       = m_eBehaviour;
			oEvent.pUnit            = pUnit;
			oEvent.fForceAnswerTime = m_fForceAnswerTime;

			TGlobalEmitter<ATriggerGuiEvent>::Throw( oEvent );
		}
	}

	return RESULT_DONE;
}
