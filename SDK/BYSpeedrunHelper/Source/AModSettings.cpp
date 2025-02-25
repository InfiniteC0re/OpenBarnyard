#include "pch.h"
#include "AModSettings.h"
#include "ARunTimer.h"
#include "AGUISpeedometer.h"

#include <Plugins/PTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AModSettings::AModSettings()
{
}

AModSettings::~AModSettings()
{
}


void AModSettings::Save()
{
	PTRB                        outFile;
	PTRBSections*               pSections  = outFile.GetSections();
	PTRBSymbols*                pSymbols   = outFile.GetSymbols();
	PTRBSections::MemoryStream* pMemStream = pSections->CreateStream();

	// Write settings to the stream
	auto pSettings = pMemStream->Alloc<AModSettingsProperties>();
	*pSettings     = *this;

	// Save the file
	pSymbols->Add( pMemStream, "Settings", pSettings.get() );
	outFile.WriteToFile( "Mods\\BYSpeedrunHelper.trb" );

	TINFO( "Saved mod settings file\n" );
}

void AModSettings::Load()
{
	TTRB inFile;

	if ( inFile.Load( "Mods\\BYSpeedrunHelper.trb" ) == TTRB::ERROR_OK )
	{
		TINFO( "Reading mod settings file...\n" );
		AModSettingsProperties* pSettings = inFile.CastSymbol<AModSettingsProperties>( "Settings" );

		if ( pSettings->uiMagic != TFourCC( "BYSP" ) )
		{
			TERROR( "The settings file has wrong signature!\n" );
			return;
		}

		switch ( pSettings->uiVersion )
		{
			case 1:
				bShowTimer       = pSettings->bShowTimer;
				bShowSpeedometer = pSettings->bShowSpeedometer;
				vecHUDColor      = pSettings->vecHUDColor;
				break;
			default:
				TINFO( "The settings file's version is not supported!\n" );
				break;
		}
	}

	Apply();
}

extern ARunTimer       g_Timer;
extern AGUISpeedometer g_Speedometer;

void AModSettings::Apply()
{
	g_Timer.GetUIElement().ApplyUIStyle();
	g_Speedometer.ApplyUIStyle();
}

void AModSettings::Reset()
{
	*(AModSettingsProperties*)this = AModSettingsProperties();
	Apply();
}
