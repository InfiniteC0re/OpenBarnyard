#pragma once
#include <Math/TVector4.h>

enum CowSkin
{
	CowSkin_American,
	CowSkin_Angus,
	CowSkin_Beefmaster,
	CowSkin_Spaniard,
	CowSkin_Longhorn,
	CowSkin_Brahman,
	CowSkin_Holsten
};

struct AModSettingsProperties
{
	// Common for all versions:
	TUINT uiMagic   = TFourCC( "BYSP" );
	TUINT uiVersion = 3;

	// Version 1:
	TBOOL           bShowLRTTimer    = TTRUE;
	TBOOL           bShowSpeedometer = TTRUE;
	Toshi::TVector4 vecHUDColor      = Toshi::TVector4( 253.0f / 255.0f, 226.0f / 255.0f, 1.0f / 255.0f, 1.0f );

	// Version 2:
	TBOOL bShowRTATimer = TFALSE;

	// Version 3:
	TBOOL   bForceSkin = TFALSE;
	TBOOL   bIsMale    = TTRUE;
	CowSkin eCowSkin   = CowSkin_American;
};

inline struct AModSettings
    : public AModSettingsProperties
{

	AModSettings();
	~AModSettings();

	void Save();
	void Load();

	void Apply();
	void Reset();

} g_oSettings;
