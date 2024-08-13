#pragma once
#include "TFile.h"

TOSHI_NAMESPACE_START

class TNullFile : TFile
{
	TINT m_iBytesWritten;

protected:
	inline void Close() {}

public:
	inline TINT GetBytesWritten() { return m_iBytesWritten; }
};

TOSHI_NAMESPACE_END
