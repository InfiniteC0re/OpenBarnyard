#pragma once
#include "Helpers/AGameTimeManager.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>

// Decides the weather of the sim world. The next weather type is picked every
// few day phases from a fixed cycle and becomes the current one on the next
// day phase change, so the time-of-day FX can blend towards it beforehand.
//
// NOTE: named after AWeatherManager of JPOG (older TOSHI), this one has no TClass. It's
// the 0x20 bytes object at 0x00783ecc created by AGameSystemManager::OnCreate.
class AWeatherManager
    : public Toshi::TSingleton<AWeatherManager>
{
public:
	using WEATHERTYPE = TINT;
	enum WEATHERTYPE_ : WEATHERTYPE
	{
		WEATHERTYPE_NORMAL,
		WEATHERTYPE_HEATWAVE,
		WEATHERTYPE_LIGHTRAIN,
		WEATHERTYPE_HEAVYRAIN,
		WEATHERTYPE_RAINBOW,
		WEATHERTYPE_LIGHTNINGSTORM,
		WEATHERTYPE_NUMOF
	};

public:
	AWeatherManager();
	~AWeatherManager();

	// Picks the next weather type from the cycle unless the weather is locked
	void PickNextWeather();

	// Sets both the current and the next weather type
	void SetCurrentWeather( WEATHERTYPE a_eWeather );

	// Returns the weather type with the name a_szName or -1 if there's no such type
	static WEATHERTYPE GetWeatherFromName( const TCHAR* a_szName );

	// $Barnyard: FUNCTION 0043cda0
	// Whether the current weather is one of the rains (light, heavy or the storm)
	TBOOL IsBadWeather() const { return m_eCurrentWeather == WEATHERTYPE_LIGHTRAIN || m_eCurrentWeather == WEATHERTYPE_HEAVYRAIN || m_eCurrentWeather == WEATHERTYPE_LIGHTNINGSTORM; }

	// $Barnyard: FUNCTION 0043cdc0
	// Whether the current weather is the heavy rain or the storm
	TBOOL IsHeavyRain() const { return m_eCurrentWeather == WEATHERTYPE_HEAVYRAIN || m_eCurrentWeather == WEATHERTYPE_LIGHTNINGSTORM; }

	WEATHERTYPE GetCurrentWeather() const { return m_eCurrentWeather; }
	WEATHERTYPE GetNextWeather() const { return m_eNextWeather; }

	void  LockToCurrentWeatherType( TBOOL a_bLock ) { m_bLocked = a_bLock; }
	TBOOL IsLocked() const { return m_bLocked; }

private:
	static TBOOL __stdcall OnDateChangeCallback( AWeatherManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );

private:
	// Names of the weather types (0x00774200)
	inline static const TCHAR* ms_aWeatherNames[ WEATHERTYPE_NUMOF ] = {
		"normal",
		"heatwave",
		"lightrain",
		"heavyrain",
		"rainbow",
		"lightningstorm"
	};

	// Number of day phases left until the next weather is picked (0x00774218)
	inline static TINT ms_iDayPhasesLeft = 2;

	// Position in the weather cycle (0x00783ed0)
	inline static TUINT ms_uiCycleIndex = 0;

private:
	TBOOL                                                                                  m_bLocked;             // 0x0
	WEATHERTYPE                                                                            m_eCurrentWeather;     // 0x4
	WEATHERTYPE                                                                            m_eNextWeather;        // 0x8
	Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, AWeatherManager> m_oDateChangeListener; // 0xc
};
