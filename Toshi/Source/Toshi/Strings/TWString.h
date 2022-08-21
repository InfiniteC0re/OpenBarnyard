#pragma once
#include "TString.h"

namespace Toshi
{
	class TWString
	{
	public:
		inline uint32_t Length() const { return m_iStrLen; }
		
	public:
		inline bool IsIndexValid(int index) const { return index <= m_iStrLen && index >= 0; }
		inline const wchar_t* GetString(int index = 0) const { if (!IsIndexValid(index)) { return 0; } return m_pBuffer + index * 2; }
		inline operator wchar_t* () const { return m_pBuffer; }

	private:
		wchar_t* m_pBuffer = (wchar_t*)NullString;
		uint8_t m_iExcessLen : 8 = 0;
		uint32_t m_iStrLen : 24 = 0;
	};
}


