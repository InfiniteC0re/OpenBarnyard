#pragma once
#include "../TCString.h"
#include "../Typedefs.h"

namespace Toshi
{
	class TFileManager
	{
	public:
		TFileManager() = default;
		// delete copy constructor since this has to be a singleton class
		TFileManager(const TFileManager&) = delete;
		
		inline static TFileManager& Instance()
		{
			static TFileManager instance;
			return instance;
		};

	private:
		void ValidateSystemPath();

	public:
		class TFile* CreateFile(class TCString const*, unsigned int);
	};

	class TFile
	{
	public:
		static TFile* __stdcall Create(class TCString const*, unsigned int);

	private:
		int unk;
		void* file;
	};
}


