#include "ToshiPCH.h"
#include "TTexture.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

TDEFINE_CLASS_NORUNTIME( TTexture );

TTexture::TTexture()
{
    m_eAddress      = 0;
    m_eTextureFlags = 0;
    m_pNameEntry    = TNULL;
}

TTexture::~TTexture()
{
}

TBOOL TTexture::Lock( LOCKSTATE& a_rLockState )
{
    return TFALSE;
}

void TTexture::Unlock()
{
}

TBOOL TTexture::CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, BOOL a_bNoMipLevels )
{
    return TResource::Create();
}

} // namespace Toshi