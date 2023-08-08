#include "ToshiPCH.h"
#include "PPropertyName.h"

TOSHI_NAMESPACE_USING

void PPropertyName::Split(const TPString8& a_pName)
{
	const TString8& str = a_pName.GetString8();
	
	if (str.Find('[') == -1)
	{
		m_Name = a_pName;
		return;
	}

	TIMPLEMENT();
}
