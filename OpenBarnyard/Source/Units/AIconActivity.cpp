#include "pch.h"
#include "AIconActivity.h"
#include "AUnit.h"
#include "AUnitNameManager.h"
#include "AAnimalPopulationManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b80c0, CreateObject 005b82e0, CreateObjectInPlace 005b8360
TDEFINE_CLASS( AIconActivity );

TPSTRING8_DECLARE( action );
TPSTRING8_DECLARE( exclamationremove );
TPSTRING8_DECLARE( exclamationadd );
TPSTRING8_DECLARE( questionremove );
TPSTRING8_DECLARE( questionadd );
TPSTRING8_DECLARE( unitname );

// NOTE: inlined into CreateObject
AIconActivity::AIconActivity()
    : m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005b80d0
// $Barnyard: FUNCTION 005b83d0
AIconActivity::~AIconActivity()
{
}

// $Barnyard: FUNCTION 005b8140
TBOOL AIconActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strAction;

	if ( a_pProperties->GetOptionalPropertyValue( strAction, TPS8( action ) ) )
	{
		if ( strAction == TPS8( exclamationremove ) )
			m_uiFlags |= FLAGS_EXCLAMATIONREMOVE;
		else if ( strAction == TPS8( exclamationadd ) )
			m_uiFlags |= FLAGS_EXCLAMATIONADD;
		else if ( strAction == TPS8( questionremove ) )
			m_uiFlags |= FLAGS_QUESTIONREMOVE;
		else if ( strAction == TPS8( questionadd ) )
			m_uiFlags |= FLAGS_QUESTIONADD;
	}

	if ( a_pProperties->GetOptionalPropertyValue( m_strUnitName, TPS8( unitname ) ) )
		m_uiFlags |= FLAGS_UNITNAME;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b8220
AInteractionActivity::RESULT AIconActivity::Update( AInteractionSequence* a_pSequence )
{
	AUnit* pUnit;

	if ( m_uiFlags & FLAGS_UNITNAME )
		pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( m_strUnitName );
	else
		pUnit = a_pSequence->GetUnitByCategory( m_eCategory );

	// NOTE: "exclamationadd" makes the activity do nothing
	if ( pUnit && !( m_uiFlags & FLAGS_EXCLAMATIONADD ) )
	{
		AAnimalPopulationManager* pPopulationManager = AAnimalPopulationManager::GetSingleton();

		if ( m_uiFlags & FLAGS_EXCLAMATIONREMOVE )
		{
			pPopulationManager->RemoveMarker( pUnit );
			return RESULT_DONE;
		}

		if ( m_uiFlags & FLAGS_QUESTIONADD )
		{
			pPopulationManager->SetMarker( pUnit, AAnimalPopulationManager::MARKERTYPE_QUESTION, TFALSE );
			return RESULT_DONE;
		}

		if ( m_uiFlags & FLAGS_QUESTIONREMOVE )
		{
			pPopulationManager->RemoveMarker( pUnit );

			ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

			if ( pMiniMap )
				pMiniMap->RemoveUnitIcon( pUnit->GetSelfProxy() );
		}
	}

	return RESULT_DONE;
}
