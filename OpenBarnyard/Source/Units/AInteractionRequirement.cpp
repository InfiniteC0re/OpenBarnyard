#include "pch.h"
#include "AInteractionRequirement.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a6010
TDEFINE_CLASS_NORUNTIME( AInteractionRequirement );

//-----------------------------------------------------------------------------
// AInteractionUnitTypeNames
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a6370
void AInteractionUnitTypeNames::Set( AInteractionSequence::CATEGORY a_eCategory, const TPString8& a_rcName )
{
	if ( a_eCategory == AInteractionSequence::CATEGORY_ALL )
	{
		Set( AInteractionSequence::CATEGORY_PRIMARY, a_rcName );
		Set( AInteractionSequence::CATEGORY_SECONDARY, a_rcName );
		a_eCategory = AInteractionSequence::CATEGORY_AUXILIARY;
	}

	astrNames[ a_eCategory ] = a_rcName;
}

//-----------------------------------------------------------------------------
// AInteractionRequirement
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005a63d0
AInteractionRequirement::AInteractionRequirement()
    : m_uiParticipantsId( TUINT( -1 ) )
    , m_eCategory( AInteractionSequence::CATEGORY_ALL )
    , m_bInvert( TFALSE )
    , m_eForcedState( FORCEDSTATE_NONE )
{
	m_oRoles.Reset();
}

// $Barnyard: FUNCTION 005a6450
// $Barnyard: FUNCTION 005a6600
AInteractionRequirement::~AInteractionRequirement()
{
}

// $Barnyard: FUNCTION 005a64c0
TBOOL AInteractionRequirement::Create( const PBProperties* a_pProperties )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const TCHAR* szName = it->GetName().GetString();

		if ( szName[ 0 ] == 'c' )
		{
			if ( TStringManager::String8Compare( szName, "category" ) == 0 )
				m_eCategory = AInteractionSequence::GetCategoryFromName( it->GetValue()->GetString() );
		}
		else if ( szName[ 0 ] == 'i' )
		{
			if ( TStringManager::String8Compare( szName, "invert" ) == 0 )
				m_bInvert = it->GetValue()->GetBoolean();
		}
		else if ( szName[ 0 ] == 'n' )
		{
			if ( TStringManager::String8Compare( szName, "name" ) == 0 )
				m_strName = it->GetValue()->GetTPString8();
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a7940
void AInteractionRequirement::SetUnitAllowed( AInteractionSequence::CATEGORY a_eCategory, TUINT a_uiUnit, TBOOL a_bAllowed )
{
	if ( m_bInvert == TTRUE )
		a_bAllowed = !a_bAllowed;

	switch ( a_eCategory )
	{
		case AInteractionSequence::CATEGORY_PRIMARY:
		case AInteractionSequence::CATEGORY_SECONDARY:
		case AInteractionSequence::CATEGORY_AUXILIARY:
			m_oRoles.SetUnit( a_eCategory, a_uiUnit, a_bAllowed );
			break;
		case AInteractionSequence::CATEGORY_ALL:
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, a_uiUnit, a_bAllowed );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, a_uiUnit, a_bAllowed );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, a_uiUnit, a_bAllowed );
			break;
	}
}

// $Barnyard: FUNCTION 0059e540
TBOOL AInteractionComparison::Compare( TFLOAT a_fValue, TFLOAT a_fReference ) const
{
	switch ( eOperator )
	{
		case OPERATOR_ALL:
			return TTRUE;
		case OPERATOR_LESSTHAN:
			return a_fValue < a_fReference;
		case OPERATOR_LESSTHANEQUAL:
			return a_fValue <= a_fReference;
		case OPERATOR_EQUAL:
			return a_fValue == a_fReference;
		case OPERATOR_NOTEQUAL:
			return a_fValue != a_fReference;
		case OPERATOR_GREATERTHAN:
			return a_fValue > a_fReference;
		case OPERATOR_GREATERTHANEQUAL:
			return a_fValue >= a_fReference;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0059e4a0
TBOOL AInteractionComparison::Compare( TINT a_iValue, TINT a_iReference ) const
{
	switch ( eOperator )
	{
		case OPERATOR_ALL:
			return TTRUE;
		case OPERATOR_LESSTHAN:
			return a_iValue < a_iReference;
		case OPERATOR_LESSTHANEQUAL:
			return a_iValue <= a_iReference;
		case OPERATOR_EQUAL:
			return a_iValue == a_iReference;
		case OPERATOR_NOTEQUAL:
			return a_iValue != a_iReference;
		case OPERATOR_GREATERTHAN:
			return a_iValue > a_iReference;
		case OPERATOR_GREATERTHANEQUAL:
			return a_iValue >= a_iReference;
	}

	return TFALSE;
}

TPSTRING8_DECLARE( all );
TPSTRING8_DECLARE( lessthan );
TPSTRING8_DECLARE( lessthanequal );
TPSTRING8_DECLARE( equal );
TPSTRING8_DECLARE( notequal );
TPSTRING8_DECLARE( greaterthan );
TPSTRING8_DECLARE( greaterthanequal );

// $Barnyard: FUNCTION 0059e5f0
TBOOL AInteractionComparison::SetOperator( const TPString8& a_rcName )
{
	// The names of the operators (0x007751e8)
	const TPString8* s_apNames[ OPERATOR_NUMOF ] = {
		&TPS8( all ), &TPS8( lessthan ), &TPS8( lessthanequal ), &TPS8( equal ), &TPS8( notequal ), &TPS8( greaterthan ), &TPS8( greaterthanequal )
	};

	for ( TINT i = 0; i < OPERATOR_NUMOF; i++ )
	{
		if ( a_rcName == *s_apNames[ i ] )
		{
			eOperator = OPERATOR( i );
			return TTRUE;
		}
	}

	return TFALSE;
}
