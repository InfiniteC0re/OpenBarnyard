#include "pch.h"

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/TTextureResourceHAL_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#include "ABINKMoviePlayer.h"

#include <Toshi/TSystem.h>
#include <Render/TTextureFactory.h>

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ABINKMoviePlayer);

ABINKMoviePlayer::ABINKMoviePlayer()
{
	m_Bink = TNULL;
	m_bPlaying = TFALSE;
	m_bPaused = TFALSE;
	m_bVisible = TFALSE;
	m_pSurface = TNULL;
	m_iNumRects = 0;
}

TBOOL ABINKMoviePlayer::OnCreate()
{
	BinkSetMemory(
		[](unsigned int size)
		{
			return TMalloc(size);
		},
		[](void* ptr)
		{
			TFree(ptr);
		}
	);
	
	int iOutput = FSOUND_GetOutput();

	if (iOutput == FSOUND_OUTPUT_WINMM)
	{
		BinkSetSoundSystem(BinkOpenWaveOut, 0);
	}
	else if (iOutput == FSOUND_OUTPUT_DSOUND)
	{
		auto outHandle = FSOUND_GetOutputHandle();

		if (outHandle)
		{
			BinkSetSoundSystem(BinkOpenDirectSound, TREINTERPRETCAST(int, outHandle));
		}
	}

	return TTRUE;
}

TBOOL ABINKMoviePlayer::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (!IsMoviePlaying())
	{
		return TTRUE;
	}

	if (m_Bink->LastFrameNum == m_Bink->Frames)
	{
		StopMovie();
		return TTRUE;
	}

	if (g_oSystemManager.IsPaused())
	{
		if (!m_bPaused)
		{
			BinkPause(m_Bink, 1);
			BinkSetSoundOnOff(m_Bink, 0);
			m_bPaused = TTRUE;
		}
	}
	else
	{
		if (m_bPaused)
		{
			BinkPause(m_Bink, 0);
			BinkSetSoundOnOff(m_Bink, 1);
			m_bPaused = TFALSE;
		}
	}

	auto iWaitRes = BinkWait(m_Bink);

	if (iWaitRes == 0)
	{
		if (BinkDoFrame(m_Bink) == 0)
		{
			if (!m_bVideoFitsBackBuffer || !m_pSurface)
			{
				if (!m_pRects.GetArray())
				{
					if (!CreateSurfaces())
					{
						return TTRUE;
					}

					m_eFormat |= 0x80000000;
				}
			}

			auto ppRenderSurface = &m_pSurface;

			if (m_bVideoFitsBackBuffer ||
				TSTATICCAST(TTextureResourceHAL*, m_pRects[0].pTexture)->GetD3DTexture()->GetSurfaceLevel(0, ppRenderSurface) == S_OK)
			{
				D3DLOCKED_RECT rect;
				if (S_OK == (*ppRenderSurface)->LockRect(&rect, TNULL, 0))
				{
					BinkCopyToBuffer(m_Bink, rect.pBits, rect.Pitch, m_Bink->Height, 0, 0, m_eFormat);
					m_eFormat &= 0x7fffffff;
					(*ppRenderSurface)->UnlockRect();

					if (!m_bVideoFitsBackBuffer)
					{
						(*ppRenderSurface)->Release();
						(*ppRenderSurface) = TNULL;
					}
				}
			}
		}

		BinkNextFrame(m_Bink);
	}

	m_bVisible = TTRUE;
	return TTRUE;
}

void ABINKMoviePlayer::OnDestroy()
{

}

TBOOL ABINKMoviePlayer::PlayMovie(const TCHAR* a_szFileName, TUINT32 a_Unk)
{
	TVALIDPTR(a_szFileName);

	SetMovieFile(a_szFileName);
	m_Bink = BinkOpen(m_szMoviePath, 0);

	if (m_Bink != TNULL)
	{
		if (!CreateSurfaces())
		{
			BinkClose(m_Bink);
			m_Bink = TNULL;
			return TFALSE;
		}

		m_bPlaying = TTRUE;
		m_bVisible = TFALSE;
		m_bPaused = TFALSE;
		OnUpdate(0.0f);
	}

	return m_Bink != TNULL;
}

TBOOL ABINKMoviePlayer::CreateSurfaces()
{
	auto pRender = TRenderD3DInterface::Interface();
	auto pDevice = pRender->GetDirect3DDevice();

	IDirect3DSurface8* pSurface;
	auto hRes = pDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurface);

	if (hRes == S_OK)
	{
		D3DSURFACE_DESC desc;
		hRes = pSurface->GetDesc(&desc);

		if (hRes == S_OK)
		{
			switch (desc.Format)
			{
			case D3DFMT_R8G8B8:
				m_eFormat = 1;
				break;
			case D3DFMT_A8R8G8B8:
				m_eFormat = 5;
				break;
			case D3DFMT_X8R8G8B8:
				m_eFormat = 3;
				break;
			case D3DFMT_R5G6B5:
				m_eFormat = 10;
				break;
			case D3DFMT_X1R5G5B5:
				m_eFormat = 9;
				break;
			case D3DFMT_A1R5G5B5:
				m_eFormat = 8;
				break;
			case D3DFMT_A4R4G4B4:
				m_eFormat = 7;
				break;
			}
		}

		if (desc.Width == m_Bink->Width && desc.Height == m_Bink->Height)
		{
			m_bVideoFitsBackBuffer = TTRUE;
			hRes = pDevice->CreateImageSurface(desc.Width, desc.Height, desc.Format, &m_pSurface);
		}
		else
		{
			m_bVideoFitsBackBuffer = TFALSE;
			CreateAGUISurfaces(desc.Width, desc.Height, desc.Format);
		}

		pSurface->Release();
	}

	return hRes == S_OK;
}

void ABINKMoviePlayer::StopMovie()
{
	if (m_Bink)
	{
		BinkClose(m_Bink);
		m_Bink = TNULL;
		DestroySurfaces();
	}

	m_bPlaying = TFALSE;
	m_bVisible = TFALSE;
	m_bPaused = TFALSE;
}

void ABINKMoviePlayer::PauseMovie(TBOOL a_bPause)
{
	m_bPaused = a_bPause;
}

TBOOL ABINKMoviePlayer::IsMoviePlaying()
{
	return m_bPlaying;
}

TBOOL ABINKMoviePlayer::IsMoviePaused()
{
	return m_bPaused;
}

void ABINKMoviePlayer::Render(TFLOAT a_fDeltaTime)
{
	if (IsMoviePlaying() && m_bVisible)
	{
		auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

		if (m_bVideoFitsBackBuffer)
		{
			IDirect3DSurface8* pSurface;
			auto hRes = pDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurface);

			if (hRes == S_OK)
			{
				pDevice->CopyRects(m_pSurface, TNULL, 0, pSurface, TNULL);
				pSurface->Release();
			}
		}
		else
		{
			auto pRender = AGUI2::GetRenderer();

			pRender->BeginScene();

			for (TINT i = 0; i < m_iNumRects; i++)
			{
				pRender->SetMaterial(m_pRects[i].pMaterial);
				pRender->RenderRectangle(m_pRects[i].Pos1, m_pRects[i].Pos2, m_pRects[i].UV1, m_pRects[i].UV2);
			}

			pRender->EndScene();
		}
	}
}

void ABINKMoviePlayer::SetMovieFile(const TCHAR* a_szFile)
{
	TStringManager::String8Format(m_szMoviePath, sizeof(m_szMoviePath), "Data/Movies/%s.bik", a_szFile);
}

TBOOL ABINKMoviePlayer::CreateAGUISurfaces(TUINT a_uiWidth, TUINT a_uiHeight, D3DFORMAT a_eFormat)
{
	auto pDevice = TRenderD3DInterface::Interface()->GetDirect3DDevice();

	D3DCAPS8 caps;
	auto hRes = pDevice->GetDeviceCaps(&caps);

	if (hRes == S_OK)
	{
		TUINT uiWidth = m_Bink->Width;
		TUINT uiHeight = m_Bink->Height;
		TUINT uiMaxTextureWidth = caps.MaxTextureWidth;
		TUINT uiMaxTextureHeight = caps.MaxTextureHeight;

		m_iNumRects = ((uiHeight - 1) + uiMaxTextureHeight) / uiMaxTextureHeight * ((uiWidth - 1) + uiMaxTextureWidth) / uiMaxTextureWidth;
		m_pRects.Create(m_iNumRects);

		auto pDisplayParams = TRenderD3DInterface::Interface()->GetCurrentDisplayParams();

		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);

		TFLOAT fScaleX = a_uiWidth / ((pDisplayParams->uiWidth / fWidth) * m_Bink->Width);
		TFLOAT fScaleY = a_uiHeight / ((pDisplayParams->uiHeight / fHeight) * m_Bink->Height);

		TTEXTURERESOURCEFORMAT eFormat;

		if (a_eFormat == D3DFMT_A8R8G8B8)
		{
			eFormat = TTEXTURERESOURCEFORMAT::R8G8B8A8;
		}
		else if (a_eFormat == D3DFMT_X8R8G8B8)
		{
			eFormat = TTEXTURERESOURCEFORMAT::R8G8B8;
		}
		else if (a_eFormat == D3DFMT_A1R5G5B5)
		{
			eFormat = TTEXTURERESOURCEFORMAT::R5G5B5A1;
		}
		else
		{
			return TFALSE;
		}

		TFLOAT fVideoWidth = TMath::Abs(TFLOAT(m_Bink->Width));
		TFLOAT fVideoHeight = TMath::Abs(TFLOAT(m_Bink->Height));

		TUINT uiPosY = 0;
		TUINT uiRectIndex = 0;

		TUINT uiLeftHeight = uiHeight;
		while (0 < TINT(uiLeftHeight))
		{
			TUINT uiRectHeight = TMath::Min(uiLeftHeight, uiMaxTextureHeight);
			uiWidth = m_Bink->Width;

			TUINT uiPosX = 0;
			TUINT uiLeftWidth = m_Bink->Width;

			while (0 < TINT(uiLeftWidth))
			{
				TUINT uiRectWidth = TMath::Min(uiLeftWidth, uiMaxTextureWidth);
				TBOOL bCreated = m_pRects[uiRectIndex].Create(
					uiPosX,
					uiPosY,
					uiRectWidth,
					uiRectHeight,
					(uiPosX + fVideoWidth * -0.5f) * fScaleX,
					(uiPosY + fVideoHeight * -0.5f) * fScaleY,
					uiRectWidth * fScaleX,
					uiRectHeight * fScaleY,
					eFormat
				);

				if (!bCreated)
				{
					TASSERT(TFALSE, "ABINKMoviePlayer: Unable to create rectangle");
					m_pRects.Destroy();
					return TFALSE;
				}

				uiPosX += TMath::Min(uiWidth, uiMaxTextureWidth);

				uiLeftWidth -= uiRectWidth;
				uiRectIndex++;
			}

			uiPosY += uiRectHeight;
			uiLeftHeight -= uiRectHeight;
		}
	}

	return hRes == S_OK;
}

void ABINKMoviePlayer::DestroySurfaces()
{
	if (m_pSurface)
	{
		m_pSurface->Release();
		m_pSurface = TNULL;
	}

	m_pRects.Destroy();
	m_iNumRects = 0;
}

ABINKMoviePlayer::Rect::~Rect()
{
	if (pMaterial)
	{
		AGUI2::GetRenderer()->DestroyMaterial(pMaterial);
		pMaterial = TNULL;
	}

	if (pTexture)
	{
		pTexture->DestroyResource();
		pTexture = TNULL;
	}

	delete[] pData;
	pData = TNULL;
}

TBOOL ABINKMoviePlayer::Rect::Create(int a_iPosX, int a_iPosY, int a_iWidth, int a_iHeight, TFLOAT a_iRenderPos1X, TFLOAT a_iRenderPos1Y, TFLOAT a_iRenderWidth, TFLOAT a_iRenderHeight, TTEXTURERESOURCEFORMAT a_eFormat)
{
	iPosY = a_iPosY;
	iPosX = a_iPosX;
	iWidth = a_iWidth;
	iHeight = a_iHeight;
	Pos1 = { a_iRenderPos1X, a_iRenderPos1Y };
	Pos2 = { a_iRenderPos1X + a_iRenderWidth, a_iRenderPos1Y + a_iRenderHeight };

	TUINT32 uiWidth = TMath::RoundToNextPowerOfTwo(a_iWidth);
	TUINT32 uiHeight = TMath::RoundToNextPowerOfTwo(a_iHeight);
	TINT iPixelSize;

	if (a_eFormat == TTEXTURERESOURCEFORMAT::Unknown)
	{
		return TFALSE;
	}
	else if (a_eFormat == TTEXTURERESOURCEFORMAT::R8G8B8A8 || a_eFormat == TTEXTURERESOURCEFORMAT::R8G8B8)
	{
		iPixelSize = 4;
	}
	else if (a_eFormat == TTEXTURERESOURCEFORMAT::R5G5B5A1)
	{
		iPixelSize = 2;
	}
	else
	{
		return TFALSE;
	}

	auto uiDataSize = iPixelSize * uiWidth * uiHeight;
	pData = new TUINT8[uiDataSize];
	auto pTexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>(SYSRESOURCE_TEXTUREFACTORY);

	pTexture = pTexFactory->CreateEx(pData, uiDataSize, uiWidth, uiHeight, 1, a_eFormat, 4);
	pTexture->SetAddress(2);

	pMaterial = AGUI2::GetRenderer()->CreateMaterial(pTexture);
	pMaterial->SetBlendState(0);

	UV1 = { 0.0f, 0.0f };
	UV2 = { TFLOAT(a_iWidth) / uiWidth, TFLOAT(a_iHeight) / uiHeight };

	return TTRUE;
}
