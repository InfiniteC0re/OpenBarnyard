#pragma once
#include "Toshi/File/TFile.h"
#include "Toshi/File/TNativeFile.h"

namespace Toshi
{
	class TNativeFileSystem
	{
	public:
		virtual TFile* CreateFile(TCString const&, unsigned int);
	};
}


