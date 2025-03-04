#include "pch.h"
#include "AHTTPClient.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AHTTPClient::AHTTPClient()
    : m_pClient( TNULL )
{

}

AHTTPClient::~AHTTPClient()
{
	if ( m_pClient )
		delete m_pClient;
}

void AHTTPClient::Create( Toshi::T2ConstString8 strUrl )
{
	if ( m_pClient )
		delete m_pClient;

	// Find base url
	Toshi::T2ConstString8 baseUrl = strUrl;
	if ( baseUrl.FindString( "http://" ) ) baseUrl += 7;
	else if ( baseUrl.FindString( "https://" ) ) baseUrl += 8;

	m_strBaseUrl = baseUrl.Get();
	TINT iPathPos = m_strBaseUrl.Find( '/' );

	if ( iPathPos != -1 )
	{
		m_strPath = m_strBaseUrl.GetString( iPathPos );
		m_strBaseUrl.Truncate( iPathPos );
	}
	else
		m_strPath = "/";

	m_pClient = new httplib::Client( m_strBaseUrl.GetString() );
}

void AHTTPClient::SetHeaders( httplib::Headers& headers )
{
	TASSERT( m_pClient );
	m_pClient->set_default_headers( std::move( headers ) );
}

httplib::Result AHTTPClient::Get()
{
	return m_pClient->Get( m_strPath.GetString() );
}

MODCORE_API AHTTPClient* CreateHTTPClient()
{
	return new AHTTPClient();
}
