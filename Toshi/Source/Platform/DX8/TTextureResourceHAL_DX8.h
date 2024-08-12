#pragma once
#include "Render/TTexture.h"
#include "Toshi/TFreeList.h"

#include <d3dx8.h>
#include "Core/TMemoryDebugOff.h"

namespace Toshi
{

class TTextureResourceHAL : public TTexture
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

public:
    virtual TBOOL Validate() override;
    virtual void  Invalidate() override;

    virtual TUINT GetWidth() override;
    virtual TUINT GetHeight() override;
    virtual TBOOL Lock( TTexture::LOCKSTATE& a_rLockState ) override;
    virtual void  Unlock() override;
    virtual void  CreateFromT2Texture( T2Texture* a_pTexture ) override;
    virtual TBOOL Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight ) override;
    virtual TBOOL Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags ) override;
    virtual TBOOL CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels ) override;

    TBOOL CreateFromFormat();
    TBOOL CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
    TBOOL CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
    TBOOL CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
    TBOOL CreateFromMemoryDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );
    TBOOL CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData );

    TBOOL CreateFromFileDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const TCHAR* a_szFile );

    IDirect3DTexture8* GetD3DTexture() { return m_pD3DTexture; }
    void               SetD3DTexture( IDirect3DTexture8* a_pD3DTexture ) { m_pD3DTexture = a_pD3DTexture; }

    TINT GetAddressUState() const { return m_eAddressUState; }
    void SetAddressUState( TINT val ) { m_eAddressUState = val; }

    TINT GetAddressVState() const { return m_eAddressVState; }
    void SetAddressVState( TINT val ) { m_eAddressVState = val; }

    D3DXIMAGE_INFO&       GetImageInfo() { return m_ImageInfo; }
    const D3DXIMAGE_INFO& GetImageInfo() const { return m_ImageInfo; }

public:
    static TBOOL IsPPM( const TCHAR* a_szName );

private:
    TINT                   m_iNumLocks;
    TBOOL                  m_bLoadFromMemory;
    TBOOL                  m_bIsToshi2;
    void*                  m_pData;
    TUINT                  m_uiDataSize;
    TUINT                  m_uiWidth;
    TUINT                  m_uiHeight;
    TUINT                  m_uiMipLevels;
    BOOL                   m_bNoMipLevels;
    TTEXTURERESOURCEFORMAT m_eResourceFormat;
    IDirect3DTexture8*     m_pD3DTexture;
    D3DXIMAGE_INFO         m_ImageInfo;
    TINT                   m_eAddressUState;
    TINT                   m_eAddressVState;
};

} // namespace Toshi