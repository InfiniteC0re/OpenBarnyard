#pragma once
#include "../TCString.h"
#include "../Typedefs.h"
namespace Toshi
{

	class TFileManager
	{
	private:
		void __thiscall ValidateSystemPath();
	public:
		class TFile* __thiscall CreateFile(class TCString const*, unsigned int);

	};

	class TFile
	{
		int unk;
		void* file;
	private:
		static class Toshi::TFileManager* s_pFileManager;
	public:
		static class Toshi::TFile* __stdcall Create(class TCString const*, unsigned int);
	};

	
}


