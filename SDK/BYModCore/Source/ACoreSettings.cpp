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
	PTRB outFile;
	PTRBSections* pSections = outFile.GetSections();
	PTRBSymbols*  pSymbols  = outFile.GetSymbols();
	PTRBSections::MemoryStream* pMemStream = pSections->CreateStream();

	// Write settings to the stream
	auto pSettings = pMemStream->Alloc<ACoreSettingsProperties>();
	*pSettings = *this;

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
				bLimitFPS = pSettings->bLimitFPS;
				iMaxFPS   = pSettings->iMaxFPS;
				break;
			default:
				TINFO( "The settings file's version is not supported!\n" );
				break;
		}
	}
}
