#include "pch.h"
#include "ASuggestionActivity.h"
#include "AUnit.h"
#include "AAnimalAttributes.h"
#include "AUnitBehaviour.h"

#include <Plugins/PPropertyParser/PPropertyValueArray.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c1e60, CreateObject 005c2150, CreateObjectInPlace 005c21d0
TDEFINE_CLASS( ASuggestionActivity );

// NOTE: inlined in CreateObject, m_fDriveStrength is left uninitialised
ASuggestionActivity::ASuggestionActivity()
    : m_pFsmParams( TNULL )
{
}

// $Barnyard: FUNCTION 005c1e70
// $Barnyard: FUNCTION 005c2240
ASuggestionActivity::~ASuggestionActivity()
{
	if ( m_pFsmParams )
		delete m_pFsmParams;
}

// $Barnyard: FUNCTION 005c1f00
TBOOL ASuggestionActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pFsm = a_pProperties->GetProperty( "fsm" );

	if ( pFsm )
		m_strFsm = pFsm->GetTPString8();

	const PBPropertyValue* pFsmParams = a_pProperties->GetProperty( "fsmparams" );

	if ( !pFsmParams )
	{
		m_pFsmParams = TNULL;
	}
	else
	{
		// NOTE: retail doesn't delete the previous params
		m_pFsmParams = PBPropertyReader::ToValueArray( pFsmParams->GetArray() );
	}

	const PBPropertyValue* pDriveStrength = a_pProperties->GetProperty( "drivestrength" );

	if ( pDriveStrength )
		m_fDriveStrength = ( pDriveStrength->GetType() == PBPropertyValue::Type::Float ) ? pDriveStrength->GetFloat() : TFLOAT( pDriveStrength->GetInteger() );

	return AInteractionActivityBoolean::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c2000
AInteractionActivity::RESULT ASuggestionActivity::Update( AInteractionSequence* a_pSequence )
{
	auto it = a_pSequence->GetCategoryBegin( m_eCategory );

	if ( it != a_pSequence->GetCategoryEnd( m_eCategory ) )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			TBOOL bSuggested = TFALSE;

			AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

			if ( pAttributes )
				pAttributes->SetAttribute( AAnimalAttributes::GetAttributeFromName( TPS8D( "suggestion" ).GetString() ), m_fDriveStrength );

			AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
			// Retail expects the unit to have a behaviour
			if ( pBehaviour )
#endif
				bSuggested = pBehaviour->SetSuggestion( m_strFsm, m_pFsmParams, TFALSE );

			if ( bSuggested == TTRUE )
				UpdateTrue( a_pSequence );
			else
				UpdateFalse( a_pSequence );
		}
	}

	return RESULT_DONE;
}
