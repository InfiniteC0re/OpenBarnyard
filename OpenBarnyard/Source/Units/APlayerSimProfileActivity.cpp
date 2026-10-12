#include "pch.h"
#include "APlayerSimProfileActivity.h"
#include "AUnitPlayer.h"
#include "Projectiles/AProjectileLauncher.h"
#include "Projectiles/ALaunchAmmo.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ABarnRewardState.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bafe0, CreateObject 005bb350, CreateObjectInPlace 005bb3d0
TDEFINE_CLASS( APlayerSimProfileActivity );

// The adders of the values in s_apFloatValues and s_apIntValues (0x007b49a8 and 0x007b49a0)
using t_FloatAdder = void ( APlayerSimProfile::* )( TFLOAT );
using t_IntAdder   = void ( APlayerSimProfile::* )( TINT );

static t_FloatAdder s_afnFloatAdders[] = { &APlayerSimProfile::AddStamina, &APlayerSimProfile::AddMaxStamina };
static t_IntAdder   s_afnIntAdders[]   = { &APlayerSimProfile::AddPickups };

// $Barnyard: FUNCTION 00620e70
static void AddProfileFloatValue( APlayerSimProfile* a_pProfile, TINT a_iIndex, TFLOAT a_fValue )
{
	( a_pProfile->*s_afnFloatAdders[ a_iIndex ] )( a_fValue );
}

// $Barnyard: FUNCTION 00620e90
static void AddProfileIntValue( APlayerSimProfile* a_pProfile, TINT a_iIndex, TINT a_iValue )
{
	( a_pProfile->*s_afnIntAdders[ a_iIndex ] )( a_iValue );
}

TPSTRING8_DECLARE( add );
TPSTRING8_DECLARE( mode );
TPSTRING8_DECLARE( type );
TPSTRING8_DECLARE( value );
TPSTRING8_DECLARE( energy );
TPSTRING8_DECLARE( energylimit );
TPSTRING8_DECLARE( money );
TPSTRING8_DECLARE( powerup_full_milk );

// Names of the modes (0x0077532c), the index is the MODE
static Toshi::TPString8* s_apModes[] = { &g_str_add };

// Values of APlayerSimProfile set with a float (0x00778d04), the index is passed to the setter table 0x007b49a8
static Toshi::TPString8* s_apFloatValues[] = { &g_str_energy, &g_str_energylimit };

// Values of APlayerSimProfile set with an int (0x00778d0c), the index is passed to the setter table 0x007b49a0
static Toshi::TPString8* s_apIntValues[] = { &g_str_money };

// NOTE: inlined into CreateObject 005bb350 / CreateObjectInPlace 005bb3d0
APlayerSimProfileActivity::APlayerSimProfileActivity()
    : m_pRewardState( TNULL )
{
}

// $Barnyard: FUNCTION 005bb1b0
// $Barnyard: FUNCTION 005bb440
APlayerSimProfileActivity::~APlayerSimProfileActivity()
{
}

// $Barnyard: FUNCTION 005bb220
TBOOL APlayerSimProfileActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strMode;
	a_pProperties->GetOptionalPropertyValue( strMode, TPS8( mode ) );

	TINT iMode = 0;
	for ( ; iMode < TARRAYSIZE( s_apModes ); iMode++ )
	{
		if ( strMode == *s_apModes[ iMode ] )
			break;
	}

	m_eMode = iMode;
	a_pProperties->GetOptionalPropertyValue( m_strType, TPS8( type ) );

	for ( TINT i = 0; i < TARRAYSIZE( s_apFloatValues ); i++ )
	{
		if ( m_strType == *s_apFloatValues[ i ] )
		{
			a_pProperties->GetOptionalFloatOrIntValue( m_fValue, TPS8( value ) );
			break;
		}
	}

	for ( TINT i = 0; i < TARRAYSIZE( s_apIntValues ); i++ )
	{
		if ( m_strType == *s_apIntValues[ i ] )
		{
			a_pProperties->GetOptionalPropertyValue( m_iValue, TPS8( value ) );
			break;
		}
	}

	if ( m_strType == TPS8( powerup_full_milk ) )
		a_pProperties->GetOptionalFloatOrIntValue( m_fValue, TPS8( value ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005baff0
AInteractionActivity::RESULT APlayerSimProfileActivity::Update( AInteractionSequence* a_pSequence )
{
	// Waits for the reward screen to be over
	if ( m_pRewardState )
	{
		if ( AGameStateController::GetSingleton()->GetCurrentState() != m_pRewardState )
		{
			m_pRewardState = TNULL;
			return RESULT_DONE;
		}

		return RESULT_RUNNING;
	}

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	for ( TINT i = 0; i < TARRAYSIZE( s_apFloatValues ); i++ )
	{
		if ( m_strType == *s_apFloatValues[ i ] )
		{
			if ( m_eMode == MODE_ADD )
			{
				AddProfileFloatValue( pProfile, i, m_fValue );
			}

			break;
		}
	}

	for ( TINT i = 0; i < TARRAYSIZE( s_apIntValues ); i++ )
	{
		if ( m_strType == *s_apIntValues[ i ] )
		{
			if ( m_eMode == MODE_ADD )
			{
				AddProfileIntValue( pProfile, i, m_iValue );
			}

			break;
		}
	}

	if ( m_eMode == MODE_ADD )
	{
		if ( m_strType == TPS8( powerup_full_milk ) )
		{
			// NOTE: APlayerManager::GetPlayer (0061faf0) inlined
			AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerUnit( 0 ) );
			ALaunchAmmo* pAmmo   = pPlayer->GetActiveLauncher()->GetAmmo();
			pAmmo->SetAmmo( pAmmo->GetMaxAmmo() );

			// NOTE: ASoundManager::PlayCueAtUnit (plays the cue at the unit) inlined
			ASoundManager::GetSingleton()->PlayCueAtPosition( 0xb3, *pPlayer->GetSteer()->GetPosition() );
		}
		else if ( m_strType == TPS8( money ) && m_iValue > 0 )
		{
			// Show the money on the barn reward screen
			m_pRewardState = new ABarnRewardState( ABarnRewardState::TYPE_MONEY, m_iValue );
			AGameStateController::GetSingleton()->PushState( m_pRewardState );
			return RESULT_RUNNING;
		}
	}

	return RESULT_DONE;
}
