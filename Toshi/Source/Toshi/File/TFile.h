#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{
	class TFileManager
	{
	public:
		TFileManager() = default;
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

		enum TSEEK
		{
			TSEEK_SET,
			TSEEK_CUR,
			TSEEK_END
		};

		static TFile* __stdcall Create(class TCString const*, unsigned int);

	private:
		int unk;
		void* file;
	};
}


