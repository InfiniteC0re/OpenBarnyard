#pragma once
#include "APlayerSimProfile.h"
#include "Helpers/AGameTimeManager.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>

// Owns the main player's profile (gender, costume, stamina...) and lowers its max stamina every new day.
class MODAPI APlayerProfileManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerProfileManager>
{
public:
	TDECLARE_CLASS( APlayerProfileManager, Toshi::TObject );

public:
	APlayerProfileManager();
	~APlayerProfileManager();

	// Updates the main profile.
	TBOOL UpdateMainProfile( TFLOAT a_fDeltaTime );

	// Saves / loads the main profile.
	TBOOL SaveMainProfile( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL LoadMainProfile( AHackedQuickSave::Buffer* a_pBuffer );

	APlayerSimProfile*     GetMainProfile() { return &m_oMainPlayerProfile; }
	APlayerProfile::GENDER GetMainGender() const { return m_oMainPlayerProfile.GetGender(); }
	void                   SetMainGender( APlayerProfile::GENDER a_eGender ) { m_oMainPlayerProfile.SetGender( a_eGender ); }
	TINT                   GetMainCostume() const { return m_oMainPlayerProfile.GetCostume(); }

private:
	static TBOOL __stdcall OnDateChanged( APlayerProfileManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );

private:
	APlayerSimProfile                                                                            m_oMainPlayerProfile;  // 0x04
	Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, APlayerProfileManager> m_oDateChangeListener; // 0x1d4
};

TSTATICASSERT( sizeof( APlayerProfileManager ) == 0x1e8 );
