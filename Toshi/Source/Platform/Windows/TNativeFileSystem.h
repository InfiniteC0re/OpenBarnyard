#pragma once
#include "Platform/Windows/TNativeFile.h"

namespace Toshi
{
	class TNativeFileSystem : public TFileSystem
	{
	public:
		TNativeFileSystem(const char *);

		virtual TFile* CreateFile(TCString const& fn, uint32_t flags) override;
		virtual void DestroyFile(TFile*) override;
		virtual TCString MakeInternalPath(TCString const&) { return {}; }
		virtual void SetPrefix(TCString const&) { };
		virtual bool MakeDirectory(TCString const&) override;
	};
}


