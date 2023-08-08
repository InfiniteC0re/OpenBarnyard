#include "ToshiPCH.h"
#include "PProperty.h"

void PPropertyName::Split(const Toshi::TPCString& a_pName)
{
	Toshi::TString8 str = a_pName.GetPtr() ? a_pName.GetPtr()->m_oString : Toshi::TPCString::ms_sEmpty;
	
	
	if (str.Find('[') == -1)
	{
		m_pName = a_pName;
		return;
	}
	TIMPLEMENT();
}
