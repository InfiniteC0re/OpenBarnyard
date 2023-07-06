#include "ToshiPCH.h"
#include "TFileLexerUTF8.h"

namespace Toshi {

	TFileLexerUTF8::TFileLexerUTF8()
	{
		TIMPLEMENT();

#ifdef TOSHI_PLATFORM_WINDOWS
		Define("TOSHI_SKU_WINDOWS");
#endif
	}

	TFileLexerUTF8::~TFileLexerUTF8()
	{
		TIMPLEMENT();
	}

	void TFileLexerUTF8::Define(const char* a_szDefinition)
	{
		TIMPLEMENT();
	}

}
