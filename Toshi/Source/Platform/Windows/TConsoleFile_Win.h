#pragma once
#include "File/TFile.h"

namespace Toshi {

	//-----------------------------------------------------------------------------
	// Use this to redirect log information to the console.
	//-----------------------------------------------------------------------------
	class TConsoleFileSystem : public TFileSystem
	{
	public:
		TConsoleFileSystem(const char* a_szName);

		virtual TFile* CreateFile(TString8 const& fn, uint32_t flags) override;
		virtual void DestroyFile(TFile* a_pFile) override;
		virtual TString8 MakeInternalPath(TString8 const&);
		virtual TBOOL MakeDirectory(TString8 const&) override;
		virtual TBOOL GetNextFile(TString8& fileName, uint32_t flags);
	};

}