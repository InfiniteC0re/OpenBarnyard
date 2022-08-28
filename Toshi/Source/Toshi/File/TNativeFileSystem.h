#pragma once
#include "Toshi/File/TFile.h"
#include "Toshi/File/TNativeFile.h"
//#include "Toshi/File/TFileSystem.h"

namespace Toshi
{
	class TNativeFileSystem : public TFileSystem
	{
	public:
		TNativeFileSystem(const char *);
		virtual TFile* CreateFile(TCString const&, unsigned int);
		virtual void DestroyFile(TFile*);
	};
}


