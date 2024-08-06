#pragma once
#include "TFile.h"

namespace Toshi {

	class TNullFileSystem :
		public TFileSystem,
		public TSingleton<TNullFileSystem>
	{
	public:
		TNullFileSystem(const TCHAR* name) : TFileSystem(name)
		{
			TFileManager::GetSingletonSafe()->MountFileSystem(this);
		}

		//-----------------------------------------------------------------------------
		// TFileSystem
		//-----------------------------------------------------------------------------
		virtual TFile* CreateFile(const TString8& a_rcFileName, TFILEMODE a_eFileMode) OVERRIDE;
		virtual void DestroyFile(TFile* a_pFile) OVERRIDE;
		virtual TString8 MakeInternalPath(const TString8& a_rcPath) OVERRIDE;
	};

}