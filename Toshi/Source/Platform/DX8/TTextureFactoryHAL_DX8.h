#pragma once
#include "Toshi/Render/TTextureFactory.h"

namespace Toshi {

	class TTextureFactoryHAL :
		public TGenericClassDerived<TTextureFactoryHAL, TTextureFactory, "TTextureFactoryHAL", TMAKEVERSION(1, 0), TTRUE>
	{
	public:
		TTexture* CreateTextureFromFile(const char* a_szFilePath, TUINT32 a_eTextureFlags) override;
		TTexture* CreateTextureFromMemory(void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight) override;
		TTexture* CreateEx(void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels) override;
		TTexture* CreateFromT2Texture(void* a_pUnk) override;

	private:

	};

}