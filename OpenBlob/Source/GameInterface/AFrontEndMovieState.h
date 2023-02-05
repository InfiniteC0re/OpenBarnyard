#pragma once
#include "AGameState.h"

#include <Toshi2/T2GUI/T2GUIRectangle.h>
#include <A2GUI/A2GUIBackground.h>

class AFrontEndMovieState : public AGameState
{
public:
	constexpr static const char* s_Assets[] = {
		"bkg_Legal_Blob_ENG",
		"THQNORDIC_LOGO_Color_white",
		"logoMovie",
		"00_ChromaCity"
	};

	typedef uint32_t Asset;
	enum Asset_ : Asset
	{
		Asset_Legal,
		Asset_THQLogo,
		Asset_LogoMovie,
		Asset_Intro
	};

public:
	AFrontEndMovieState()
	{
		m_iAssetId = Asset_Legal;
	}

	virtual UpdateResult OnUpdate(float deltaTime) override;
	virtual void OnInsertion() override;
	virtual void OnRemoval() override;
	virtual void OnActivate() override;
	virtual void OnDeactivate() override;

private:
	void StartMovie(Asset assetId);

	bool IsMoviePlaying()
	{

	}

private:
	A2GUIBackground m_Background;
	float m_fUnknown; // 0x104
	Asset m_iAssetId; // 0x108
	bool m_bFlag1;    // 0x10C

	Toshi::T2GUIRectangle* m_Test;
	float m_TestRotAngle;
};