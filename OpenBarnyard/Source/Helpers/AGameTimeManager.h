#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>

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
	TBOOL OnSerialize( void* a_pSerializer );
	TBOOL OnDeserialize( void* a_pSerializer );

	void PushUpdateState();
	void PopUpdateState();
	void ToggleUpdateState();

	void SetPhaseOfDay( DAYPHASE a_eDayPhase, TBOOL a_bUpdateTime, TBOOL a_bFireEvent );
	void SetTime( TUINT8 a_ui8Hour, TUINT8 a_ui8Minute, TUINT8 a_ui8ExtraMinutes, TBOOL a_bKeepDayPhase, TBOOL a_bFireEvent );
	void UpdateDate();

	TBOOL IsDaytime() const;
	TBOOL IsItNight() const;

	TINT   GetWeekDay() const;
	TFLOAT GetCurrentPhaseTimeLeft() const;
	TINT   GetCurrentMinute() const;

	DAYPHASE GetCurrentDayPhase() const { return m_eDayPhase; }

private:
	TFLOAT             m_flTotalTime;
	TINT               m_iDateYear;
	TINT               m_iDateMonth;
	TINT               m_iDateDay;
	TINT               m_iDateWeekDay;
	DAYPHASE           m_eDayPhase;
	TFLOAT             m_flDayTime;
	TFLOAT             m_flSecondTimer;
	TBOOL              m_bStopUpdate;
	TFLOAT             m_flCurrentPhaseTime;
	TBOOL              m_bSavedState;
	TBOOL              m_bUpdatePhaseOfDay;
	DateChangeEmitter* m_pDateChangeEmitter;
};

TSINGLETON_DECLARE_ALIAS( AGameTimeManager, GameTimeMngr );
