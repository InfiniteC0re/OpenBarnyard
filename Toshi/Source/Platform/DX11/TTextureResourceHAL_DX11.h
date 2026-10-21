#pragma once
#include "Render/TTexture.h"
#include "Toshi/TFreeList.h"

#include <d3d11.h>
#include "Core/TMemoryDebugOff.h"

TOSHI_NAMESPACE_START

class MODAPI TTextureResourceHAL : public TTexture
{
public:
	TDECLARE_CLASS( TTextureResourceHAL, TTexture );
	TDECLARE_FREELIST_ALLOCATOR( TTextureResourceHAL );

	static TTextureResourceHAL* CreateNew()
	{
		return new TTextureResourceHAL();
	}

private:
	TTextureResourceHAL();
	~TTextureResourceHAL();

public:
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;

	virtual TUINT GetWidth() override;
	virtual TUINT GetHeight() override;
	virtual TBOOL Lock( TTexture::LOCKSTATE& a_rLockState ) override;
	virtual void  Unlock() override;
	virtual void  CreateFromT2Texture( T2Texture* a_pTexture ) override;
	virtual TBOOL Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags ) override;
	virtual TBOOL Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight ) override;
	virtual TBOOL CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags ) override;

	TBOOL CreateFromFormat();
	TBOOL CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
	TBOOL CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
	TBOOL CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
	TBOOL CreateFromMemoryDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
	TBOOL CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );

	TBOOL CreateFromFileDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const TCHAR* a_szFile );

	ID3D11ShaderResourceView* GetD3DTexture() { return m_pD3DTexture; }
	void                      SetD3DTexture( ID3D11ShaderResourceView* a_pD3DTexture ) { m_pD3DTexture = a_pD3DTexture; }

	ADDRESSINGMODE GetAddressUState() const { return m_eAddressUState; }
	void           SetAddressUState( ADDRESSINGMODE a_eAddressing ) { m_eAddressUState = a_eAddressing; }

	ADDRESSINGMODE GetAddressVState() const { return m_eAddressVState; }
	void           SetAddressVState( ADDRESSINGMODE a_eAddressing ) { m_eAddressVState = a_eAddressing; }

	D3D11_TEXTURE2D_DESC&       GetImageInfo() { return m_ImageInfo; }
	const D3D11_TEXTURE2D_DESC& GetImageInfo() const { return m_ImageInfo; }

public:
	static TBOOL IsPPM( const TCHAR* a_szName );

	inline static TINT s_iHALMemoryUsage;

protected:
	TINT                      m_iNumLocks;
	TBOOL                     m_bLoadFromMemory;
	TBOOL                     m_bIsToshi2;
	void*                     m_pData;
	TUINT                     m_uiDataSize;
	TUINT                     m_uiWidth;
	TUINT                     m_uiHeight;
	TUINT                     m_uiMipLevels;
	MIPFLAGS                  m_uiMipFlags;
	TTEXTURERESOURCEFORMAT    m_eResourceFormat;
	ID3D11ShaderResourceView* m_pD3DTexture;
	D3D11_TEXTURE2D_DESC      m_ImageInfo;
	ADDRESSINGMODE            m_eAddressUState;
	ADDRESSINGMODE            m_eAddressVState;
	ID3D11Texture2D*          m_pLockTexture;
};

TOSHI_NAMESPACE_END
