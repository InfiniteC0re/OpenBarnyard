#include "ToshiPCH.h"
#include "TRender.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	bool TRender::Create()
	{
		TASSERT(TFALSE == IsCreated(), "TRender already created");
		TRenderContext* a_pRenderContext = CreateRenderContext();
		if (a_pRenderContext != TNULL)
		{
			SetCurrentRenderContext(a_pRenderContext);
			m_bCreated = true;
			return true;
		}
		return false;
	}

	bool TRender::CreateDisplay()
	{
		TASSERT(TTRUE == IsCreated(), "TRender must be created");
		TASSERT(TFALSE == IsDisplayCreated(), "Display already created");
		m_bDisplayCreated = true;
		return true;
	}

	void TRender::DumpStats()
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