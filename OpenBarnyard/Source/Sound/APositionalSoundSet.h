#pragma once
#include <File/TTRB.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

class PBPropertyValue;

//-----------------------------------------------------------------------------
// Purpose: Ambient sounds placed in the world (the "ambientsoundbanks" of a
// library or the .trb of a minigame, e.g. Data/Sound/Mudjump.trb).
// A sound plays while the camera is closer than 100 units and the current day
// phase is in its mask; it's faded in and out.
// DAT_0078c3ec is the instance (created by AGameSystemManager).
//
// Size 0x150 in retail.
//-----------------------------------------------------------------------------
class APositionalSoundSet
    : public Toshi::TTRB
    , public Toshi::TSingleton<APositionalSoundSet>
{
public:
	// A sound of the "sounds" symbol, 0x20 bytes
	struct Sound
	{
		// Bits 0-4: the day phases the sound plays in (BITFLAG( day phase ))
		using FLAGS = TUINT32;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE       = 0,
			FLAGS_NO_FADE_IN = BITFLAG( 5 ), // no fade in on a day phase change
		};

		Toshi::TVector4 vecPosition; // 0x00
		TINT            iSoundId;    // 0x10
		TFLOAT          fVolume;     // 0x14
		FLAGS           uiFlags;     // 0x18
		TINT            iCue;        // 0x1c
	};

	static constexpr TFLOAT MAX_DISTANCE_SQ = 10000.0f;

public:
	APositionalSoundSet();
	~APositionalSoundSet();

	// Creates and destroys the instance
	static void Create();
	static void Destroy();

	// Loads the first file of the string array a_pFiles that can be loaded
	void LoadFromProperty( const PBPropertyValue* a_pFiles );

	// Unloads the sounds, a_pFiles is ignored
	void UnloadFromProperty( const PBPropertyValue* a_pFiles );

	// Loads the sounds of a .trb file, returns TFALSE if it can't be loaded
	TBOOL Load( const TCHAR* a_szFileName );

	// Stops the sounds and closes the file
	void Unload();

	// Stops the sounds (they're started again by Update)
	void StopSounds();

	// Starts and stops the sounds by the distance to the camera and the day phase
	void Update( TFLOAT a_fDeltaTime );

	// Pauses or resumes every playing sound
	void SetPaused( TBOOL a_bPaused );

private:
	TINT   m_iNumSounds;    // 0x140
	Sound* m_pSounds;       // 0x144
	TINT   m_iLastDayPhase; // 0x148
	TBOOL  m_bLoaded;       // 0x14c
	TBOOL  m_bPaused;       // 0x14d
};
