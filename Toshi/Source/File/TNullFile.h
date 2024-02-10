#pragma once
#include "TFile.h"

namespace Toshi
{
	class TNullFile : TFile
	{
		TINT m_iBytesWritten;
	protected:

		inline void Close() { }
	public:
		inline TINT GetBytesWritten() { return m_iBytesWritten; }

	};
}


