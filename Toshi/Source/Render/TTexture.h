#pragma once
#include "Render/TResource.h"
#include "Render/TRenderAdapter.h"

#include "TTextureFactory.h"

TOSHI_NAMESPACE_START

class T2Texture;

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
	virtual void  CreateFromT2Texture( T2Texture* a_pTexture )															= 0;
	virtual TBOOL Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight ) = 0;
	virtual TBOOL Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags )											= 0;
	virtual TBOOL CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels );

	void SetAddress( TUINT a_eAddress )
	{
		m_eAddress = a_eAddress;
	}

	TUINT GetAddress() const
	{
		return m_eAddress;
	}

	TBOOL CreateResource( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight )
	{
		return TResource::Create();
	}

	TBOOL CreateResource( const TCHAR* a_szFileName, TUINT a_eTextureFlags )
	{
		return TResource::Create();
	}

	TBOOL CreateResource( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels )
	{
		return TResource::Create();
	}

protected:
	TUINT						m_eAddress;
	TUINT						m_eTextureFlags;
	TTextureFactory::NameEntry* m_pNameEntry;
};

TOSHI_NAMESPACE_END
