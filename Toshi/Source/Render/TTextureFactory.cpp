#include "ToshiPCH.h"
#include "TTextureFactory.h"
#include "TTexture.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TDEFINE_CLASS_NORUNTIME( Toshi::TTextureFactory );

TOSHI_NAMESPACE_START

TTextureFactory::NameEntry::NameEntry( const TCHAR* a_szName, TTexture* a_pTexture )
{
	auto uiNameLen = TStringManager::String8Length( a_szName );
	TASSERT( uiNameLen < MAX_TEXTURENAMELEN );

	TStringManager::String8Copy( m_szName, a_szName, MAX_TEXTURENAMELEN );
	m_pTexture = a_pTexture;
}

TTextureFactory::NameEntry::~NameEntry(){
	TASSERT( TFALSE == IsLinked() )
}

TTextureFactory::~TTextureFactory()
{
	for ( TUINT32 i = 0; i < NUM_LISTS; i++ )
	{
		m_aLists[ i ].DeleteAll();
	}
}

void TTextureFactory::Dump() const
{
	TUINT uiTexIndex = 0;

	for ( TUINT32 i = 0; i < NUM_LISTS; i++ )
	{
		for ( auto it = m_aLists[ i ].Begin(); it != m_aLists[ i ].End(); it++ )
		{
			TTRACE( "[%d] Texture [%s]: W=%d , H=%d\n", uiTexIndex++, it->GetName(), it->GetTexture()->GetWidth(), it->GetTexture()->GetHeight() );
		}
	}
}

TNodeList<TTextureFactory::NameEntry>* TTextureFactory::GetList( TUINT a_uiList )
{
	TASSERT( a_uiList < NUM_LISTS );
	return &m_aLists[ a_uiList ];
}

TTexture* TTextureFactory::FindTexture( const TCHAR* a_szName ) const
{
	auto pList = &m_aLists[ HashName( a_szName ) ];

	for ( auto it = pList->Begin(); it != pList->End(); it++ )
	{
		if ( 0 == TStringManager::String8CompareNoCase( it->GetName(), a_szName ) )
		{
			return it->GetTexture();
		}
	}

	return TNULL;
}

TTextureFactory::NameEntry* TTextureFactory::RegisterTexture( const TCHAR* a_szName, TTexture* a_pTexture )
{
	auto pList	= &m_aLists[ HashName( a_szName ) ];
	auto pEntry = new TTextureFactory::NameEntry( a_szName, a_pTexture );

	a_pTexture->m_pNameEntry = pEntry;
	pList->InsertHead( pEntry );

	return a_pTexture->m_pNameEntry;
}

void TTextureFactory::DeregisterTexture( TTexture* a_pTexture )
{
	TVALIDPTR( a_pTexture );

	auto pNameEntry = a_pTexture->m_pNameEntry;

	if ( pNameEntry )
	{
		pNameEntry->Remove();
		delete pNameEntry;
	}
}

TUINT32 TTextureFactory::HashName( const TCHAR* a_szName )
{
	TUINT32 iHash = 0;

	for ( TSIZE i = 0; i < TStringManager::String8Length( a_szName ); i++ )
	{
		iHash += a_szName[ i ];
	}

	return iHash % NUM_LISTS;
}

TOSHI_NAMESPACE_END
