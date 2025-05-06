#pragma once

#if 0

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "ModLoader.h"

#include <Toshi/T2String.h>
#include <Toshi/TString8.h>

class AHTTPClient
{
public:
	AHTTPClient();
	virtual ~AHTTPClient();

	virtual void            Create( Toshi::T2StringView strUrl );
	virtual void            Destroy();
	virtual void            SetHeaders( httplib::Headers& headers );
	virtual httplib::Result Get();

private:
	union 
	{
		httplib::Client* m_pClient;
		httplib::SSLClient* m_pSSLClient;
	};

	Toshi::TString8      m_strBaseUrl;
	Toshi::TString8      m_strPath;
	TBOOL                m_bHTTPS;
};

MODCORE_API AHTTPClient* CreateHTTPClient();

#endif 