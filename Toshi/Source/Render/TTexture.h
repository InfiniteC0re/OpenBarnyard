/**
 * @file TTexture.h
 * Texture system for the Toshi engine
 * 
 * This file defines the texture system that provides:
 * - Texture loading and management
 * - Texture format handling
 * - Texture state management
 * - Platform-specific texture abstraction
 */

#pragma once
#include "Render/TResource.h"
#include "Render/TRenderAdapter.h"
#include "Render/TRender.h"

#include "TTextureFactory.h"

TOSHI_NAMESPACE_START

class T2Texture;

enum TTEXTURE_FORMAT
{
	// Windows
	TTEX_FMT_WIN_BASE = 0,
	TTEX_FMT_WIN_DDS,

	// GameCube / Wii
	TTEX_FMT_REV_BASE = 0x300,
	TTEX_FMT_REV_I4,
	TTEX_FMT_REV_IA4,
	TTEX_FMT_REV_I8,
	TTEX_FMT_REV_IA8,
	TTEX_FMT_REV_RGB565,
	TTEX_FMT_REV_RGB5A3,
	TTEX_FMT_REV_RGBA8,
	TTEX_FMT_REV_CMPR,
	TTEX_FMT_REV_Z8,
	TTEX_FMT_REV_Z16,
	TTEX_FMT_REV_Z24X8,
	TTEX_FMT_REV_CI4_RGB565,
	TTEX_FMT_REV_CI4_RGB5A3,
	TTEX_FMT_REV_CI8_RGB565,
	TTEX_FMT_REV_CI8_RGB5A3,
	TTEX_FMT_REV_CI8_IA8,
};

class TTexture : public TResource
{
public:
	TDECLARE_CLASS( TTexture, TResource );

public:
	struct LOCKSTATE
	{
		TINT  Pitch;
		void* pBits;
	};

	friend TTextureFactory;

public:
	TTexture();
	~TTexture();

	virtual TUINT GetWidth()  = 0;
	virtual TUINT GetHeight() = 0;
	virtual TBOOL Lock( LOCKSTATE& a_rLockState );
	virtual void  Unlock();
	virtual void  CreateFromT2Texture( T2Texture* a_pTexture )                                                          = 0;
	virtual TBOOL Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight ) = 0;
	virtual TBOOL Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags )                                            = 0;
	virtual TBOOL CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags );

	void SetAddressing( ADDRESSINGMODE a_eAddressing )
	{
		m_eAddressing = a_eAddressing;
	}

	ADDRESSINGMODE GetAddressing() const
	{
		return m_eAddressing;
	}

	TBOOL CreateResource( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight )
	{
		return TResource::Create();
	}

	TBOOL CreateResource( const TCHAR* a_szFileName, TUINT a_eTextureFlags )
	{
		return TResource::Create();
	}

	TBOOL CreateResource( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiFlags )
	{
		return TResource::Create();
	}

protected:
	ADDRESSINGMODE              m_eAddressing;
	TUINT                       m_eTextureFlags;
	TTextureFactory::NameEntry* m_pNameEntry;
};

TOSHI_NAMESPACE_END
