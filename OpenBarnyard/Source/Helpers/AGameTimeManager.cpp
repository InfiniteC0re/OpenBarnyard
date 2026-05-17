#include "pch.h"
#include "AGameTimeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGameTimeManager );

// $Barnyard: FUNCTION 005440f0
AGameTimeManager::DateChangeEvent::DateChangeEvent( DATECHANGETYPE a_eType, DAYPHASE a_eCurrDayPhase, DAYPHASE a_ePrevDayPhase, TINT a_iDay, TINT a_iMonth, TINT a_iYear )
{
	m_eUpdateType   = a_eType;
	m_eCurrDayPhase = a_eCurrDayPhase;
	m_ePrevDayPhase = a_ePrevDayPhase;
	m_iDay          = a_iDay;
	m_iMonth        = a_iMonth;
	m_iYear         = a_iYear;
}

// $Barnyard: FUNCTION 00544130
// $Barnyard: FUNCTION 00544140
AGameTimeManager::DateChangeEvent::~DateChangeEvent()
{
}

// $Barnyard: FUNCTION 005462c0
AGameTimeManager::AGameTimeManager()
{
	m_flTotalTime        = 0.0f;
	m_iDateYear          = 1;
	m_iDateMonth         = 1;
	m_iDateDay           = 1;
	m_iDateWeekDay       = 6;
	m_eDayPhase          = DAYPHASE_DAWN;
	m_flDayTime          = REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS;
	m_flSecondTimer      = 0.0f;
	m_bStopUpdate        = TTRUE;
	m_flCurrentPhaseTime = 0.0f;
	m_bSavedState        = TFALSE;
	m_bUpdatePhaseOfDay  = TTRUE;

	m_pDateChangeEmitter = new DateChangeEmitter( this );
}

// $Barnyard: FUNCTION 005469a0
AGameTimeManager::~AGameTimeManager()
{
	if ( m_pDateChangeEmitter )
	{
		delete m_pDateChangeEmitter;
	}
}

// $Barnyard: FUNCTION 00546050
TBOOL AGameTimeManager::Reset()
{
	m_flTotalTime        = 0.0f;
	m_iDateYear          = 1;
	m_iDateMonth         = 1;
	m_iDateDay           = 1;
	m_eDayPhase          = DAYPHASE_DAWN;
	m_iDateWeekDay       = 0;
	m_flSecondTimer      = 0.0f;
	m_bStopUpdate        = TTRUE;
	m_bSavedState        = TTRUE;
	m_flDayTime          = REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS;
	m_bUpdatePhaseOfDay  = TTRUE;

	return TTask::Reset();
}

// $Barnyard: FUNCTION 00546a30
TBOOL AGameTimeManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bStopUpdate )
		return TFALSE;

	if ( m_flSecondTimer >= 1.0f )
		m_flSecondTimer -= 1.0f;

	m_flSecondTimer      += a_fDeltaTime;
	m_flTotalTime        += a_fDeltaTime;
	m_flDayTime          += a_fDeltaTime;
	m_flCurrentPhaseTime += a_fDeltaTime;

	if ( m_bUpdatePhaseOfDay && m_flCurrentPhaseTime > DAYPHASE_DURATION[ m_eDayPhase ] )
	{
		SetPhaseOfDay( ( ( m_eDayPhase + 1 ) % DAYPHASE_NUMOF ), TFALSE, TTRUE );
		UpdateDate();
		return TTRUE;
	}

	UpdateDate();
	return TTRUE;
}

// $Barnyard: FUNCTION 00546090
TBOOL AGameTimeManager::OnSerialize( void* a_pSerializer )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 00546730
TBOOL AGameTimeManager::OnDeserialize( void* a_pSerializer )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 00546000
void AGameTimeManager::PushUpdateState()
{
	m_bSavedState = m_bStopUpdate;
	m_bStopUpdate = TTRUE;
}

// $Barnyard: FUNCTION 00546010
void AGameTimeManager::PopUpdateState()
{
	m_bSavedState = m_bStopUpdate;
	m_bStopUpdate = TFALSE;
}

// $Barnyard: FUNCTION 00546020
void AGameTimeManager::ToggleUpdateState()
{
	if ( m_bStopUpdate )
	{
		if ( !m_bSavedState )
		{
			m_bSavedState = m_bStopUpdate;
			m_bStopUpdate = TFALSE;
			return;
		}
		return;
	}

	if ( m_bSavedState )
	{
		m_bSavedState = TFALSE;
		m_bStopUpdate = TTRUE;
	}
}

// $Barnyard: FUNCTION 005463a0
void AGameTimeManager::SetPhaseOfDay( DAYPHASE a_eDayPhase, TBOOL a_bUpdateTime, TBOOL a_bFireEvent )
{
	DAYPHASE ePrevDayPhase = m_eDayPhase;
	m_eDayPhase            = a_eDayPhase;

	if ( a_bUpdateTime )
	{
		TFLOAT fAccum = 0.0f;

		for ( TINT i = 0; i < a_eDayPhase; i++ )
			fAccum += DAYPHASE_DURATION[ i ];

		TFLOAT fNewDayTime = REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS + fAccum;
		m_flTotalTime      = m_flTotalTime - ( m_flDayTime - fNewDayTime );
		m_flDayTime        = fNewDayTime;
	}

	if ( a_eDayPhase == DAYPHASE_DAWN )
		m_iDateWeekDay = ( m_iDateWeekDay + 1 ) % 7;

	TFLOAT fPhaseTimeLeft = m_flCurrentPhaseTime - DAYPHASE_DURATION[ ePrevDayPhase ];
	if ( fPhaseTimeLeft < 0.0f ) fPhaseTimeLeft = 0.0f;
	m_flCurrentPhaseTime = fPhaseTimeLeft;

	if ( a_bFireEvent )
	{
		DateChangeEvent event( DATECHANGETYPE_DAYPHASE, m_eDayPhase, ePrevDayPhase, m_iDateDay, m_iDateMonth, m_iDateYear );
		m_pDateChangeEmitter->Throw( &event );
	}
}

// $Barnyard: FUNCTION 005464b0
void AGameTimeManager::SetTime( TUINT8 a_ui8Hour, TUINT8 a_ui8Minute, TUINT8 a_ui8ExtraMinutes, TBOOL a_bKeepDayPhase, TBOOL a_bFireEvent )
{
	DAYPHASE ePrevDayPhase = m_eDayPhase;

	TFLOAT fNewDayTime = ( a_ui8Hour * 60 + a_ui8Minute + a_ui8ExtraMinutes ) * 60.0f * REAL_TIME_TO_GAME_TIME;
	m_flTotalTime      = m_flTotalTime - ( m_flDayTime - fNewDayTime );
	m_flDayTime        = fNewDayTime;

	if ( !a_bKeepDayPhase )
	{
		DAYPHASE iPhase = 0;
		TFLOAT   fAccum = 0.0f;

		do
		{
			fAccum += DAYPHASE_DURATION[ iPhase ];

			if ( fNewDayTime < REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS + fAccum )
			{
				m_eDayPhase          = iPhase;
				m_flCurrentPhaseTime = fNewDayTime - ( REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS + ( fAccum - DAYPHASE_DURATION[ iPhase ] ) );
				break;
			}

			iPhase++;
		} while ( iPhase < DAYPHASE_NUMOF );
	}

	if ( a_bFireEvent )
	{
		DateChangeEvent event( DATECHANGETYPE_DAYPHASE, m_eDayPhase, ePrevDayPhase, m_iDateDay, m_iDateMonth, m_iDateYear );
		m_pDateChangeEmitter->Throw( &event );
	}
}

// $Barnyard: FUNCTION 005465e0
void AGameTimeManager::UpdateDate()
{
	if ( m_flDayTime <= DAY_LENGTH_UNITS )
		return;

	m_iDateDay++;

	DateChangeEvent dayEvent( DATECHANGETYPE_DAY, m_eDayPhase, m_eDayPhase, m_iDateDay, m_iDateMonth, m_iDateYear );
	m_pDateChangeEmitter->Throw( &dayEvent );

	if ( m_iDateDay > 30 )
	{
		m_iDateMonth++;
		m_iDateDay = 1;

		DateChangeEvent monthEvent( DATECHANGETYPE_MONTH, m_eDayPhase, m_eDayPhase, 1, m_iDateMonth, m_iDateYear );
		m_pDateChangeEmitter->Throw( &monthEvent );

		if ( m_iDateMonth > 12 )
		{
			m_iDateYear++;
			m_iDateMonth = 1;

			DateChangeEvent yearEvent( DATECHANGETYPE_YEAR, m_eDayPhase, m_eDayPhase, m_iDateDay, 1, m_iDateYear );
			m_pDateChangeEmitter->Throw( &yearEvent );
		}
	}

	m_flDayTime = m_flDayTime - DAY_LENGTH_UNITS;
}

// $Barnyard: FUNCTION 00546130
TBOOL AGameTimeManager::IsDaytime() const
{
	return m_eDayPhase == DAYPHASE_MORNING || m_eDayPhase == DAYPHASE_AFTERNOON;
}

// $Barnyard: FUNCTION 00546150
TBOOL AGameTimeManager::IsItNight() const
{
	return m_eDayPhase == DAYPHASE_NIGHT;
}

// $Barnyard: FUNCTION 00546160
TINT AGameTimeManager::GetWeekDay() const
{
	return m_iDateWeekDay;
}

// $Barnyard: FUNCTION 00546170
TFLOAT AGameTimeManager::GetCurrentPhaseTimeLeft() const
{
	TFLOAT fLeft = DAYPHASE_DURATION[ m_eDayPhase ] - m_flCurrentPhaseTime;

	if ( fLeft < 0.0f )
		fLeft = 0.0f;

	return fLeft;
}

// $Barnyard: FUNCTION 0043cd30
TINT AGameTimeManager::GetCurrentMinute() const
{
	TFLOAT fSeconds = GAME_TIME_TO_REAL_TIME * m_flDayTime;

	if ( SECONDS_PER_DAY < fSeconds )
		fSeconds -= SECONDS_PER_DAY;

	return ( SECONDS_PER_DAY % SECONDS_PER_HOUR ) / 60;
}
