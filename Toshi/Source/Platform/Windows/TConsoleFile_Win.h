#pragma once
#include "File/TFile.h"

namespace Toshi {

	//-----------------------------------------------------------------------------
	// Use this to redirect log information to the console.
	//-----------------------------------------------------------------------------
	class TConsoleFileSystem : public TFileSystem
	{
	public:
		TConsoleFileSystem(const TCHAR* a_szName);

		//-----------------------------------------------------------------------------
		// TFileSystem
		//-----------------------------------------------------------------------------
		virtual TFile* CreateFile(const TString8& a_rcFileName, TFILEMODE a_eFileMode) OVERRIDE;
		virtual void DestroyFile(TFile* a_pFile) OVERRIDE;
		virtual TString8 MakeInternalPath(const TString8& a_rcPath) OVERRIDE;
	};

}