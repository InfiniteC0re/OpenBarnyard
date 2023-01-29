#pragma once
#include <d3d11.h>

namespace Toshi
{
	class TTexture
	{
	public:
		friend class TTextureManager;

		struct Info
		{
			UINT Width;
			UINT Height;
			UINT MipMapCount;
			UINT16 Width2;
			UINT16 Height2;
			DXGI_FORMAT Format;
			ID3D11ShaderResourceView* SRView;

			Info()
			{
				Width = 0;
				Height = 0;
				MipMapCount = 0;
				Width2 = 0;
				Height2 = 0;
				Format = DXGI_FORMAT_UNKNOWN;
				SRView = TNULL;
			}
		};

	public:
		TTexture()
		{
			m_Unk1 = TNULL;
			m_TexInfo = TNULL;
			m_TexName = TNULL;
			m_DataSize = 0;
			m_TexData = 0;
			m_pPrevTexture = TNULL;
			m_pNextTexture = TNULL;
		}

		void Init();

		TTexture* InitRunTime(Info* pTextureInfo);
		static TTexture* InitRunTime(DXGI_FORMAT format, UINT width, UINT height, const void* srcData);

		void SetName(const char* name)
		{
			m_TexName = name;
		}

		const char* GetName() const
		{
			return m_TexName;
		}

	private:
		void* m_Unk1;             // 0x00
		const char* m_TexName;    // 0x04
		Info* m_TexInfo;          // 0x08
		uint32_t m_DataSize;      // 0x0C
		uint8_t* m_TexData;       // 0x10
		TTexture* m_pPrevTexture; // 0x14
		TTexture* m_pNextTexture; // 0x18
	};

	class TTextureManager : public TSingleton<TTextureManager>
	{
	public:
		TTextureManager();
		~TTextureManager() = default;

		void SetLastTexture(TTexture* pTexture)
		{
			m_pLastTexture = pTexture;
		}

		TTexture* GetLastTexture() const
		{
			return m_pLastTexture;
		}

		TTexture* GetInvalidTexture() const
		{
			return m_pInvalidTexture;
		}

		TTexture* GetWhiteTexture() const
		{
			return m_pWhiteTexture;
		}

		void AddTexture(TTexture* pTexture)
		{	
			pTexture->m_pPrevTexture = GetLastTexture();
			pTexture->m_pNextTexture = TNULL;

			if (pTexture->m_pPrevTexture != TNULL)
			{
				pTexture->m_pPrevTexture->m_pNextTexture = pTexture;
			}

			SetLastTexture(pTexture);
		}

	private:
		TTexture* m_pLastTexture;
		TTexture* m_pInvalidTexture;
		TTexture* m_pWhiteTexture;
	};
}
