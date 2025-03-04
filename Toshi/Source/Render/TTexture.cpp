#include "ToshiPCH.h"
#include "TTexture.h"
#include "TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TTexture );

TTexture::TTexture()
{
	m_eAddress      = 0;
	m_eTextureFlags = 0;
	m_pNameEntry    = TNULL;
}

TTexture::~TTexture()
{
	TTextureFactory* pTexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

	pTexFactory->DeregisterTexture( this );
}

TBOOL TTexture::Lock( LOCKSTATE& a_rLockState )
{
	return TFALSE;
}

void TTexture::Unlock()
{
}

TBOOL TTexture::CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags )
{
	return TResource::Create();
}

TOSHI_NAMESPACE_END
