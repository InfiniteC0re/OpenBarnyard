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
			m_Flags1 = 0;
            m_eAddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            m_eAddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            m_eFilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            m_SamplerId = 3;
		}

        void Init();
        void Deinit();
		void Bind(UINT startSlot);
		TTexture* InitRunTime(Info* pTextureInfo);
		static TTexture* InitRunTime(DXGI_FORMAT format, UINT width, UINT height, const void* srcData);
		void SelectSamplerId();

		const char* GetName() const       { return m_TexName; }
		uint8_t GetFlags1() const         { return m_Flags1; }
		void* GetHandle() const           { return m_TexInfo->SRView; }
        UINT GetWidth() const             { return m_TexInfo->Width; }
		UINT GetHeight() const            { return m_TexInfo->Height; }
        
        TTexture* GetPrev() const         { return m_pPrevTexture; }
        TTexture* GetNext() const         { return m_pNextTexture; }
		
        void SetName(const char* name)    { m_TexName = name; }
		void SetSamplerId(int iSamplerId) { m_SamplerId = iSamplerId; }
        void SetWrap(D3D11_TEXTURE_ADDRESS_MODE eAddressU, D3D11_TEXTURE_ADDRESS_MODE eAddressV);

		void SetAlphaEnabled(TBOOL bEnabled)
		{
			if (bEnabled)
				m_Flags1 |= 1;
			else
				m_Flags1 &= ~1;
		}

	private:
        void* m_Unk1;                           // 0x00
        const char* m_TexName;                  // 0x04
        Info* m_TexInfo;                        // 0x08
        uint32_t m_DataSize;                    // 0x0C
        uint8_t* m_TexData;                     // 0x10
        TTexture* m_pPrevTexture;               // 0x14
        TTexture* m_pNextTexture;               // 0x18
        uint8_t m_Flags1;                       // 0x1C
        D3D11_TEXTURE_ADDRESS_MODE m_eAddressU; // 0x20
        D3D11_TEXTURE_ADDRESS_MODE m_eAddressV; // 0x24
        D3D11_FILTER m_eFilter;                 // 0x28
		int m_SamplerId;                        // 0x2C
	};

	class TTextureManager : public TSingleton<TTextureManager>
	{
	public:
		friend class TTexture;

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

		TTexture* FindTexture(const char* texName);

		void AddTexture(TTexture* pTexture);

#ifdef TOSHI_DEBUG
	public:
        size_t DEBUG_GetNumTextures() const
        {
            return s_NumTextures;
        }

	private:
		inline static size_t s_NumTextures = 0;
#endif // TOSHI_DEBUG

	private:
		TTexture* m_pLastTexture;
		TTexture* m_pInvalidTexture;
		TTexture* m_pWhiteTexture;
	};
}
