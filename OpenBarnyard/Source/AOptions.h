#pragma once
#include <T2Locale/T2Locale.h>

class MODAPI AOptions : public Toshi::TObject
    , public Toshi::TSingleton<AOptions>
{
public:
	TDECLARE_CLASS( AOptions, Toshi::TObject );

	using Flags = TUINT32;
	enum Flags_ : Flags
	{
		Flags_Rumble      = BITFLAG( 0 ), // set by default, there's no option for it on PC
		Flags_InvertMouse = BITFLAG( 1 )
	};

	using UpdatedSettings = TUINT32;
	enum UpdatedSettings_ : UpdatedSettings
	{
		UpdatedSettings_None     = 0,
		UpdatedSettings_Audio    = BITFLAG( 0 ),
		UpdatedSettings_Language = BITFLAG( 1 ),
		UpdatedSettings_Input    = BITFLAG( 2 ),
#ifdef BARNYARD_LATEST
		UpdatedSettings_Camera = BITFLAG( 4 ), // nothing applies it, the camera reads the speed directly
#endif                                         // BARNYARD_LATEST
		UpdatedSettings_ForceUpdate = 0xFFFFFFFFU,
	};

public:
	AOptions();
	~AOptions();

	virtual TBOOL ApplySettings();

	void LoadSettings();

	void ReloadSettings()
	{
		LoadSettings();
		m_eUpdatedSettings = UpdatedSettings_ForceUpdate;
	}

	TBOOL IsResolutionCompatible( TINT a_iWidth, TINT a_iHeight );

	// Copies the settings (0x2c bytes from m_iUnk04 to m_bControllerEnabled) into the save header
	TBOOL SaveSettings( void* a_pBuffer, TINT a_iSize );

	// Copies the settings back from the save header, every setting gets updated
	TBOOL LoadSavedSettings( const void* a_pBuffer, TINT a_iSize );

	// Sets the language, applied by ApplySettings
	void SetLanguage( Toshi::T2Locale::Lang a_eLanguage );

	// Size of the settings stored in the save
	static constexpr TINT SAVED_SETTINGS_SIZE = 0x2c;

	Toshi::T2Locale::Lang GetLanguage() const { return m_eLanguage; }
	TFLOAT                GetGamma() const { return m_fGamma; }
	TFLOAT                GetSensitivity() const { return m_fSensitivity; }
	TINT                  GetWidth() const { return m_iWidth; }
	TINT                  GetHeight() const { return m_iHeight; }
	TBOOL                 IsWindowed() const;
	TBOOL                 IsControllerEnabled() const { return m_bControllerEnabled; }
	TBOOL                 IsHighDetailGrassEnabled() const { return m_bHighDetailGrass; }
	TBOOL                 IsMouseInverted() const { return m_eFlags & Flags_InvertMouse; }
	TBOOL                 IsRumbleEnabled() const { return m_eFlags & Flags_Rumble; }
	TINT                  GetSfxVolume() const { return m_iSfxVolume; }
	TINT                  GetMusicVolume() const { return m_iMusicVolume; }
	TINT                  GetVOVolume() const { return m_iVOVolume; }

#ifdef BARNYARD_LATEST
	// Camera rotation speed, 0 to CAMERA_SPEED_MAX (ACameraOptionsState)
	static constexpr TINT CAMERA_SPEED_DEFAULT = 3;
	static constexpr TINT CAMERA_SPEED_MAX     = 5;

	TINT GetCameraSpeed() const { return m_uiCameraSpeed; }

	// $BarnyardWiiJP: FUNCTION 804398b0
	void SetCameraSpeed( TINT a_iSpeed )
	{
		m_uiCameraSpeed = TUINT8( a_iSpeed );
		m_eUpdatedSettings |= UpdatedSettings_Camera;
	}

	// NOTE: not in the original. "Wii Style" camera option (ACameraOptionsState): TTRUE uses the camera behaviours
	// of the latest version (walking camera, bike camera, sunglasses aim camera, car chase follow view), TFALSE the
	// PC retail ones. Read every frame by the cameras, so it switches live.
	TBOOL IsWiiStyleCamera() const { return m_bWiiStyleCamera; }

	void SetWiiStyleCamera( TBOOL a_bEnabled )
	{
		m_bWiiStyleCamera = a_bEnabled;
		m_eUpdatedSettings |= UpdatedSettings_Camera;
	}

	// NOTE: not in the original. "Follow Jumps" camera option: TTRUE makes the walking camera follow the player's
	// jumps (PC retail), TFALSE holds its height while the player jumps (latest version, AGTAStyleCameraHelper::HoldHeight).
	// Independent of the Wii Style option.
	TBOOL IsFollowJumpsCamera() const { return m_bFollowJumpsCamera; }

	void SetFollowJumpsCamera( TBOOL a_bEnabled )
	{
		m_bFollowJumpsCamera = a_bEnabled;
		m_eUpdatedSettings |= UpdatedSettings_Camera;
	}

	// Same as IsWiiStyleCamera of the singleton, TTRUE (the default) before it's created
	static TBOOL IsWiiCameraEnabled() { return !IsSingletonCreated() || GetSingleton()->IsWiiStyleCamera(); }

	// TTRUE if the walking camera holds its height while the player jumps (Follow Jumps off, the default)
	static TBOOL IsJumpHeightHoldEnabled() { return !IsSingletonCreated() || !GetSingleton()->IsFollowJumpsCamera(); }

	// NOTE: not in the original. Difficulty of the latest version (ADifficultyOptionState, APlayerProgress::IsEasyDifficulty):
	// TTRUE is Easy, TFALSE (the default) is Normal, which plays like PC retail.
	TBOOL IsEasyDifficulty() const { return m_bEasyDifficulty; }
	void  SetEasyDifficulty( TBOOL a_bEasy ) { m_bEasyDifficulty = a_bEasy; }

	// Same as IsEasyDifficulty of the singleton, TFALSE (Normal) before it's created
	static TBOOL IsEasyDifficultyEnabled() { return IsSingletonCreated() && GetSingleton()->IsEasyDifficulty(); }

	// NOTE: not in the original. Settings the original game doesn't have (the camera and difficulty options) are kept in
	// OpenBarnyard.cfg next to savedata.by, so the save stays exactly what PC retail writes.
	// The options screens call SaveExtraSettings when a setting changes (retried by the destructor); the setters don't.
	void LoadExtraSettings();
	void SaveExtraSettings();

private:
	static void GetExtraSettingsPath( TCHAR ( &a_rPath )[ 260 ], TCHAR ( &a_rDir )[ 260 ] );
	TBOOL       WriteExtraSettings();

public:
#endif // BARNYARD_LATEST

	// $Barnyard: FUNCTION 00663290
	void SetMouseInverted( TBOOL a_bInverted )
	{
		if ( a_bInverted )
			m_eFlags |= Flags_InvertMouse;
		else
			m_eFlags &= ~Flags_InvertMouse;

		m_eUpdatedSettings |= UpdatedSettings_Input;
	}

	// $Barnyard: FUNCTION 006632c0
	void SetSensitivity( TFLOAT a_fSensitivity )
	{
		m_fSensitivity = a_fSensitivity;
		m_eUpdatedSettings |= UpdatedSettings_Input;
	}

	// $Barnyard: FUNCTION 00663250
	void SetSfxVolume( TINT a_iVolume )
	{
		m_iSfxVolume = a_iVolume;
		m_eUpdatedSettings |= UpdatedSettings_Audio;
	}

	// $Barnyard: FUNCTION 00663260
	void SetMusicVolume( TINT a_iVolume )
	{
		m_iMusicVolume = a_iVolume;
		m_eUpdatedSettings |= UpdatedSettings_Audio;
	}

	// $Barnyard: FUNCTION 00663270
	void SetVOVolume( TINT a_iVolume )
	{
		m_iVOVolume = a_iVolume;
		m_eUpdatedSettings |= UpdatedSettings_Audio;
	}

private:
	TINT                  m_iUnk04;                // 0x04
	TINT                  m_iSfxVolume;            // 0x08, 0-255
	TINT                  m_iMusicVolume;          // 0x0c, 0-255
	TINT                  m_iVOVolume;             // 0x10, 0-255
	Toshi::T2Locale::Lang m_eLanguage;             // 0x14
	Flags                 m_eFlags = BITFLAG( 0 ); // 0x18
	TFLOAT                m_fGamma;                // 0x1c
	TBOOL                 m_bForcedWindowed;       // 0x20
	TUINT8                PADDING[ 3 ];            // 0x21, saved too (SAVED_SETTINGS_SIZE)
	TINT                  m_iWidth;                // 0x24
	TINT                  m_iHeight;               // 0x28
	TFLOAT                m_fSensitivity;          // 0x2c
	TBOOL                 m_bHighDetailGrass;      // 0x30
	TBOOL                 m_bControllerEnabled;    // 0x31
	UpdatedSettings       m_eUpdatedSettings;      // 0x34
#ifdef BARNYARD_LATEST
	// NOTE: not in the original, see LoadExtraSettings
	TUINT8 m_uiCameraSpeed;
	TBOOL  m_bWiiStyleCamera;
	TBOOL  m_bFollowJumpsCamera;
	TBOOL  m_bEasyDifficulty;
	TBOOL  m_bExtraSettingsChanged;
#endif // BARNYARD_LATEST
};

#ifndef BARNYARD_LATEST
TSTATICASSERT( sizeof( AOptions ) == 0x38 );
#endif // BARNYARD_LATEST
