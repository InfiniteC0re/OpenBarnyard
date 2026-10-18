#include "pch.h"
#include "AGameTimeManager.h"
#include "FX/AGameTimeFXManager.h"
#include "FSM/AMethodPointer.h"
#include "Render/ADebugOverlay.h"

#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00545fe0, CreateObject 00546900, CreateObjectInPlace 00546960
TDEFINE_CLASS_INIT( AGameTimeManager, 1, 0 );

// 0x00783d90
AFunctionTable AGameTimeManager::ms_oFunctionTable( &TGetClass( AGameTimeManager ), TNULL );

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 005462b0
static TObject* __stdcall AGameTimeManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AGameTimeManager::GetSingleton();
}

// $Barnyard: FUNCTION 00546cd0
void AGameTimeManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AGameTimeManagerFunctionFallback ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AGameTimeManager, TPString8>::Create( "IsCurPhase", &AGameTimeManager::IsCurPhase ) );
}

// $Barnyard: FUNCTION 00545ff0
void AGameTimeManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00546390
AFunctionTable* AGameTimeManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

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
	m_iDateWeekDay       = WEEKDAY_SUNDAY;
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
// $Barnyard: FUNCTION 00546a10
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
	m_flTotalTime       = 0.0f;
	m_iDateYear         = 1;
	m_iDateMonth        = 1;
	m_iDateDay          = 1;
	m_eDayPhase         = DAYPHASE_DAWN;
	m_iDateWeekDay      = WEEKDAY_MONDAY;
	m_flSecondTimer     = 0.0f;
	m_bStopUpdate       = TTRUE;
	m_bSavedState       = TTRUE;
	m_flDayTime         = REAL_TIME_TO_GAME_TIME * DAY_START_SECONDS;
	m_bUpdatePhaseOfDay = TTRUE;

	return TTask::Reset();
}

// $Barnyard: FUNCTION 00546a30
TBOOL AGameTimeManager::OnUpdate( TFLOAT a_fDeltaTime )
{
#ifdef ADEBUGOVERLAY
	// Development: the time on the debug overlay (once a frame, "-devtimescale" updates the time again)
	static TUINT s_uiPrintFrame = 0;

	if ( ADebugOverlay::IsPrinting() && s_uiPrintFrame != g_oSystemManager.GetFrameCount() )
	{
		s_uiPrintFrame = g_oSystemManager.GetFrameCount();

		ADEBUGLINE( LINE_GAME_TIME, ADebugOverlay::INFO_COLOUR, "Total Actual Time = %.3f", GetTotalTime() );
		ADEBUGLINE( LINE_GAME_TIME, ADebugOverlay::INFO_COLOUR, "Time of Day: %.3f", GetDayTime() );
		ADEBUGLINE( LINE_GAME_TIME, ADebugOverlay::INFO_COLOUR, "Day/Month/Year: %d/%d/%d", GetDay(), GetMonth(), GetYear() );
		ADEBUGLINE( LINE_GAME_TIME, ADebugOverlay::INFO_COLOUR, "Day Phase: %d", GetCurrentDayPhase() );
		ADEBUGLINE( LINE_GAME_TIME, ADebugOverlay::INFO_COLOUR, "Hour/Minute/Second: %d:%d:%d", GetCurrentHour(), GetCurrentMinute(), GetCurrentSecond() );
	}
#endif

	if ( m_bStopUpdate )
		return TFALSE;

	if ( m_flSecondTimer >= 1.0f )
		m_flSecondTimer -= 1.0f;

	m_flSecondTimer += a_fDeltaTime;
	m_flTotalTime += a_fDeltaTime;
	m_flDayTime += a_fDeltaTime;
	m_flCurrentPhaseTime += a_fDeltaTime;

	if ( m_bUpdatePhaseOfDay && m_flCurrentPhaseTime > DAYPHASE_DURATION[ m_eDayPhase ] )
	{
		ChangePhaseOfDay( ( ( m_eDayPhase + 1 ) % DAYPHASE_NUMOF ), TFALSE, TTRUE );
		UpdateDate();
		return TTRUE;
	}

	UpdateDate();
	return TTRUE;
}

// $Barnyard: FUNCTION 00546090
TBOOL AGameTimeManager::OnSerialize( AHackedQuickSave::Buffer* a_pSerializer )
{
	a_pSerializer->WriteString( "GameTimeManager" );
	a_pSerializer->WriteFloat( m_flTotalTime );
	a_pSerializer->WriteInt( m_iDateYear );
	a_pSerializer->WriteInt( m_iDateMonth );
	a_pSerializer->WriteInt( m_iDateDay );
	a_pSerializer->WriteFloat( m_flDayTime );
	a_pSerializer->WriteFloat( m_flSecondTimer );
	a_pSerializer->WriteInt( m_eDayPhase );
	a_pSerializer->WriteInt( m_iDateWeekDay );
	a_pSerializer->WriteBool( m_bStopUpdate );
	a_pSerializer->WriteFloat( m_flCurrentPhaseTime );
	a_pSerializer->WriteBool( m_bSavedState );

	return TTRUE;
}

// $Barnyard: FUNCTION 00546730
TBOOL AGameTimeManager::OnDeserialize( AHackedQuickSave::Buffer* a_pSerializer )
{
	a_pSerializer->CheckString( "GameTimeManager" );
	a_pSerializer->ReadFloat( m_flTotalTime );
	a_pSerializer->ReadUInt( *TREINTERPRETCAST( TUINT*, &m_iDateYear ) );
	a_pSerializer->ReadUInt( *TREINTERPRETCAST( TUINT*, &m_iDateMonth ) );
	a_pSerializer->ReadUInt( *TREINTERPRETCAST( TUINT*, &m_iDateDay ) );
	a_pSerializer->ReadFloat( m_flDayTime );
	a_pSerializer->ReadFloat( m_flSecondTimer );

	TINT iDayPhase = -1;
	TINT iWeekDay  = -1;
	a_pSerializer->ReadInt( iDayPhase );
	m_eDayPhase = iDayPhase;
	a_pSerializer->ReadInt( iWeekDay );
	m_iDateWeekDay = iWeekDay;

	a_pSerializer->ReadBool( m_bStopUpdate );
	a_pSerializer->ReadFloat( m_flCurrentPhaseTime );
	a_pSerializer->ReadBool( m_bSavedState );

	AGameTimeFXManager::GetSingleton()->SetDayPhase( m_eDayPhase, TTRUE, m_flCurrentPhaseTime );

	return TTRUE;
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
void AGameTimeManager::ChangePhaseOfDay( DAYPHASE a_eDayPhase, TBOOL a_bUpdateTime, TBOOL a_bFireEvent )
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

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 802a9964
void AGameTimeManager::SkipToNextDay()
{
	m_flDayTime = 1201.0f;
	UpdateDate();
}

#endif // BARNYARD_LATEST

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

TPSTRING8_DECLARE( dawn );
TPSTRING8_DECLARE( morning );
TPSTRING8_DECLARE( afternoon );
TPSTRING8_DECLARE( dusk );
TPSTRING8_DECLARE( night );

// $Barnyard: FUNCTION 00546ad0
AGameTimeManager::DAYPHASE AGameTimeManager::GetDayPhaseFromName( TPString8 a_strName ) const
{
	// The names of the day phases (0x007741d8)
	static const TPString8* s_apNames[ DAYPHASE_NUMOF ] = {
		&TPS8( dawn ),
		&TPS8( morning ),
		&TPS8( afternoon ),
		&TPS8( dusk ),
		&TPS8( night ),
	};

	for ( TINT i = 0; i < DAYPHASE_NUMOF; i++ )
	{
		if ( *s_apNames[ i ] == a_strName )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00546c10
TBOOL AGameTimeManager::IsCurPhase( TPString8 a_strName )
{
	return GetDayPhaseFromName( a_strName ) == m_eDayPhase;
}

// $Barnyard: FUNCTION 005461a0
TFLOAT AGameTimeManager::GetDayPhaseStartTime( DAYPHASE a_eDayPhase )
{
	static TFLOAT s_fDayStart       = DAY_START_SECONDS * REAL_TIME_TO_GAME_TIME;                  // 0x00783d54
	static TFLOAT s_fDawnStart      = s_fDayStart;                                                 // 0x00783d50
	static TFLOAT s_fMorningStart   = s_fDawnStart + DAYPHASE_DURATION[ DAYPHASE_DAWN ];           // 0x00783d4c
	static TFLOAT s_fAfternoonStart = s_fMorningStart + DAYPHASE_DURATION[ DAYPHASE_MORNING ];     // 0x00783d48
	static TFLOAT s_fDuskStart      = s_fAfternoonStart + DAYPHASE_DURATION[ DAYPHASE_AFTERNOON ]; // 0x00783d44
	static TFLOAT s_fNightStart     = s_fDuskStart + DAYPHASE_DURATION[ DAYPHASE_DUSK ];           // 0x00783d40

	switch ( a_eDayPhase )
	{
		case DAYPHASE_DAWN: return s_fDawnStart;
		case DAYPHASE_MORNING: return s_fMorningStart;
		case DAYPHASE_AFTERNOON: return s_fAfternoonStart;
		case DAYPHASE_DUSK: return s_fDuskStart;
		case DAYPHASE_NIGHT: return s_fNightStart;
	}

	// NOTE: retail returns whatever is on the FPU stack
	return 0.0f;
}

// $Barnyard: FUNCTION 00546160
AGameTimeManager::WEEKDAY AGameTimeManager::GetWeekDay() const
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

// $Barnyard: FUNCTION 0043ac00
TINT AGameTimeManager::GetCurrentHour() const
{
	TFLOAT fSeconds = GAME_TIME_TO_REAL_TIME * m_flDayTime;

	if ( SECONDS_PER_DAY < fSeconds )
		fSeconds -= SECONDS_PER_DAY;

	return TUINT( fSeconds ) / SECONDS_PER_HOUR;
}

// $Barnyard: FUNCTION 0060e3a0
TINT AGameTimeManager::GetCurrentSecond() const
{
	TFLOAT fSeconds = GAME_TIME_TO_REAL_TIME * m_flDayTime;

	if ( SECONDS_PER_DAY < fSeconds )
		fSeconds -= SECONDS_PER_DAY;

	return TINT( TUINT64( fSeconds ) ) % 60;
}

// $Barnyard: FUNCTION 0043cd30
TINT AGameTimeManager::GetCurrentMinute() const
{
	TFLOAT fSeconds = GAME_TIME_TO_REAL_TIME * m_flDayTime;

	if ( SECONDS_PER_DAY < fSeconds )
		fSeconds -= SECONDS_PER_DAY;

	return ( TUINT( fSeconds ) % SECONDS_PER_HOUR ) / 60;
}
