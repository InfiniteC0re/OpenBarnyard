#include "pch.h"
#include "APlayerProfileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006200b0, CreateObject 00620270, CreateObjectInPlace 006202d0
TDEFINE_CLASS( APlayerProfileManager );

// $Barnyard: FUNCTION 006201c0
APlayerProfileManager::APlayerProfileManager()
{
	m_oDateChangeListener.Connect(
	    *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(),
	    this,
	    OnDateChanged,
	    0
	);
}

// $Barnyard: FUNCTION 00620250
// $Barnyard: FUNCTION 00620120
APlayerProfileManager::~APlayerProfileManager()
{
	m_oDateChangeListener.Disconnect();
}

// $Barnyard: FUNCTION 006200c0
TBOOL APlayerProfileManager::UpdateMainProfile( TFLOAT a_fDeltaTime )
{
	return m_oMainPlayerProfile.Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006200d0
TBOOL APlayerProfileManager::SaveMainProfile( AHackedQuickSave::Buffer* a_pBuffer )
{
	m_oMainPlayerProfile.Save( a_pBuffer );
	return TTRUE;
}

// $Barnyard: FUNCTION 006200f0
TBOOL APlayerProfileManager::LoadMainProfile( AHackedQuickSave::Buffer* a_pBuffer )
{
	m_oMainPlayerProfile.Load( a_pBuffer );
	return TTRUE;
}

// $Barnyard: FUNCTION 006201a0
TBOOL APlayerProfileManager::OnDateChanged( APlayerProfileManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	a_pCaller->m_oMainPlayerProfile.OnDateChanged( a_pEvent );
	return TTRUE;
}
