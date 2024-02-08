#include "ToshiPCH.h"
#include "TConsoleFile_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	class TConsoleFile : public TFile
	{
	public:
		TConsoleFile(TFileSystem* a_pFileSystem) : TFile(a_pFileSystem)
		{

		}

		virtual size_t Read(void* dst, size_t size) override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual size_t Write(const void* buffer, size_t size) override
		{
			TASSERT(!"Use Printf or VCPrintf");
			return 0;
		}
		
		virtual TBOOL Seek(int offset, TFile::TSEEK seek) override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual uint32_t Tell() override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual DWORD GetSize() override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual _FILETIME GetDate() override
		{
			TASSERT(!"Console doesn't support this");
			return {};
		}
		
		virtual int GetCChar() override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual wchar_t GetWChar() override
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual int PutCChar(char character)
		{
			putchar(character);
			return 0;
		}
		
		virtual int PutWChar(wchar_t character)
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}

		virtual int CPrintf(const char* format, ...)
		{
			va_list args;
			va_start(args, format);

			char str[2048];
			T2String8::FormatV(str, sizeof(str), format, args);
			printf(str);

			va_end(args);

			return 0;
		}
		
		virtual int WPrintf(const wchar_t* format, ...)
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
		
		virtual int VCPrintf(const char* format, va_list vargs)
		{
			char str[2048];
			T2String8::FormatV(str, sizeof(str), format, vargs);
			printf(str);

			return 0;
		}
		
		virtual int VWPrintf(const wchar_t* format, ...)
		{
			TASSERT(!"Console doesn't support this");
			return 0;
		}
	};

	TConsoleFileSystem::TConsoleFileSystem(const char* a_szName) :
		TFileSystem(a_szName)
	{
		TFileManager::GetSingletonSafe()->MountFileSystem(this);
	}

	TFile* TConsoleFileSystem::CreateFile(TString8 const& fn, uint32_t flags)
	{
		return new TConsoleFile(this);
	}

	void TConsoleFileSystem::DestroyFile(TFile* a_pFile)
	{
		if (a_pFile != TNULL)
		{
			delete a_pFile;
		}
	}

	TString8 TConsoleFileSystem::MakeInternalPath(TString8 const&)
	{
		return TString8();
	}

	TBOOL TConsoleFileSystem::MakeDirectory(TString8 const&)
	{
		return TFALSE;
	}

	TBOOL TConsoleFileSystem::GetNextFile(TString8& fileName, uint32_t flags)
	{
		return TFALSE;
	}

}