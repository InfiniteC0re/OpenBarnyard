#pragma once
#include "TFile.h"

namespace Toshi
{
	class TNullFile : TFile
	{
		int m_iBytesWritten;
	protected:

		inline void Close() { }
	public:
		inline int GetBytesWritten() { return m_iBytesWritten; }

	};
}


