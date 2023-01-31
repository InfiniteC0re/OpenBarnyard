#include "ToshiPCH.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"
#include "XTK/DDSTextureLoader.h"
#include "stb_image.h"

namespace Toshi
{
	void TTexture::Init()
	{
		TASSERT(0 < m_DataSize, "DataSize is not empty");
		TRenderDX11* pRender = TRenderDX11::Interface();
		TString8 path("Data\\TextureOverride\\");
		path += m_TexName;

		ID3D11Resource* pResource = TNULL;
		TFile* pFile = TFile::Create(path, TFile::FileMode_Read);

		if (pFile != TNULL)
		{
			if (path.Find(".tga") <= 0)
			{
				auto fileSize = pFile->GetSize();
				uint8_t* mem = (uint8_t*)TMemalign(16, fileSize + 1);
				pFile->Read(mem, fileSize);
				mem[fileSize] = '\0';

				DirectX::CreateDDSTextureFromMemory(pRender->m_pDevice, mem, fileSize, &pResource, &m_TexInfo->SRView, 0, TNULL);

				TFree(mem);
			}
			else
			{
				TASSERT(path.IsIndexValid(0));

				int width, height, bpp;

				stbi_uc* data = stbi_load(path, &width, &height, &bpp, 4);

				if (data != NULL)
				{
					m_TexInfo->SRView = pRender->CreateTexture(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, data, 0, D3D11_USAGE_IMMUTABLE, 0, 1);
					stbi_image_free(data);
				}
				else
				{
					DirectX::CreateDDSTextureFromMemory(pRender->m_pDevice, m_TexData, m_DataSize, &pResource, &m_TexInfo->SRView, 0, TNULL);
				}
			}

			pFile->Destroy();
		}
		else
		{
			DirectX::CreateDDSTextureFromMemory(pRender->m_pDevice, m_TexData, m_DataSize, &pResource, &m_TexInfo->SRView, 0, TNULL);
		}

		if (pResource != TNULL)
		{
			pResource->Release();
		}
		else
		{
			TASSERT(TFALSE);
			return;
		}

		TTextureManager::GetSingletonWeak()->AddTexture(this);
		TTODO("FUN_00693a00");
	}

	TTexture* TTexture::InitRunTime(Info* pTextureInfo)
	{
		TRenderDX11* pRender = TRenderDX11::Interface();

		TTexture* pTexture = new TTexture;
		TASSERT(pTexture != TNULL);

		pTexture->m_TexName = "runtime";
		pTexture->m_TexInfo = pTextureInfo;
		pTexture->m_TexData = TNULL;
		TTextureManager::GetSingletonWeak()->AddTexture(pTexture);

		return pTexture;
	}

	TTexture* TTexture::InitRunTime(DXGI_FORMAT format, UINT width, UINT height, const void* srcData)
	{
		TRenderDX11* pRender = TRenderDX11::Interface();

		TTexture* pTexture = new TTexture;
		TASSERT(pTexture != TNULL);

		pTexture->m_TexName = "runtime";

		pTexture->m_TexInfo = new TTexture::Info;
		TASSERT(pTexture->m_TexInfo != TNULL);

		pTexture->m_TexInfo->SRView = pRender->CreateTexture(width, height, format, srcData, 0, D3D11_USAGE_IMMUTABLE, 0, 1);
		pTexture->m_TexInfo->Width = width;
		pTexture->m_TexInfo->Height = height;
		pTexture->m_TexInfo->MipMapCount = 1;
		pTexture->m_TexInfo->Format = format;

		TTextureManager::GetSingletonWeak()->AddTexture(pTexture);

		return pTexture;
	}

	TTextureManager::TTextureManager() :
		m_pLastTexture(TNULL)
	{
		uint8_t* srcData;

		// White texture
		constexpr int WhiteTextureWidth = 8;
		constexpr int WhiteTextureHeight = 8;
		constexpr int WhiteTextureSize = WhiteTextureWidth * WhiteTextureHeight * 4;

		srcData = (uint8_t*)TMemalign(16, WhiteTextureSize);
		TUtil::MemSet(srcData, 0xFF, WhiteTextureSize);
		m_pWhiteTexture = TTexture::InitRunTime(DXGI_FORMAT_R8G8B8A8_UNORM, WhiteTextureWidth, WhiteTextureHeight, srcData);
		m_pWhiteTexture->SetName("white");
		m_pWhiteTexture->m_pNextTexture = TNULL;
		m_pWhiteTexture->m_pPrevTexture = TNULL;
		TFree(srcData);
		
		// Invalid texture
		constexpr int InvalidTextureWidth = 128;
		constexpr int InvalidTextureHeight = 128;
		constexpr int InvalidTextureSize = InvalidTextureWidth * InvalidTextureHeight * 4;

		srcData = (uint8_t*)TMemalign(16, InvalidTextureSize);
		
		for (uint8_t* pos = srcData; pos < srcData; pos += 4)
		{
			*(pos) = rand() % 256;
			*(pos + 1) = rand() % 256;
			*(pos + 2) = rand() % 256;
			*(pos + 3) = rand() % 256;
		}

		m_pInvalidTexture = TTexture::InitRunTime(DXGI_FORMAT_R8G8B8A8_UNORM, 8, 8, srcData);
		m_pInvalidTexture->SetName("invalid");
		m_pInvalidTexture->m_pNextTexture = TNULL;
		m_pInvalidTexture->m_pPrevTexture = TNULL;
		TFree(srcData);
	}
}