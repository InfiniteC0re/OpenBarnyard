#include "pch.h"
#include "AInputMap.h"
#include "AInputMapManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS_NORUNTIME( AInputMap );

// $Barnyard: FUNCTION 00596c60
void AInputMap::LoadFromProperties( const PBProperties& a_rProperties )
{
	m_oDoodadToCommandMap.Clear();

	auto pInputManager = AInputMapManager::GetSingleton();

	for ( auto it = a_rProperties.Begin(); it != a_rProperties.End(); it++ )
	{
		auto eCommandCode = pInputManager->GetCommandCode( Toshi::TPString8( it->GetName().GetString() ) );
		auto pArray       = it->GetValue()->GetArray();

		for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		{
			auto pValue  = pArray->GetValue( i );
			auto iDoodad = pInputManager->GetDoodadFromKey( pValue->GetTPString8() );

			// The index of the binding is used to find the button that is listed first
			CommandBinding uiBinding = TUINT16( eCommandCode ) | ( TUINT32( TUINT16( i + 1 ) ) << 16 );
			m_oDoodadToCommandMap.Insert( iDoodad, uiBinding );
		}
	}
}

// $Barnyard: FUNCTION 005968f0
Toshi::TInputDevice::Doodad AInputMap::GetCommandDoodad( AInputCommand a_eCommand, TBOOL a_bPreferMouseKeyboard )
{
	Toshi::TInputDevice::Doodad iDoodad       = -1;
	TUINT16                     uiLowestIndex = TUINT16( -1 );

	auto pInputManager = AInputMapManager::GetSingleton();

	for ( auto it = m_oDoodadToCommandMap.Begin(); it != m_oDoodadToCommandMap.End(); it++ )
	{
		CommandBinding uiBinding = it->GetSecond();

		if ( a_eCommand != GetBindingCommand( uiBinding ) )
			continue;

		Toshi::TInputDevice::Doodad iBindingDoodad = it->GetFirst();
		
#ifdef BARNYARD_COMMUNITY_PATCH
		if ( !pInputManager->HasBindForDoodad( iBindingDoodad ) )
			continue;
#endif // BARNYARD_COMMUNITY_PATCH

		if ( a_bPreferMouseKeyboard == TTRUE )
		{
			// Mouse buttons are returned straight away, keyboard keys are preferred to anything else
			if ( iBindingDoodad > 0x30000 && iBindingDoodad < 0x3000d )
				return iBindingDoodad;

			if ( iBindingDoodad > 0x20000 && iBindingDoodad < 0x20080 )
				iDoodad = iBindingDoodad;
			else if ( iDoodad == -1 )
				iDoodad = iBindingDoodad;
		}
		else if ( GetBindingIndex( uiBinding ) < uiLowestIndex )
		{
			uiLowestIndex = GetBindingIndex( uiBinding );
			iDoodad       = iBindingDoodad;
		}
	}

	return iDoodad;
}
