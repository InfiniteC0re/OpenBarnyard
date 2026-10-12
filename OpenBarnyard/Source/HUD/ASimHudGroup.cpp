#include "pch.h"
#include "Helpers/ABikeManager.h"
#include "ASimHudGroup.h"
#include "AHudManager.h"
#include "AGameTimeHudElement.h"
#include "ASimPlayerMainHudElement.h"
#include "ASimPlayerPickUpHudElement.h"
#include "ATimeLeftHudElement.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Units/AUnitPlayer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0045ad40
ASimHudGroup::ASimHudGroup()
    : m_pGameTime( TNULL )
    , m_pMain( TNULL )
    , m_pPickUp( TNULL )
    , m_pTimeLeft( TNULL )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 0045adc0
void ASimHudGroup::Create()
{
	m_pGameTime = new AGameTimeHudElement();
	m_pMain     = new ASimPlayerMainHudElement();
	m_pPickUp   = new ASimPlayerPickUpHudElement();
	m_pTimeLeft = new ATimeLeftHudElement();

	AHudManager* pHudManager = AHudManager::GetSingleton();
	pHudManager->AddElement( m_pMain, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( m_pGameTime, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( m_pPickUp, TFALSE, TTRUE, TFALSE );
	pHudManager->AddElement( m_pTimeLeft, TFALSE, TTRUE, TFALSE );

	// Make the timer as big as the clock it replaces
	TINT iTimerWidth, iTimerHeight;
	m_pTimeLeft->GetDimensions( iTimerWidth, iTimerHeight );

	TINT iClockWidth, iClockHeight;
	m_pGameTime->GetDimensions( iClockWidth, iClockHeight );

	TFLOAT fScaleY = TFLOAT( iClockHeight ) / TFLOAT( iTimerHeight );
	TFLOAT fScaleX = TFLOAT( iClockWidth ) / TFLOAT( iTimerWidth );
	m_pTimeLeft->SetScale( fScaleX, fScaleY );

	m_pTimeLeft->SetForceHide( TTRUE );
}

// $Barnyard: FUNCTION 0045af50
void ASimHudGroup::Destroy()
{
	m_pGameTime->Detach();
	m_pMain->Detach();
	m_pPickUp->Detach();
	m_pTimeLeft->Detach();

	if ( m_pGameTime )
		delete m_pGameTime;

	m_pGameTime = TNULL;

	if ( m_pMain )
		delete m_pMain;

	m_pMain = TNULL;

	if ( m_pPickUp )
		delete m_pPickUp;

	m_pPickUp = TNULL;

	if ( m_pTimeLeft )
		delete m_pTimeLeft;

	m_pTimeLeft = TNULL;
}

// $Barnyard: FUNCTION 0045afd0
void ASimHudGroup::Update( TFLOAT a_fDeltaTime )
{
	// NOTE: the original adds 4 to the singleton's address and compares the result to TNULL
	if ( !APlayerProfileManager::GetSingleton() )
		return;

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	// While riding the bike shows its own stamina
	if ( !ABikeManager::GetSingleton()->IsPlayerOnBike() )
	{
		m_pMain->SetStamina( pProfile->GetStamina() / 5.0f );
	}

	AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayer( 0 ) );

	if ( pPlayer && pPlayer->GetActiveLauncher() )
		m_pMain->UpdateMilkAmmo( pPlayer->GetActiveLauncher() );
}

// $Barnyard: FUNCTION 0045ad60
void ASimHudGroup::SetVisible( TBOOL a_bVisible )
{
	m_pMain->SetForceHide( !a_bVisible );
	m_pPickUp->SetForceHide( !a_bVisible );

	if ( m_eFlags & FLAGS_SHOW_TIMER )
		m_pTimeLeft->SetForceHide( !a_bVisible );
	else
		m_pGameTime->SetForceHide( !a_bVisible );
}

// $Barnyard: FUNCTION 0045ad90
void ASimHudGroup::SetShowTimer( TBOOL a_bShowTimer )
{
	if ( a_bShowTimer )
		m_eFlags |= FLAGS_SHOW_TIMER;
	else
		m_eFlags &= ~FLAGS_SHOW_TIMER;

	m_pTimeLeft->SetForceHide( !a_bShowTimer );
	m_pGameTime->SetForceHide( a_bShowTimer );
}
