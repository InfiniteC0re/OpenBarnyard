#include "ToshiPCH.h"
#include "TTextureResourceHAL_DX8.h"

#include "TRenderInterface_DX8.h"

#include <d3dtypes.h>

namespace Toshi {

	TFreeList TTextureResourceHAL::ms_oFreeList = TFreeList(sizeof(TTextureResourceHAL), 0, 8);

	TUINT TTextureResourceHAL::GetWidth()
	{
		Validate();
		return m_ImageInfo.Width;
	}

	TUINT TTextureResourceHAL::GetHeight()
	{
		Validate();
		return m_ImageInfo.Height;
	}

	TBOOL TTextureResourceHAL::Lock(LOCKSTATE& a_rLockState)
	{
		if (m_pD3DTexture)
		{
			D3DLOCKED_RECT rect;

			HRESULT hRes = m_pD3DTexture->LockRect(0, &rect, NULL, 0);
			TASSERT(SUCCEEDED(hRes));

			if (FAILED(hRes))
			{
				TRenderD3DInterface::PrintError(hRes, "Unable to lock texture!");
			}
			
			if (SUCCEEDED(hRes))
			{
				a_rLockState.Pitch = rect.Pitch;
				a_rLockState.pBits = rect.pBits;
				m_iNumLocks += 1;

				return TTRUE;
			}
		}
		
		return TFALSE;
	}

	void TTextureResourceHAL::Unlock()
	{
		TASSERT(m_iNumLocks != 0);

		if (m_iNumLocks != 0 && m_pD3DTexture)
		{
			m_pD3DTexture->UnlockRect(0);
			m_iNumLocks -= 1;
		}
	}

	void TTextureResourceHAL::CreateShared(void* a_pUnknown)
	{
		TIMPLEMENT();
		TASSERT(TFALSE, "This is probably unused but it's important if it's actually used");

		m_bIsShared = TTRUE;
		m_bNoMipLevels = FALSE;
		m_eResourceFormat = TTEXTURERESOURCEFORMAT::Unknown;
		m_iUnk1 = 0;
		m_iUnk2 = 0;
		// ...

		TResource::Create();
		TResource::Validate();
	}

	TBOOL TTextureResourceHAL::Create(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight)
	{
		TOSHI_CORE_WARN("Deprecated! Please use: TTextureResourceHAL::CreateEx()");

		if (!CreateResource(a_pData, a_uiDataSize, a_eTextureFlags, a_uiWidth, a_uiHeight))
		{
			return TFALSE;
		}

		m_eTextureFlags = a_eTextureFlags;
		m_uiWidth = a_uiWidth;
		m_uiHeight = a_uiHeight;
		m_uiMipLevels = 0;
		m_eResourceFormat = TTEXTURERESOURCEFORMAT::Unknown;
		m_bNoMipLevels = TRUE;
		m_bLoadFromMemory = TTRUE;
		m_pData = a_pData;
		m_uiDataSize = a_uiDataSize;
		m_iUnk1 = 0;
		m_iUnk2 = 0;

		Validate();
		return TTRUE;
	}

	TBOOL TTextureResourceHAL::Create(const char* a_szFileName, TUINT a_eTextureFlags)
	{
		if (!CreateResource(a_szFileName, a_eTextureFlags))
		{
			return TFALSE;
		}

		m_eTextureFlags = a_eTextureFlags;
		m_iUnk1 = 0;
		m_iUnk2 = 0;
		m_bNoMipLevels = TRUE;

		return TTRUE;
	}

	TBOOL TTextureResourceHAL::CreateEx(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels)
	{
		if (!CreateResource(a_pData, a_uiDataSize, a_uiWidth, a_uiHeight, a_uiMipLevels, a_eFormat, a_bNoMipLevels))
		{
			return TFALSE;
		}

		m_uiWidth = a_uiWidth;
		m_pData = a_pData;
		m_uiHeight = a_uiHeight;
		m_uiDataSize = a_uiDataSize;
		m_eTextureFlags = 0x40;
		m_uiMipLevels = a_uiMipLevels;
		m_eResourceFormat = a_eFormat;
		m_bNoMipLevels = a_bNoMipLevels;
		m_bLoadFromMemory = TTRUE;
		m_iUnk1 = 0;
		m_iUnk2 = 0;

		Validate();
		return TTRUE;
	}

	TBOOL TTextureResourceHAL::CreateFromFormat()
	{
		TUINT uiMipLevels = (m_bNoMipLevels == TRUE) ? m_uiMipLevels : 0;

		switch (m_eResourceFormat)
		{
		case TTEXTURERESOURCEFORMAT::R8G8B8A8:
			return CreateFromMemory8888(m_uiWidth, m_uiHeight, uiMipLevels, m_pData);
		case TTEXTURERESOURCEFORMAT::R8G8B8:
			return CreateFromMemory888(m_uiWidth, m_uiHeight, uiMipLevels, m_pData);
		case TTEXTURERESOURCEFORMAT::R5G5B5A1:
			return CreateFromMemory5551(m_uiWidth, m_uiHeight, uiMipLevels, m_pData);
		case TTEXTURERESOURCEFORMAT::DDS:
			return CreateFromMemoryDDS(m_uiWidth, m_uiHeight, uiMipLevels, m_pData);
		case TTEXTURERESOURCEFORMAT::R4G4B4A4:
			return CreateFromMemory4444(m_uiWidth, m_uiHeight, uiMipLevels, m_pData);
		default:
			TASSERT(TFALSE, "Unknown format!");
			return TFALSE;
		}
	}

	TBOOL TTextureResourceHAL::CreateFromMemory8888(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TTextureResourceHAL::CreateFromMemory888(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TTextureResourceHAL::CreateFromMemory5551(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TTextureResourceHAL::CreateFromMemory4444(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TTextureResourceHAL::CreateFromMemoryDDS(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData)
	{
		auto pRenderer = TSTATICCAST(TRenderD3DInterface*, GetRenderer());

		HRESULT hRes = D3DXCreateTextureFromFileInMemoryEx(
			pRenderer->GetDirect3DDevice(),
			a_pData,
			m_uiDataSize,
			-1,
			-1,
			a_uiLevels,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DFILTER_LINEARMIPNEAREST,
			D3DFILTER_LINEARMIPNEAREST,
			0,
			&m_ImageInfo,
			NULL,
			&m_pD3DTexture
		);

		if (FAILED(hRes))
		{
			TRenderD3DInterface::PrintError(hRes, TNULL);
		}

		return hRes == S_OK;
	}

	TBOOL TTextureResourceHAL::CreateFromFileDDS(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const char* a_szFile)
	{
		auto pRenderer = TSTATICCAST(TRenderD3DInterface*, GetRenderer());

		HRESULT hRes = D3DXCreateTextureFromFileExA(
			pRenderer->GetDirect3DDevice(),
			a_szFile,
			-1,
			-1,
			a_uiLevels,
			0,
			D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED,
			D3DFILTER_LINEARMIPNEAREST,
			D3DFILTER_LINEARMIPNEAREST,
			0,
			&m_ImageInfo,
			NULL,
			&m_pD3DTexture
		);

		if (FAILED(hRes))
		{
			TRenderD3DInterface::PrintError(hRes, TNULL);
		}

		return hRes == S_OK;
	}

	TBOOL TTextureResourceHAL::IsPPM(const char* a_szName)
	{
		auto iLen = TStringManager::String8Length(a_szName);

		if (iLen >= 4)
		{
			return TStringManager::String8Compare(a_szName + iLen - 4, ".ppm") == 0;
		}

		return TFALSE;
	}

	TBOOL TTextureResourceHAL::Validate()
	{
		if (IsCreated() && IsValid())
		{
			return TTRUE;
		}

		auto pRenderer = TSTATICCAST(TRenderD3DInterface*, GetRenderer());

		if (m_bLoadFromMemory)
		{
			// Load from memory
			if (m_pData && m_uiDataSize != 0)
			{
				if (HASFLAG(m_eTextureFlags & 0x40))
				{
					if (!CreateFromFormat())
					{
						return TFALSE;
					}
				}

				TUINT uiLayout = m_eTextureFlags & 0x38;

				if (uiLayout == 8)
				{
					CreateFromMemory5551(m_uiWidth, m_uiHeight, 0, m_pData);
				}
				else if (uiLayout == 16)
				{
					CreateFromMemory4444(m_uiWidth, m_uiHeight, 0, m_pData);
				}
				else if (uiLayout == 32)
				{
					CreateFromMemory8888(m_uiWidth, m_uiHeight, 0, m_pData);
				}
				
				CreateFromMemoryDDS(m_uiWidth, m_uiHeight, -1, m_pData);
			}
		}
		else
		{
			// Load from file
			if (m_pNameEntry && !IsPPM(m_pNameEntry->GetName()))
			{
				CreateFromFileDDS(m_uiWidth, m_uiHeight, -1, m_pNameEntry->GetName());
			}
		}

		TFIXME("Doesn't affect anything but here should be some manipulations with s_iHALMemoryUsage");
		return TResource::Validate();
	}

	void TTextureResourceHAL::Invalidate()
	{
		if (IsCreated() && IsValid()) 
		{
			if (m_bIsShared == TTRUE)
			{
				m_pD3DTexture = TNULL;
			}
			else if (m_pD3DTexture)
			{
				TFIXME("Doesn't affect anything but here should be some manipulations with s_iHALMemoryUsage");
				m_pD3DTexture->Release();
			}

			TResource::Invalidate();
		}
	}

}
