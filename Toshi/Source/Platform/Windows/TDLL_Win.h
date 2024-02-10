#pragma once
#include "Toshi/TString8.h"

namespace Toshi {

	class TDLL
	{
	public:
		TDLL();
		TDLL(const TDLL& other);

		virtual ~TDLL();

		TBOOL Load(const TString8& a_sFileName);
		void Unload();

		void* GetAddress(const TString8& a_sSymbolName);

		const TString8& GetFileName() const
		{
			return m_sFileName;
		}

		HMODULE GetDLL() const
		{
			return m_pDLL;
		}

		TDLL& operator=(const TDLL& other)
		{
			m_sFileName = other.m_sFileName;
			m_pDLL = other.m_pDLL;
		}

	private:
		TString8 m_sFileName;
		HMODULE m_pDLL;
	};
}