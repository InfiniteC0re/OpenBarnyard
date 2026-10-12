#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>

#include "GameInterface/AHackedQuickSave.h"
#include "FSM/AFunctionTable.h"

class AGameTimeFXManager;

class AGameTimeManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AGameTimeManager>
{
public:
	TDECLARE_CLASS( AGameTimeManager, Toshi::TTask );

public:
	static constexpr TFLOAT GAME_TIME_TO_REAL_TIME = 72.0f;
	static constexpr TFLOAT REAL_TIME_TO_GAME_TIME = 1.0f / GAME_TIME_TO_REAL_TIME;

	static constexpr TUINT SECONDS_PER_HOUR = 60 * 60;
	static constexpr TUINT SECONDS_PER_DAY  = 24 * SECONDS_PER_HOUR;

	static constexpr TFLOAT DAY_START_SECONDS = 6 * SECONDS_PER_HOUR; // 6 AM
	static constexpr TFLOAT DAY_LENGTH_UNITS  = SECONDS_PER_DAY * REAL_TIME_TO_GAME_TIME;

	using DAYPHASE = TINT;
	enum DAYPHASE_ : DAYPHASE
	{
		DAYPHASE_DAWN      = 0,
		DAYPHASE_MORNING   = 1,
		DAYPHASE_AFTERNOON = 2,
		DAYPHASE_DUSK      = 3,
		DAYPHASE_NIGHT     = 4,
		DAYPHASE_NUMOF     = 5,
	};

	// The day of the week (see the names in AGUI2DayOfWeek::Show)
	using WEEKDAY = TINT;
	enum WEEKDAY_ : WEEKDAY
	{
		WEEKDAY_MONDAY    = 0,
		WEEKDAY_TUESDAY   = 1,
		WEEKDAY_WEDNESDAY = 2,
		WEEKDAY_THURSDAY  = 3,
		WEEKDAY_FRIDAY    = 4,
		WEEKDAY_SATURDAY  = 5,
		WEEKDAY_SUNDAY    = 6,
		WEEKDAY_NUMOF     = 7,
	};

	static constexpr TFLOAT DAYPHASE_DURATION[ AGameTimeManager::DAYPHASE_NUMOF ] = {
		40.0f,  // DAYPHASE_DAWN
		350.0f, // DAYPHASE_MORNING
		350.0f, // DAYPHASE_AFTERNOON
		40.0f,  // DAYPHASE_DUSK
		420.0f  // DAYPHASE_NIGHT
	};

	using DATECHANGETYPE = TINT;
	enum DATECHANGETYPE_ : DATECHANGETYPE
	{
		DATECHANGETYPE_DAYPHASE = 1,
		DATECHANGETYPE_DAY      = 2,
		DATECHANGETYPE_MONTH    = 3,
		DATECHANGETYPE_YEAR     = 4,
	};

	class DateChangeEvent
	{
	public:
		DateChangeEvent( DATECHANGETYPE a_eType, DAYPHASE a_eCurrDayPhase, DAYPHASE a_ePrevDayPhase, TINT a_iDay, TINT a_iMonth, TINT a_iYear );
		virtual ~DateChangeEvent();

	public:
		DATECHANGETYPE m_eUpdateType;
		DAYPHASE       m_eCurrDayPhase;
		DAYPHASE       m_ePrevDayPhase;
		TINT           m_iDay;
		TINT           m_iMonth;
		TINT           m_iYear;
	};

	using DateChangeEmitter = Toshi::TEmitter<AGameTimeManager, DateChangeEvent>;

public:
	AGameTimeManager();
	~AGameTimeManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x38

	TBOOL OnSerialize( AHackedQuickSave::Buffer* a_pSerializer );
	TBOOL OnDeserialize( AHackedQuickSave::Buffer* a_pSerializer );

	void PushUpdateState();
	void PopUpdateState();
	void ToggleUpdateState();

	void ChangePhaseOfDay( DAYPHASE a_eDayPhase, TBOOL a_bUpdateTime, TBOOL a_bFireEvent );
	void SetTime( TUINT8 a_ui8Hour, TUINT8 a_ui8Minute, TUINT8 a_ui8ExtraMinutes, TBOOL a_bKeepDayPhase, TBOOL a_bFireEvent );
	void UpdateDate();

#ifdef BARNYARD_LATEST
	// Wii: ends the current day, the date moves on to the next one
	void SkipToNextDay();
#endif

	TBOOL IsDaytime() const;
	TBOOL IsItNight() const;

	WEEKDAY GetWeekDay() const;
	TFLOAT  GetCurrentPhaseTimeLeft() const;
	TINT    GetCurrentHour() const;
	TINT    GetCurrentMinute() const;
	TINT    GetCurrentSecond() const;

	DAYPHASE GetCurrentDayPhase() const { return m_eDayPhase; }
	TINT     GetYear() const { return m_iDateYear; }
	TINT     GetMonth() const { return m_iDateMonth; }
	TINT     GetDay() const { return m_iDateDay; }
	TFLOAT   GetDayTime() const { return m_flDayTime; }
	TFLOAT   GetTotalTime() const { return m_flTotalTime; }
	TBOOL    IsUpdateStopped() const { return m_bStopUpdate; }

	// While TFALSE the day phase doesn't change by itself (ASleepCycleState changes it)
	void SetUpdatePhaseOfDay( TBOOL a_bUpdate ) { m_bUpdatePhaseOfDay = a_bUpdate; }

	// Game time (0-1200) the day phase a_eDayPhase starts at
	static TFLOAT GetDayPhaseStartTime( DAYPHASE a_eDayPhase );

	// Returns the day phase named a_strName ("dawn", "morning", ...), -1 if there's none
	DAYPHASE GetDayPhaseFromName( Toshi::TPString8 a_strName ) const;

	// FSM "IsCurPhase": TRUE if the current day phase is named a_strName
	TBOOL IsCurPhase( Toshi::TPString8 a_strName );

public:
	static AFunctionTable ms_oFunctionTable;

	DateChangeEmitter* GetDateChangeEmitter() const { return m_pDateChangeEmitter; }

private:
	TFLOAT             m_flTotalTime;        // 0x20
	TINT               m_iDateYear;          // 0x24
	TINT               m_iDateMonth;         // 0x28
	TINT               m_iDateDay;           // 0x2c
	WEEKDAY            m_iDateWeekDay;       // 0x30
	DAYPHASE           m_eDayPhase;          // 0x34
	TFLOAT             m_flDayTime;          // 0x38
	DateChangeEmitter* m_pDateChangeEmitter; // 0x3c
	TFLOAT             m_flSecondTimer;      // 0x40
	TBOOL              m_bStopUpdate;        // 0x44
	TFLOAT             m_flCurrentPhaseTime; // 0x48
	TBOOL              m_bSavedState;        // 0x4c
	TBOOL              m_bUpdatePhaseOfDay;  // 0x4d
};

TSINGLETON_DECLARE_ALIAS( AGameTimeManager, GameTimeMngr );
