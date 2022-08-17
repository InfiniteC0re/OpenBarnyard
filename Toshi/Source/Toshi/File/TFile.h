#pragma once
#include "Toshi/Core/TSingleton.h"
#include "Toshi/Strings/TCString.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{
	class TFileManager : public TSingleton<TFileManager>
	{
	public:
		TFileManager() = default;
		TFileManager(const TFileManager&) = delete;

	private:
		void ValidateSystemPath();

	public:
		class TFile* CreateFile(class TCString const*, unsigned int);
	};

	class TFile
	{
	public:
		static TFile* Create(class TCString const*, unsigned int);

	private:
		int unk;
		void* file;
	};
}


