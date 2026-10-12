#include "pch.h"
#include "AGameTimeRequirement.h"
#include "Helpers/AGameTimeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005aa940, CreateObject 005aa9d0
// NOTE: GetClass 005aa940, CreateObject 005aa9d0, CreateObjectInPlace 005aaa30
TDEFINE_CLASS( AGameTimeRequirement );

// $Barnyard: FUNCTION 005aa950
AGameTimeRequirement::AGameTimeRequirement()
    : m_oCompare( AInteractionComparison::OPERATOR_ALL )
    , m_iMonth( -1 )
    , m_iDay( -1 )
    , m_iHour( -1 )
    , m_iMinute( -1 )
    , m_iSecond( -1 )
    , m_eDayPhase( -1 )
    , m_fDuration( 0.0f )
{
}

// $Barnyard: FUNCTION 005aa9c0
// $Barnyard: FUNCTION 005aaa70
AGameTimeRequirement::~AGameTimeRequirement()
{
}

// $Barnyard: FUNCTION 005aacb0
TBOOL AGameTimeRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strValue;
	TINT      iValue = 0;

	if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "compare" ) ) )
		m_oCompare.SetOperator( strValue );

	if ( a_pProperties->GetOptionalPropertyValue( iValue, TPS8D( "monthofyear" ) ) )
		m_iMonth = iValue;

	if ( a_pProperties->GetOptionalPropertyValue( iValue, TPS8D( "dayofmonth" ) ) )
		m_iDay = iValue;

	if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "phaseofday" ) ) )
		m_eDayPhase = AGameTimeManager::GetSingleton()->GetDayPhaseFromName( strValue );

	if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "timeofday" ) ) )
	{
		// "hh:mm:ss am" or "hh:mm:ss pm"
		TString8 strTime( strValue.GetString() );
		TString8 strHour   = strTime.Mid( 0, 2 );
		TString8 strMinute = strTime.Mid( 3, 2 );
		TString8 strSecond = strTime.Mid( 6, 2 );

		TBOOL bAM = strTime.Find( "am", 8 ) != -1;

		m_iHour   = T2String8::StringToInt( strHour );
		m_iMinute = T2String8::StringToInt( strMinute );
		m_iSecond = T2String8::StringToInt( strSecond );

		TINT iHour = m_iHour;

		if ( !bAM )
			iHour += 12;

		m_iHour = iHour % 24;
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005aaa90
void AGameTimeRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
	TBOOL             bAllowed         = TTRUE;

	if ( m_eDayPhase != -1 && m_oCompare.eOperator == AInteractionComparison::OPERATOR_EQUAL )
	{
		bAllowed = m_eDayPhase == pGameTimeManager->GetCurrentDayPhase();
	}
	else
	{
		TINT iSeconds = ( m_iHour == -1 ) ? 0 : m_iHour * 3600;
		iSeconds += ( m_iMinute == -1 ) ? 0 : m_iMinute * 60;
		iSeconds += ( m_iSecond == -1 ) ? 0 : m_iSecond;

		// NOTE: the month and the day are only used when they aren't set
		TINT iMonth = ( m_iMonth == -1 ) ? pGameTimeManager->GetMonth() : 0;
		TINT iDay   = ( m_iDay == -1 ) ? pGameTimeManager->GetDay() : 0;

		TFLOAT fStart = TFLOAT( TUINT( 86400 * ( 30 * ( iMonth + 12 * ( pGameTimeManager->GetYear() - 1 ) - 1 ) + iDay - 1 ) ) ) + TFLOAT( iSeconds );
		TFLOAT fEnd   = fStart + m_fDuration;
		TFLOAT fNow   = AGameTimeManager::GAME_TIME_TO_REAL_TIME * pGameTimeManager->GetTotalTime() + 21600.0f;

		switch ( m_oCompare.eOperator )
		{
			case AInteractionComparison::OPERATOR_LESSTHAN:
			case AInteractionComparison::OPERATOR_LESSTHANEQUAL:
				bAllowed = m_oCompare.Compare( fNow, fStart );
				break;
			case AInteractionComparison::OPERATOR_GREATERTHAN:
				bAllowed = m_oCompare.Compare( fNow, fEnd );
				break;
			case AInteractionComparison::OPERATOR_EQUAL:
				// NOTE: the operator is changed for good
				m_oCompare.SetOperator( AInteractionComparison::OPERATOR_LESSTHANEQUAL );
				bAllowed = m_oCompare.Compare( fNow, fEnd );

				if ( bAllowed )
				{
					m_oCompare.SetOperator( AInteractionComparison::OPERATOR_GREATERTHANEQUAL );
					bAllowed &= m_oCompare.Compare( fNow, fStart );
				}

				break;
			case AInteractionComparison::OPERATOR_NOTEQUAL:
				// NOTE: the operator is changed for good
				m_oCompare.SetOperator( AInteractionComparison::OPERATOR_LESSTHAN );
				bAllowed = m_oCompare.Compare( fEnd, fNow );

				if ( bAllowed )
				{
					m_oCompare.SetOperator( AInteractionComparison::OPERATOR_GREATERTHAN );
					bAllowed &= m_oCompare.Compare( fStart, fNow );
				}

				break;
		}
	}

	// NOTE: every role is set, "category" is ignored
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL bValue = ( m_bInvert == TTRUE ) ? !bAllowed : bAllowed;

		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bValue );
	}
}
