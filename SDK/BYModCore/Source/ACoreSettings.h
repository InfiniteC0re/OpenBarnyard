#pragma once

struct ACoreSettingsProperties
{
	// Common for all versions:
	TUINT uiMagic   = TFourCC( "MDCR" );
	TUINT uiVersion = 1;

	// Version 1:
	TBOOL bLimitFPS = TTRUE;
	TINT  iMaxFPS   = 100;
};

inline struct ACoreSettings
    : public ACoreSettingsProperties
{
	
	ACoreSettings();
	~ACoreSettings();

	void Save();
	void Load();

} g_oSettings;
