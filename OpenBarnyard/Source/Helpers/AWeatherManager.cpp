#include "pch.h"
#include "AWeatherManager.h"

#include <Math/TRandom.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054c670
AWeatherManager::AWeatherManager()
    : m_eCurrentWeather( WEATHERTYPE_NORMAL )
    , m_eNextWeather( WEATHERTYPE_NORMAL )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original never initialises the lock
	m_bLocked = TFALSE;
#endif

	m_oDateChangeListener.Connect( *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(), this, OnDateChangeCallback, -100 );
}

// $Barnyard: FUNCTION 0054c5c0
AWeatherManager::~AWeatherManager()
{
	m_oDateChangeListener.Disconnect();
}

// $Barnyard: FUNCTION 0054c500
void AWeatherManager::PickNextWeather()
{
	if ( !m_bLocked )
	{
		const WEATHERTYPE aWeatherCycle[] = {
			WEATHERTYPE_NORMAL,
			WEATHERTYPE_NORMAL,
			WEATHERTYPE_LIGHTRAIN,
			WEATHERTYPE_NORMAL,
			WEATHERTYPE_NORMAL,
			WEATHERTYPE_HEAVYRAIN
		};

		m_eNextWeather = aWeatherCycle[ ms_uiCycleIndex++ ];

		if ( ms_uiCycleIndex >= TARRAYSIZE( aWeatherCycle ) )
			ms_uiCycleIndex = 0;
	}
}

// $Barnyard: FUNCTION 0054c550
void AWeatherManager::SetCurrentWeather( WEATHERTYPE a_eWeather )
{
	m_eCurrentWeather = a_eWeather;
	m_eNextWeather    = a_eWeather;
	ms_iDayPhasesLeft = 7;
}

// $Barnyard: FUNCTION 0054c570
AWeatherManager::WEATHERTYPE AWeatherManager::GetWeatherFromName( const TCHAR* a_szName )
{
	if ( !a_szName )
		return -1;

	for ( TINT i = 0; i < WEATHERTYPE_NUMOF; i++ )
	{
		if ( TStringManager::String8CompareNoCase( ms_aWeatherNames[ i ], a_szName ) == 0 )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0054c610
TBOOL AWeatherManager::OnDateChangeCallback( AWeatherManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	a_pCaller->m_eCurrentWeather = a_pCaller->m_eNextWeather;

	if ( a_pEvent->m_eUpdateType == AGameTimeManager::DATECHANGETYPE_DAYPHASE && --ms_iDayPhasesLeft <= 0 )
	{
		TRandom oRandom;
		ms_iDayPhasesLeft = oRandom.GetInt( 3 ) + 2;

		a_pCaller->PickNextWeather();
	}

	return TTRUE;
}
