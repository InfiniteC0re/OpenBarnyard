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
		virtual TFile* CreateFile(const TString8& a_rcFileName, TFILEMODE a_eFileMode) override;
		virtual void DestroyFile(TFile* a_pFile) override;
		virtual TString8 MakeInternalPath(const TString8& a_rcPath) override;
		virtual TBOOL MakeDirectory(const TString8& a_rcDirectory) override;
	};

}