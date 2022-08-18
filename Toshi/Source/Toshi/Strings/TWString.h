#pragma once
#include "TString.h"

namespace Toshi
{
	class TWString
	{
	public:
		inline uint32_t Length() const { return m_uiLength & 0x00FFFFFF; }
		
	public:
		inline operator wchar_t* () const { return m_pBuffer; }

	private:
		wchar_t* m_pBuffer = (wchar_t*)NullString;
		uint32_t m_uiLength;
	};
}


