#pragma once
#include <Toshi/File/TTRB.h>

class AGUI2FontResource
{
public:
	const char* m_szFileName;
	Toshi::TTRB m_TRB;
	TUINT m_iRefCount;
};
