#include "ToshiPCH.h"
#include "TTexture_SDL.h"
#include "TRender_SDL.h"

#include "SOIL2/SOIL2.h"

namespace Toshi {

	void TTexture::Init()
	{
		TASSERT(0 < m_DataSize, "DataSize is not empty");
		TRenderSDL* pRender = TRenderSDL::Interface();
		TString8 path("Data\\TextureOverride\\");
		path += m_TexName;

		TFile* pFile = TFile::Create(path, TFile::FileMode_Read);

		if (pFile != TNULL)
		{
			auto fileSize = pFile->GetSize();
			uint8_t* mem = (uint8_t*)TMemalign(16, fileSize + 1);
			pFile->Read(mem, fileSize);
			pFile->Destroy();
			mem[fileSize] = '\0';

			int width, height, channels;
			unsigned char* texData = SOIL_load_image_from_memory(mem, fileSize, &width, &height, &channels, 4);
			TFree(mem);

			if (texData)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_TexInfo->Handle);

				// FIXME: use settings of the texture here
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
				SOIL_free_image_data(texData);
			}
			else
			{
				TASSERT(TFALSE);
				return;
			}
		}
		else
		{
			int width, height, channels;
			unsigned char* texData = SOIL_load_image_from_memory(m_TexData, m_DataSize, &width, &height, &channels, 4);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_TexInfo->Handle);

			// FIXME: use settings of the texture here
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_TexInfo->Width, m_TexInfo->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
			SOIL_free_image_data(texData);
		}

		TTextureManager::GetSingleton()->AddTexture(this);
		SelectSamplerId();

#ifdef TOSHI_DEBUG
		TTextureManager::s_NumTextures += 1;
#endif
	}

	void TTexture::Deinit()
	{
		glDeleteTextures(1, &m_TexInfo->Handle);

		if (m_pPrevTexture)
			m_pPrevTexture->m_pNextTexture = m_pNextTexture;

		if (m_pNextTexture)
			m_pNextTexture->m_pPrevTexture = m_pPrevTexture;

		auto pTexManager = TTextureManager::GetSingleton();

		if (pTexManager->GetLastTexture() == this)
			pTexManager->SetLastTexture(m_pPrevTexture);

		m_pNextTexture = TNULL;
		m_pPrevTexture = TNULL;

#ifdef TOSHI_DEBUG
		TASSERT(TTextureManager::s_NumTextures > 0);
		TTextureManager::s_NumTextures -= 1;
#endif
	}

	void TTexture::Bind(GLenum a_eTarget)
	{
		glBindTexture(a_eTarget, m_TexInfo->Handle);
	}

	TTexture* TTexture::InitRunTime(Info* pTextureInfo)
	{
		TRenderSDL* pRender = TRenderSDL::Interface();

		TTexture* pTexture = new TTexture;
		TASSERT(pTexture != TNULL);

		pTexture->m_TexName = "runtime";
		pTexture->m_TexInfo = pTextureInfo;
		pTexture->m_TexData = TNULL;
		TTextureManager::GetSingleton()->AddTexture(pTexture);

		return pTexture;
	}

	TTexture* TTexture::InitRunTime(TEXTURE_FORMAT format, UINT width, UINT height, const void* srcData)
	{
		TRenderSDL* pRender = TRenderSDL::Interface();

		TTexture* pTexture = new TTexture;
		TASSERT(pTexture != TNULL);

		pTexture->m_TexName = "runtime";

		pTexture->m_TexInfo = new TTexture::Info;
		TASSERT(pTexture->m_TexInfo != TNULL);

		GLenum glFormat =
			(format == TEXTURE_FORMAT_R8G8B8A8_UNORM) ? GL_RGBA :
			(format == TEXTURE_FORMAT_R8G8_UNORM) ? GL_RG : GL_R;

		pTexture->m_TexInfo->Handle = pRender->CreateTexture(width, height, glFormat, TFALSE, srcData);
		pTexture->m_TexInfo->Width = width;
		pTexture->m_TexInfo->Height = height;
		pTexture->m_TexInfo->MipMapCount = 1;
		pTexture->m_TexInfo->Format = format;

		TTextureManager::GetSingleton()->AddTexture(pTexture);

		return pTexture;
	}

	void TTexture::SelectSamplerId()
	{
		if (m_eAddressU == TEXTURE_ADDRESS_CLAMP && m_eAddressV == TEXTURE_ADDRESS_CLAMP)
		{
			if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT)
				m_SamplerId = 0;
			else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR || m_eFilter == TEXTURE_FILTER_MIN_POINT_MAG_MIP_LINEAR)
				m_SamplerId = 1;
			else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT)
				m_SamplerId = 5;
			else
				TASSERT(TFALSE);
		}
		else if (m_eAddressU == TEXTURE_ADDRESS_WRAP && m_eAddressV == TEXTURE_ADDRESS_WRAP)
		{
			if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT)
				m_SamplerId = 2;
			else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR)
				m_SamplerId = 3;
			else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT)
				m_SamplerId = 6;
			else
				TASSERT(TFALSE);
		}
		else
		{
			if (m_eAddressU == TEXTURE_ADDRESS_CLAMP && m_eAddressV == TEXTURE_ADDRESS_WRAP)
			{
				if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_POINT)
					m_SamplerId = 9;
				else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_MIP_LINEAR)
					m_SamplerId = 10;
				else if (m_eFilter == TEXTURE_FILTER_MIN_MAG_LINEAR_MIP_POINT)
					m_SamplerId = 0xb;

				TASSERT(TFALSE);
			}
			else
			{
				TASSERT(TFALSE);

				m_eAddressU = TEXTURE_ADDRESS_CLAMP;
				m_eAddressV = TEXTURE_ADDRESS_CLAMP;
				m_eFilter = TEXTURE_FILTER_MIN_MAG_MIP_LINEAR;
				m_SamplerId = 3;
			}
		}
	}

	void TTexture::SetWrap(TEXTURE_ADDRESS_MODE eAddressU, TEXTURE_ADDRESS_MODE eAddressV)
	{
		m_eAddressU = eAddressU;
		m_eAddressV = eAddressV;
		SelectSamplerId();
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
		m_pWhiteTexture = TTexture::InitRunTime(TEXTURE_FORMAT_R8G8B8A8_UNORM, WhiteTextureWidth, WhiteTextureHeight, srcData);
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

		m_pInvalidTexture = TTexture::InitRunTime(TEXTURE_FORMAT_R8G8B8A8_UNORM, 8, 8, srcData);
		m_pInvalidTexture->SetName("invalid");
		m_pInvalidTexture->m_pNextTexture = TNULL;
		m_pInvalidTexture->m_pPrevTexture = TNULL;
		TFree(srcData);
	}

	TTexture* TTextureManager::FindTexture(const char* texName)
	{
		TASSERT(T2String8::IsLowerCase(texName));

		TTexture* pCurrentTex = m_pLastTexture;

		while (TTRUE)
		{
			if (pCurrentTex == TNULL)
			{
				return m_pInvalidTexture;
			}

			if (T2String8::CompareNoCase(pCurrentTex->GetName(), texName) == 0)
			{
				break;
			}

			pCurrentTex = pCurrentTex->m_pPrevTexture;
		}

		return pCurrentTex;
	}

	void TTextureManager::AddTexture(TTexture* pTexture)
	{
		pTexture->m_pPrevTexture = GetLastTexture();
		pTexture->m_pNextTexture = TNULL;

		if (pTexture->m_pPrevTexture != TNULL)
		{
			pTexture->m_pPrevTexture->m_pNextTexture = pTexture;
		}

		SetLastTexture(pTexture);
	}

}