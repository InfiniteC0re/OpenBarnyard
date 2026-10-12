#include "pch.h"
#include "ALauncherRequirement.h"
#include "AUnitPlayer.h"
#include "Player/APlayerManager.h"
#include "Projectiles/AMilkSquirtProjectileLauncher.h"
#include "Projectiles/AAimedSquirtProjectileLauncher.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005acd70, CreateObject 005acf30
// NOTE: GetClass 005acd70, CreateObject 005acf30, CreateObjectInPlace 005acfa0
TDEFINE_CLASS( ALauncherRequirement );

TPSTRING8_DECLARE( normal );
TPSTRING8_DECLARE( aimed );

// $Barnyard: FUNCTION 005acec0
// $Barnyard: FUNCTION 005ad010
ALauncherRequirement::~ALauncherRequirement()
{
}

// $Barnyard: FUNCTION 005ace10
TBOOL ALauncherRequirement::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail reads the value into a temporary, m_strLauncher stays empty
	TPString8 strLauncher;
	a_pProperties->GetOptionalPropertyValue( strLauncher, TPS8D( "launcher" ) );

	AInteractionRequirement::Create( a_pProperties );
	return TTRUE;
}

// $Barnyard: FUNCTION 005acd90
void ALauncherRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AUnitPlayer*         pPlayer   = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetPlayer( 0 ) );
	AProjectileLauncher* pLauncher = pPlayer->GetActiveLauncher();

	TBOOL bMatch;

	if ( m_strLauncher == TPS8( normal ) )
		bMatch = pLauncher->GetClass() == &TGetClass( AMilkSquirtProjectileLauncher );
	else if ( m_strLauncher == TPS8( aimed ) )
		bMatch = pLauncher->GetClass() == &TGetClass( AAimedSquirtProjectileLauncher );
	else
		return;

	// NOTE: only the weight is set
	if ( bMatch )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}
