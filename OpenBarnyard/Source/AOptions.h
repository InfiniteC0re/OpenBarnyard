#pragma once
#include <Toshi/Locale/T2Locale.h>

class AOptions :
	public Toshi::TGenericClassDerived<AOptions, Toshi::TObject, "AOptions", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AOptions>
{
public:
	using UpdatedSettings = TUINT32;
	enum UpdatedSettings_ : UpdatedSettings
	{
		UpdatedSettings_None = 0,
		UpdatedSettings_Audio = BITFIELD(0),
		UpdatedSettings_Language = BITFIELD(1),
		UpdatedSettings_Input = BITFIELD(2),
		UpdatedSettings_ForceUpdate = 0xFFFFFFFFU,
	};

public:
	AOptions();

	virtual TBOOL ApplySettings();

	void LoadSettings();

	void ReloadSettings()
	{
		LoadSettings();
		m_eUpdatedSettings = UpdatedSettings_ForceUpdate;
	}

	TBOOL IsResolutionCompatible(TINT a_iWidth, TINT a_iHeight);

	Toshi::T2Locale::Lang GetLanguage() const { return m_eLanguage; }
	TFLOAT GetGamma() const { return m_fGamma; }
	TFLOAT GetSensitivity() const { return m_fSensitivity; }
	TINT GetWidth() const { return m_iWidth; }
	TINT GetHeight() const { return m_iHeight; }
	TBOOL IsWindowed() const { return m_bForcedWindowed; }
	TBOOL IsControllerEnabled() const { return m_bControllerEnabled; }
	TBOOL IsHighDetailGrassEnabled() const { return m_bHighDetailGrass; }

private:
	Toshi::T2Locale::Lang m_eLanguage;
	TFLOAT m_fGamma;
	TBOOL m_bForcedWindowed;
	TINT m_iWidth;
	TINT m_iHeight;
	TFLOAT m_fSensitivity;
	TBOOL m_bHighDetailGrass;
	TBOOL m_bControllerEnabled;
	UpdatedSettings m_eUpdatedSettings;
};