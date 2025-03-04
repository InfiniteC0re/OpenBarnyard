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
    , m_bHTTPS( TFALSE )
{

}

AHTTPClient::~AHTTPClient()
{
	Destroy();
}

void AHTTPClient::Create( Toshi::T2StringView strUrl )
{
	Destroy();

	// Find base url
	Toshi::T2StringView baseUrl = strUrl;
	if ( baseUrl.FindString( "http://" ) )
	{
		baseUrl += 7;
		m_bHTTPS = TFALSE;
	}
	else if ( baseUrl.FindString( "https://" ) )
	{
		baseUrl += 8;
		m_bHTTPS = TTRUE;
	}

	m_strBaseUrl = baseUrl.Get();
	TINT iPathPos = m_strBaseUrl.Find( '/' );

	if ( iPathPos != -1 )
	{
		m_strPath = m_strBaseUrl.GetString( iPathPos );
		m_strBaseUrl.Truncate( iPathPos );
	}
	else
		m_strPath = "/";

	if ( m_bHTTPS )
		m_pSSLClient = new httplib::SSLClient( m_strBaseUrl.GetString() );
	else
		m_pClient = new httplib::Client( m_strBaseUrl.GetString() );
}

void AHTTPClient::Destroy()
{
	if ( m_pClient && !m_bHTTPS )
	{
		delete m_pClient;
		m_pClient = TNULL;
	}
	
	if ( m_pSSLClient && m_bHTTPS )
	{
		delete m_pSSLClient;
		m_pSSLClient = TNULL;
	}
}

void AHTTPClient::SetHeaders( httplib::Headers& headers )
{
	TASSERT( m_pClient || m_pSSLClient );

	if ( m_pClient && !m_bHTTPS )
		m_pClient->set_default_headers( std::move( headers ) );
	else if ( m_pSSLClient && m_bHTTPS )
		m_pSSLClient->set_default_headers( std::move( headers ) );
}

httplib::Result AHTTPClient::Get()
{
	TASSERT( m_pClient || m_pSSLClient );

	if ( m_pClient && !m_bHTTPS )
		return m_pClient->Get( m_strPath.GetString() );
	
	if ( m_pSSLClient && m_bHTTPS )
		return m_pSSLClient->Get( m_strPath.GetString() );

	return httplib::Result{};
}

MODCORE_API AHTTPClient* CreateHTTPClient()
{
	return new AHTTPClient();
}
