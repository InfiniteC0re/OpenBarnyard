#include "ToshiPCH.h"
#include "TTextureFactoryHAL_DX8.h"
#include "TTextureResourceHAL_DX8.h"
#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TTextureFactoryHAL );

// $Barnyard: FUNCTION 006d7bd0
TTexture* TTextureFactoryHAL::CreateTextureFromFile( const TCHAR* a_szFilePath, TUINT32 a_eTextureFlags )
{
	TTexture* pTexture = FindTexture( a_szFilePath );

	if ( !pTexture )
	{
		pTexture = TSTATICCAST(
		    TTexture,
		    GetRenderer()->CreateResource(
		        TClass::Find( "TTextureResourceHAL" ),
		        TNULL,
		        this
		    )
		);

		pTexture->Create( a_szFilePath, a_eTextureFlags );
		RegisterTexture( a_szFilePath, pTexture );
		pTexture->Validate();
	}

	return pTexture;
}

// $Barnyard: FUNCTION 006d7c30
TTexture* TTextureFactoryHAL::CreateTextureFromMemory( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight )
{
	static TUINT s_iNumMemTextures = 0;
	static TCHAR s_szName[ 32 ];

	s_szName[ 0 ] = 'm';
	s_szName[ 1 ] = 't';
	s_szName[ 2 ] = 'e';
	s_szName[ 3 ] = 'x';
	s_szName[ 4 ] = ':';
	T2String8::IntToString( ++s_iNumMemTextures, s_szName + 5, 16 );

	TTexture* pTexture = TSTATICCAST(
	    TTexture,
	    GetRenderer()->CreateResource(
	        TClass::Find( "TTextureResourceHAL" ),
	        TNULL,
	        this
	    )
	);

	pTexture->Create( a_pData, a_uiDataSize, a_eTextureFlags, a_uiWidth, a_uiHeight );
	RegisterTexture( s_szName, pTexture );

	return pTexture;
}

// $Barnyard: FUNCTION 006d7cd0
TTexture* TTextureFactoryHAL::CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags )
{
	static TUINT s_iNumMemTextures = 0;
	static TCHAR s_szName[ 32 ];

	s_szName[ 0 ] = 'm';
	s_szName[ 1 ] = 't';
	s_szName[ 2 ] = 'e';
	s_szName[ 3 ] = 'x';
	s_szName[ 4 ] = ':';
	T2String8::IntToString( ++s_iNumMemTextures, s_szName + 5, 16 );

	TTexture* pTexture = TSTATICCAST(
	    TTexture,
	    GetRenderer()->CreateResource(
	        TClass::Find( "TTextureResourceHAL" ),
	        TNULL,
	        this
	    )
	);

	pTexture->CreateEx( a_pData, a_uiDataSize, a_uiWidth, a_uiHeight, a_uiMipLevels, a_eFormat, a_uiMipMapFlags );
	RegisterTexture( s_szName, pTexture );

	return pTexture;
}

// $Barnyard: FUNCTION 006d7d80
TTexture* TTextureFactoryHAL::CreateFromT2Texture( T2Texture* a_pTexture )
{
	static TUINT s_iNumMemTextures = 0;
	static TCHAR s_szName[ 32 ];

	s_szName[ 0 ] = 't';
	s_szName[ 1 ] = '2';
	s_szName[ 2 ] = 't';
	s_szName[ 3 ] = 'x';
	s_szName[ 4 ] = ':';
	T2String8::IntToString( ++s_iNumMemTextures, s_szName + 5, 16 );

	TTexture* pTexture = TSTATICCAST(
	    TTexture,
	    GetRenderer()->CreateResource(
	        TClass::Find( "TTextureResourceHAL" ),
	        TNULL,
	        this
	    )
	);

	pTexture->CreateFromT2Texture( a_pTexture );
	RegisterTexture( s_szName, pTexture );

	return pTexture;
}

TOSHI_NAMESPACE_END
