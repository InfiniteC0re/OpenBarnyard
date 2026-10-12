#include "pch.h"
#include "APlayerLauncherData.h"
#include "AProjectileLauncher.h"
#include "AMilkSquirtProjectileLauncher.h"

#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00669d00, CreateObject 00669d20, CreateObjectInPlace 00669da0
TDEFINE_CLASS( APlayerLauncherData );

// NOTE: inlined into CreateObject 00669d20 / CreateObjectInPlace 00669da0
APlayerLauncherData::APlayerLauncherData()
    : m_eFlags( FLAGS_NONE )
    , m_iPrimaryLauncher( LAUNCHER_NONE )
    , m_iSecondaryLauncher( LAUNCHER_NONE )
{
}

// $Barnyard: FUNCTION 00669e10
// $Barnyard: FUNCTION 00669d10
APlayerLauncherData::~APlayerLauncherData()
{
}

// NOTE: inlined into Create three times in retail
TINT APlayerLauncherData::GetLauncherIndex( const TCHAR* a_szClassName )
{
	const TClass* pClass = TClass::Find( a_szClassName, &TGetClass( AProjectileLauncher ) );

	if ( pClass == &TGetClass( AMilkSquirtProjectileLauncher ) )
		return LAUNCHER_MILK_SQUIRT;

	// NOTE: retail compares with the AAimedSquirtProjectileLauncher TClass (0x007cd0c0), which isn't reconstructed yet
	if ( pClass && TStringManager::String8Compare( pClass->GetName(), "AAimedSquirtProjectileLauncher" ) == 0 )
		return LAUNCHER_AIMED_SQUIRT;

	return LAUNCHER_NONE;
}

// $Barnyard: FUNCTION 00669e30
TBOOL APlayerLauncherData::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail doesn't check the type of EnabledLaunchers
	const PBPropertyValue* pEnabledLaunchers = a_pProperties->GetProperty( "EnabledLaunchers" );

	if ( pEnabledLaunchers )
	{
		const PBPropertyValueArray* pArray = pEnabledLaunchers->GetArray();
		TUINT32                     uiSize = pArray->GetSize();

		for ( TUINT32 i = 0; i < uiSize; i++ )
		{
			TINT iLauncher = GetLauncherIndex( pArray->GetValue( i )->GetTPString8().GetString() );

			if ( iLauncher > LAUNCHER_NONE )
				m_eFlags |= 1 << iLauncher;
		}
	}

	const PBPropertyValue* pPrimaryLauncher = a_pProperties->GetProperty( "PrimaryLauncher" );

	if ( pPrimaryLauncher && pPrimaryLauncher->GetType() == PBPropertyValue::Type::String )
		m_iPrimaryLauncher = GetLauncherIndex( pPrimaryLauncher->GetString() );

	if ( m_iPrimaryLauncher > LAUNCHER_NONE && !IsLauncherEnabled( m_iPrimaryLauncher ) )
		m_iPrimaryLauncher = LAUNCHER_NONE;

	const PBPropertyValue* pSecondaryLauncher = a_pProperties->GetProperty( "SecondaryLauncher" );

	if ( pSecondaryLauncher && pSecondaryLauncher->GetType() == PBPropertyValue::Type::String )
		m_iSecondaryLauncher = GetLauncherIndex( pSecondaryLauncher->GetString() );

	if ( m_iSecondaryLauncher > LAUNCHER_NONE && !IsLauncherEnabled( m_iSecondaryLauncher ) )
		m_iSecondaryLauncher = LAUNCHER_NONE;

	return AUnitStaticData::Create( a_pProperties );
}
