#include "ToshiPCH.h"
#include "TRender.h"


namespace Toshi
{
	bool TRenderInterface::CreateDisplay()
	{
		TASSERT(TTRUE == IsCreated(), "TRender must be created");
		TASSERT(TFALSE == IsDisplayCreated(), "Display already created");
		m_bDisplayCreated = true;
		return true;
	}

	void TRenderInterface::DumpStats()
	{
		TFile* file = TFile::Create("TRender.txt", TFile::OpenFlags_CreateNew);

		if (file != TNULL)
		{
			file->CPrintf("-\r\n");
			char const* name = GetName();
			uint16_t versionMajor = GetVersionMajor();
			uint16_t versionMinor = GetVersionMinor();
			file->CPrintf("Toshi rendering interface through object[\"%s\"] Version : %u.%u\r\n", name, versionMajor, versionMinor);
			file->CPrintf("-\r\n");

		}

	}
}