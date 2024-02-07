#pragma once
#include "Render/TTexture.h"
#include "Toshi/TFreeList.h"

#include <d3dx8.h>
#include "Core/TMemoryDebugOff.h"

namespace Toshi {

	class TTextureResourceHAL : public TTexture
	{
	public:
		TDECLARE_CLASS(TTexture);
		TDECLARE_FREELIST_ALLOCATOR(TTextureResourceHAL);

	public:
		TTextureResourceHAL()
		{
			m_iNumLocks = 0;
			m_bLoadFromMemory = TFALSE;
			m_bIsToshi2 = TFALSE;
			m_pData = TNULL;
			m_uiDataSize = 0;
			m_pD3DTexture = TNULL;
			m_iUnk1 = 0;
			m_iUnk2 = 0;
		}

		virtual TBOOL Validate() override;
		virtual void Invalidate() override;

		virtual TUINT GetWidth() override;
		virtual TUINT GetHeight() override;
		virtual TBOOL Lock(TTexture::LOCKSTATE& a_rLockState) override;
		virtual void Unlock() override;
		virtual void CreateFromT2Texture(T2Texture* a_pTexture) override;
		virtual TBOOL Create(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight) override;
		virtual TBOOL Create(const char* a_szFileName, TUINT a_eTextureFlags) override;
		virtual TBOOL CreateEx(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels) override;

		TBOOL CreateFromFormat();
		TBOOL CreateFromMemory8888(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData);
		TBOOL CreateFromMemory888(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData);
		TBOOL CreateFromMemory5551(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData);
		TBOOL CreateFromMemoryDDS(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData);
		TBOOL CreateFromMemory4444(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData);

		TBOOL CreateFromFileDDS(TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const char* a_szFile);

		void SetD3DTexture(IDirect3DTexture8* a_pD3DTexture)
		{
			m_pD3DTexture = a_pD3DTexture;
		}

		IDirect3DTexture8* GetD3DTexture()
		{
			return m_pD3DTexture;
		}

	public:
		static TBOOL IsPPM(const char* a_szName);

	private:
		TINT m_iNumLocks;
		TBOOL m_bLoadFromMemory;
		TBOOL m_bIsToshi2;
		void* m_pData;
		TUINT m_uiDataSize;
		TUINT m_uiWidth;
		TUINT m_uiHeight;
		TUINT m_uiMipLevels;
		BOOL m_bNoMipLevels;
		TTEXTURERESOURCEFORMAT m_eResourceFormat;
		IDirect3DTexture8* m_pD3DTexture;
		D3DXIMAGE_INFO m_ImageInfo;
		int m_iUnk1;
		int m_iUnk2;
	};

}