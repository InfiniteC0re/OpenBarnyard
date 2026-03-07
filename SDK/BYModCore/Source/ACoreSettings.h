#pragma once

struct ACoreSettingsProperties
{
	// Common for all versions:
	TUINT uiMagic   = TFourCC( "MDCR" );
	TUINT uiVersion = 2;

	// Version 1:
	TBOOL bLimitFPS        = TTRUE;
	TINT  iMaxFPS          = 200;
	TBOOL bReduceLoadTimes = TFALSE;

	// Version 2:
	TBOOL bBetterGrass = TTRUE;

	// Not saved:
	TBOOL bLoadAnyLevel             = TFALSE;
	TBOOL bForceAllInstances        = TFALSE;
	TBOOL bDisableInstanceRendering = TFALSE;
	TBOOL bDisableTreeRendering     = TFALSE;
	TBOOL bDisableInstanceLODs      = TTRUE;

	TINT   iInstanceMaxRendered         = 1024;
	TFLOAT flInstanceRenderDistance     = 220.0f;
	TFLOAT flInstanceBoundingMultiplier = 1.2f;
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
