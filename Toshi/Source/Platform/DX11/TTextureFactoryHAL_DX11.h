#pragma once
#include "Render/TTextureFactory.h"

TOSHI_NAMESPACE_START

class MODAPI TTextureFactoryHAL : public TTextureFactory
{
public:
	TDECLARE_CLASS( TTextureFactoryHAL, TTextureFactory );

public:
	~TTextureFactoryHAL();

public:
	TTexture* CreateTextureFromFile( const TCHAR* a_szFilePath, TUINT32 a_eTextureFlags ) override;
	TTexture* CreateTextureFromMemory( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight ) override;
	TTexture* CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags ) override;
	TTexture* CreateFromT2Texture( T2Texture* a_pTexture ) override;

private:
};

TOSHI_NAMESPACE_END
