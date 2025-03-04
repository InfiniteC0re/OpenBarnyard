#pragma once
#include "httplib.h"
#include "ModLoader.h"

#include <Toshi/T2String.h>
#include <Toshi/TString8.h>

class AHTTPClient
{
public:
	AHTTPClient();
	virtual ~AHTTPClient();

	virtual void            Create( Toshi::T2ConstString8 strUrl );
	virtual void            SetHeaders( httplib::Headers& headers );
	virtual httplib::Result Get();

private:
	httplib::Client* m_pClient;
	Toshi::TString8  m_strBaseUrl;
	Toshi::TString8  m_strPath;
};

MODCORE_API AHTTPClient* CreateHTTPClient();
