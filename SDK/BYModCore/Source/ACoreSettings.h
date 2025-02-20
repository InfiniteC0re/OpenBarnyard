#pragma once

struct ACoreSettingsProperties
{
	// Common for all versions:
	TUINT uiMagic   = TFourCC( "MDCR" );
	TUINT uiVersion = 1;

	// Version 1:
	TBOOL bLimitFPS        = TTRUE;
	TINT  iMaxFPS          = 200;
	TBOOL bReduceLoadTimes = TFALSE;
};

inline struct ACoreSettings
    : public ACoreSettingsProperties
{

	ACoreSettings();
	~ACoreSettings();

	void Save();
	void Load();

	void Apply();

} g_oSettings;
