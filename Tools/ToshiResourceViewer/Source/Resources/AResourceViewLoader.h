#pragma once
#include "AResourceView.h"

#include <Toshi/Utils/TSingleton.h>
#include <Toshi2/T2Iterator.h>

class AResourceViewLoader : public Toshi::TSingleton<AResourceViewLoader>
{
public:
	AResourceViewLoader()
	{

	}

	~AResourceViewLoader()
	{
		m_Resources.DeleteAll();
	}

	Toshi::T2DList<AResourceView>::Iterator Begin()
	{
		return m_Resources.Begin();
	}

	const Toshi::T2DList<AResourceView>::Node* End() const
	{
		return m_Resources.End();
	}

	TBOOL CreateFile(const char* a_szFileName, AResourceFilePtr* a_pResourceFile = TNULL);

private:
	Toshi::T2DList<AResourceView> m_Resources;
};
