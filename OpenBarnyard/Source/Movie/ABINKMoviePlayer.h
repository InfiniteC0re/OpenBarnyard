#pragma once
#include "AMoviePlayer.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2Material.h"

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/TRenderInterface_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include <Toshi/T2SimpleArray.h>
#include <bink.h>

class ABINKMoviePlayer : public AMoviePlayer
{
public:
	TDECLARE_CLASS(AMoviePlayer);

private:
	struct Rect
	{
		TINT iPosX;
		TINT iPosY;
		TINT iWidth;
		TINT iHeight;
		Toshi::TVector2 Pos1;
		Toshi::TVector2 Pos2;
		Toshi::TVector2 UV1;
		Toshi::TVector2 UV2;
		AGUI2Material* pMaterial = TNULL;
		Toshi::TTexture* pTexture = TNULL;
		TUINT8* pData = TNULL;

		~Rect();

		TBOOL Create(int a_iPosX, int a_iPosY, int a_iWidth, int a_iHeight, TFLOAT a_iRenderPos1X, TFLOAT a_iRenderPos1Y, TFLOAT a_iRenderWidth, TFLOAT a_iRenderHeight, Toshi::TTEXTURERESOURCEFORMAT a_eFormat);
	};

public:
	ABINKMoviePlayer();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnDestroy() override;

	virtual TBOOL PlayMovie(const char* a_szFileName, TUINT32 a_Unk) override;
	virtual void StopMovie() override;
	virtual void PauseMovie(TBOOL a_bPause) override;
	virtual TBOOL IsMoviePlaying() override;
	virtual TBOOL IsMoviePaused() override;
	virtual void Render(TFLOAT a_fDeltaTime) override;
	virtual void SetMovieFile(const char* a_szFile);

private:
	TBOOL CreateSurfaces();
	TBOOL CreateAGUISurfaces(TUINT a_uiWidth, TUINT a_uiHeight, D3DFORMAT a_eFormat);

	void DestroySurfaces();

private:
	HBINK m_Bink;
	TBOOL m_bPlaying;
	TBOOL m_bPaused;
	TBOOL m_bVisible;
	char m_szMoviePath[64];
	IDirect3DSurface8* m_pSurface;
	TUINT m_eFormat;
	TBOOL m_bVideoFitsBackBuffer;
	Toshi::T2SimpleArray<Rect> m_pRects;
	TINT m_iNumRects;
};
