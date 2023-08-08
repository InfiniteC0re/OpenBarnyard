#include "ToshiPCH.h"
#include "TPCString.h"

int Toshi::TPCString::Compare(const TPCString& other) const
{
	if (!m_pPooledString && other) return -1;
	if (!other) return 1;
	if (m_pPooledString != other)
	{
		return m_pPooledString->m_oString.Compare(other.m_pPooledString->m_oString, -1);
	}
	return 0;
}
