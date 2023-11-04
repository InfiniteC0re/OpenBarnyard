#pragma once
#include "Toshi/Render/TTexture.h"

#include <d3dx8.h>

namespace Toshi {

	class TTextureResourceHAL : public TTexture
	{
	public:
		virtual TBOOL Validate() override;
		virtual void Invalidate() override;

		virtual TUINT GetWidth() override;
		virtual TUINT GetHeight() override;
		virtual TBOOL Lock(TTexture::LOCKSTATE& a_rLockState) override;
		virtual void Unlock() override;
		virtual void CreateShared(void* a_pUnknown) override;
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

		static TBOOL IsPPM(const char* a_szName);

	private:
		TINT m_iNumLocks;
		TBOOL m_bLoadFromMemory;
		TBOOL m_bIsShared;
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