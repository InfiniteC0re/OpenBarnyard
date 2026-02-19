#include "pch.h"
#include "ACoreSettings.h"

#include <Plugins/PTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ACoreSettings::ACoreSettings()
{
}

ACoreSettings::~ACoreSettings()
{
}


void ACoreSettings::Save()
{
	PTRB                        outFile;
	PTRBSections*               pSections  = outFile.GetSections();
	PTRBSymbols*                pSymbols   = outFile.GetSymbols();
	PTRBSections::MemoryStream* pMemStream = pSections->CreateStream();

	// Write settings to the stream
	auto pSettings = pMemStream->Alloc<ACoreSettingsProperties>();
	*pSettings     = *this;

	// Save the file
	pSymbols->Add( pMemStream, "CoreSettings", pSettings.get() );
	outFile.WriteToFile( "BYModCore.trb" );

	TINFO( "Saved core settings file\n" );
}

void ACoreSettings::Load()
{
	TTRB inFile;

	if ( inFile.Load( "BYModCore.trb" ) == TTRB::ERROR_OK )
	{
		TINFO( "Reading core settings file...\n" );
		ACoreSettingsProperties* pSettings = inFile.CastSymbol<ACoreSettingsProperties>( "CoreSettings" );

		if ( pSettings->uiMagic != TFourCC( "MDCR" ) )
		{
			TERROR( "The settings file has wrong signature!\n" );
			return;
		}

		switch ( pSettings->uiVersion )
		{
			case 2:
				bBetterGrass = pSettings->bBetterGrass;
			case 1:
				bLimitFPS        = pSettings->bLimitFPS;
				iMaxFPS          = pSettings->iMaxFPS;
				bReduceLoadTimes = pSettings->bReduceLoadTimes;
				break;
			default:
				TINFO( "The settings file's version is not supported!\n" );
				break;
		}
	}

	Apply();
}

void ACoreSettings::Apply()
{
	{
		//-----------------------------------------------------------------------------
		// Load times patch
		//-----------------------------------------------------------------------------
		// Note: Drastically reduces load times since loader thread is much
		// faster nowadays and it doesn't need to sleep
		//-----------------------------------------------------------------------------

		static TUINT32 s_aPatchValues[ 2 ][ 14 ];
		static TBOOL   s_bSetupValues = TTRUE;

		if ( s_bSetupValues )
		{
			s_aPatchValues[ 0 ][ 0 ]  = *(TUINT32*)( 0x004238AE );
			s_aPatchValues[ 0 ][ 1 ]  = *(TUINT32*)( 0x005EBF9E );
			s_aPatchValues[ 0 ][ 2 ]  = *(TUINT32*)( 0x005EC34E );
			s_aPatchValues[ 0 ][ 3 ]  = *(TUINT32*)( 0x005EBF20 );
			s_aPatchValues[ 0 ][ 4 ]  = *(TUINT32*)( 0x004013c8 );
			s_aPatchValues[ 0 ][ 5 ]  = *(TUINT32*)( 0x005d3b13 );
			s_aPatchValues[ 0 ][ 6 ]  = *(TUINT32*)( 0x005d5b78 );
			s_aPatchValues[ 0 ][ 7 ]  = *(TUINT32*)( 0x005d5cb8 );
			s_aPatchValues[ 0 ][ 8 ]  = *(TUINT32*)( 0x005d5dc3 );
			s_aPatchValues[ 0 ][ 9 ]  = *(TUINT32*)( 0x005dac0f );
			s_aPatchValues[ 0 ][ 10 ] = *(TUINT32*)( 0x005dd153 );
			s_aPatchValues[ 0 ][ 11 ] = *(TUINT32*)( 0x005dd288 );
			s_aPatchValues[ 0 ][ 12 ] = *(TUINT32*)( 0x005dd423 );
			s_aPatchValues[ 0 ][ 13 ] = *(TUINT32*)( 0x005dd4f2 );
			s_aPatchValues[ 1 ][ 0 ]  = 0x0020568E;
			s_aPatchValues[ 1 ][ 1 ]  = 0x0003CF9E;
			s_aPatchValues[ 1 ][ 2 ]  = 0x0003CBEE;
			s_aPatchValues[ 1 ][ 3 ]  = 0x0003D01C;
			s_aPatchValues[ 1 ][ 4 ]  = 0x00227B74;
			s_aPatchValues[ 1 ][ 5 ]  = 0x00055429;
			s_aPatchValues[ 1 ][ 6 ]  = 0x000533C4;
			s_aPatchValues[ 1 ][ 7 ]  = 0x00053284;
			s_aPatchValues[ 1 ][ 8 ]  = 0x00053179;
			s_aPatchValues[ 1 ][ 9 ]  = 0x0004E32D;
			s_aPatchValues[ 1 ][ 10 ] = 0x0004BDE9;
			s_aPatchValues[ 1 ][ 11 ] = 0x0004BCB3;
			s_aPatchValues[ 1 ][ 12 ] = 0x0004BB19;
			s_aPatchValues[ 1 ][ 13 ] = 0x0004BA49;
			s_bSetupValues            = TFALSE;

			DWORD dwOldProtection;
			VirtualProtect( (void*)0x004238AE, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EBF9E, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EC34E, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EBF20, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x004013c8, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005d3b13, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005d5b78, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005d5cb8, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005d5dc3, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005dac0f, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005dd153, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005dd288, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005dd423, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005dd4f2, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
		}

		TUINT32* pValues          = s_aPatchValues[ bReduceLoadTimes ? 1 : 0 ];
		*(TUINT32*)( 0x004238AE ) = pValues[ 0 ];
		*(TUINT32*)( 0x005EBF9E ) = pValues[ 1 ];
		*(TUINT32*)( 0x005EC34E ) = pValues[ 2 ];
		*(TUINT32*)( 0x005EBF20 ) = pValues[ 3 ];
		*(TUINT32*)( 0x004013c8 ) = pValues[ 4 ];
		*(TUINT32*)( 0x005d3b13 ) = pValues[ 5 ];
		*(TUINT32*)( 0x005d5b78 ) = pValues[ 6 ];
		*(TUINT32*)( 0x005d5cb8 ) = pValues[ 7 ];
		*(TUINT32*)( 0x005d5dc3 ) = pValues[ 8 ];
		*(TUINT32*)( 0x005dac0f ) = pValues[ 9 ];
		*(TUINT32*)( 0x005dd153 ) = pValues[ 10 ];
		*(TUINT32*)( 0x005dd288 ) = pValues[ 11 ];
		*(TUINT32*)( 0x005dd423 ) = pValues[ 12 ];
		*(TUINT32*)( 0x005dd4f2 ) = pValues[ 13 ];
	}

	{
		//-----------------------------------------------------------------------------
		// Better grass
		//-----------------------------------------------------------------------------
		// Note: Forces the game to always render all layers and enables a hidden layer
		//-----------------------------------------------------------------------------

		static TUINT16 s_aPatchValues[ 2 ][ 5 ];
		static TBOOL   s_bSetupValues = TTRUE;

		if ( s_bSetupValues )
		{
			s_aPatchValues[ 0 ][ 0 ] = *(TUINT16*)( 0x005f8525 );
			s_aPatchValues[ 0 ][ 1 ] = *(TUINT16*)( 0x005f851b );
			s_aPatchValues[ 0 ][ 2 ] = *(TUINT16*)( 0x005f851d );
			s_aPatchValues[ 0 ][ 3 ] = *(TUINT16*)( 0x005f851f );
			s_aPatchValues[ 0 ][ 4 ] = *(TUINT16*)( 0x005f8539 );
			s_aPatchValues[ 1 ][ 0 ] = 0x06B9;
			s_aPatchValues[ 1 ][ 1 ] = 0x9090;
			s_aPatchValues[ 1 ][ 2 ] = 0x9090;
			s_aPatchValues[ 1 ][ 3 ] = 0x9090;
			s_aPatchValues[ 1 ][ 4 ] = 0x52EB;
			s_bSetupValues           = TFALSE;

			DWORD dwOldProtection;
			VirtualProtect( (void*)0x005f8525, 2, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005f851b, 2, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005f851d, 2, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EBF20, 2, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005f8539, 2, PAGE_EXECUTE_READWRITE, &dwOldProtection );
		}

		TUINT16* pValues          = s_aPatchValues[ bBetterGrass ? 1 : 0 ];
		*(TUINT16*)( 0x005f8525 ) = pValues[ 0 ];
		*(TUINT16*)( 0x005f851b ) = pValues[ 1 ];
		*(TUINT16*)( 0x005f851d ) = pValues[ 2 ];
		*(TUINT16*)( 0x005f851f ) = pValues[ 3 ];
		*(TUINT16*)( 0x005f8539 ) = pValues[ 4 ];
	}

	{
		//-----------------------------------------------------------------------------
		// Let you load any level
		//-----------------------------------------------------------------------------
		// Note: Disables a lot of things just to make it possible to load any level
		// instead of EnvMain (still need to spoof the name by any way)
		//-----------------------------------------------------------------------------

		static TUINT32 s_aPatchValues[ 2 ][ 10 ];
		static TBOOL   s_bSetupValues = TTRUE;

		if ( s_bSetupValues )
		{
			s_aPatchValues[ 0 ][ 0 ] = *(TUINT32*)( 0x0043F3BE );
			s_aPatchValues[ 0 ][ 1 ] = *(TUINT32*)( 0x005de670 );
			s_aPatchValues[ 0 ][ 2 ] = *(TUINT32*)( 0x005eafe1 );
			s_aPatchValues[ 0 ][ 3 ] = *(TUINT32*)( 0x005eafe5 );
			s_aPatchValues[ 0 ][ 4 ] = *(TUINT32*)( 0x005E8DE0 );
			s_aPatchValues[ 0 ][ 5 ] = *(TUINT32*)( 0x005E8DE4 );
			s_aPatchValues[ 0 ][ 6 ] = *(TUINT32*)( 0x0054d7c0 );
			s_aPatchValues[ 0 ][ 7 ] = *(TUINT32*)( 0x0054D7E0 );
			s_aPatchValues[ 0 ][ 8 ] = *(TUINT32*)( 0x0043F927 );
			s_aPatchValues[ 0 ][ 9 ] = *(TUINT32*)( 0x0043E98E );
			s_aPatchValues[ 1 ][ 0 ] = 0x00028AE9;
			s_aPatchValues[ 1 ][ 1 ] = 0x900008C2;
			s_aPatchValues[ 1 ][ 2 ] = 0x90909090;
			s_aPatchValues[ 1 ][ 3 ] = 0x868B9090;
			s_aPatchValues[ 1 ][ 4 ] = 0x000000B8;
			s_aPatchValues[ 1 ][ 5 ] = 0x5390C300;
			s_aPatchValues[ 1 ][ 6 ] = 0x508B90C3;
			s_aPatchValues[ 1 ][ 7 ] = 0x8BF18BC3;
			s_aPatchValues[ 1 ][ 8 ] = 0x0000A4E9;
			s_aPatchValues[ 1 ][ 9 ] = 0x0000E0E9;
			s_bSetupValues           = TFALSE;

			DWORD dwOldProtection;
			VirtualProtect( (void*)0x0043F3BE, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005de670, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005eafe1, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005eafe5, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005E8DE0, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005E8DE4, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x0054d7c0, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x0054D7E0, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x0043F927, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x0043E98E, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
		}

		TUINT32* pValues          = s_aPatchValues[ bLoadAnyLevel ? 1 : 0 ];
		*(TUINT32*)( 0x0043F3BE ) = pValues[ 0 ];
		*(TUINT32*)( 0x005de670 ) = pValues[ 1 ];
		*(TUINT32*)( 0x005eafe1 ) = pValues[ 2 ];
		*(TUINT32*)( 0x005eafe5 ) = pValues[ 3 ];
		*(TUINT32*)( 0x005E8DE0 ) = pValues[ 4 ];
		*(TUINT32*)( 0x005E8DE4 ) = pValues[ 5 ];
		*(TUINT32*)( 0x0054d7c0 ) = pValues[ 6 ];
		*(TUINT32*)( 0x0054D7E0 ) = pValues[ 7 ];
		*(TUINT32*)( 0x0043F927 ) = pValues[ 8 ];
		*(TUINT32*)( 0x0043E98E ) = pValues[ 9 ];
	}
}
