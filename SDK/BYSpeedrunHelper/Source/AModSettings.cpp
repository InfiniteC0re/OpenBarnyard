#include "pch.h"
#include "AModSettings.h"
#include "AUIManager.h"

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
			case 3:
				bForceSkin  = pSettings->bForceSkin;
				bRandomSkin = pSettings->bRandomSkin;
				bIsMale     = pSettings->bIsMale;
				eCowSkin    = pSettings->eCowSkin;
			case 2:
				bShowRTATimer = pSettings->bShowRTATimer;
			case 1:
				bShowLRTTimer    = pSettings->bShowLRTTimer;
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

void AModSettings::Apply()
{
	if ( AUIManager::IsSingletonCreated() )
		AUIManager::GetSingleton()->ApplyUIStyle();
}

void AModSettings::Reset()
{
	*(AModSettingsProperties*)this = AModSettingsProperties();
	Apply();
}
