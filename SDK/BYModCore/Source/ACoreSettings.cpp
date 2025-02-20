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

		static TUINT32 s_aPatchValues[ 2 ][ 4 ];
		static TBOOL   s_bSetupValues = TTRUE;

		if ( s_bSetupValues )
		{
			s_aPatchValues[ 0 ][ 0 ] = *(TUINT32*)( 0x004238AE );
			s_aPatchValues[ 0 ][ 1 ] = *(TUINT32*)( 0x005EBF9E );
			s_aPatchValues[ 0 ][ 2 ] = *(TUINT32*)( 0x005EC34E );
			s_aPatchValues[ 0 ][ 3 ] = *(TUINT32*)( 0x005EBF20 );
			s_aPatchValues[ 1 ][ 0 ] = 0x0020568E;
			s_aPatchValues[ 1 ][ 1 ] = 0x0003CF9E;
			s_aPatchValues[ 1 ][ 2 ] = 0x0003CBEE;
			s_aPatchValues[ 1 ][ 3 ] = 0x0003D01C;
			s_bSetupValues           = TFALSE;

			DWORD dwOldProtection;
			VirtualProtect( (void*)0x004238AE, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EBF9E, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EC34E, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
			VirtualProtect( (void*)0x005EBF20, 4, PAGE_EXECUTE_READWRITE, &dwOldProtection );
		}

		TUINT32* pValues          = s_aPatchValues[ bReduceLoadTimes ? 1 : 0 ];
		*(TUINT32*)( 0x004238AE ) = pValues[ 0 ];
		*(TUINT32*)( 0x005EBF9E ) = pValues[ 1 ];
		*(TUINT32*)( 0x005EC34E ) = pValues[ 2 ];
		*(TUINT32*)( 0x005EBF20 ) = pValues[ 3 ];
	}
}
