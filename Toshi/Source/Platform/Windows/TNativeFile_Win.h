#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TNativeFile : public TFile
	{
	public:
		TNativeFile(const TNativeFile& other);

		virtual size_t Read(void*, size_t) override;
		virtual size_t Write(const void*, size_t) { return 0; }
		virtual bool Seek(int, TFile::TSEEK) override;
		virtual uint32_t Tell() override;
		virtual DWORD GetSize() override;
		virtual _FILETIME GetDate() override;
		virtual char GetCChar() override;
		virtual wchar_t GetWChar() { return 0; }
		virtual int PutCChar(char character) { return 0; }
		virtual int PutWChar(wchar_t character) { return 0; }
		virtual int CPrintf(const char* format, ...) { va_list args; va_start(args, format); return printf(format, args); }
		virtual int VCPrintf(const char* format, ...) { return 0; }
		virtual int VWPrintf(const wchar_t* format, ...) { return 0; }
		
		bool LoadBuffer(DWORD bufferPos);
		int FlushWriteBuffer();
		int ReadUnbuffered(LPVOID dst, size_t size);

	protected:
		TNativeFile(TNativeFileSystem* pFS);

		bool Open(const TString8& a_FileName, uint32_t a_Flags);
		void Close();

		friend TNativeFileSystem;

	private:
		HANDLE m_Handle;             // 0x8
		DWORD m_Position;            // 0xC
		DWORD m_RBufferPosition;     // 0x10
		DWORD m_PrevBufferPos;       // 0x14
		DWORD m_LastBufferSize;      // 0x18
		char* m_RBuffer;             // 0x1C (read buffer)
		char* m_WBuffer;             // 0x20 (write buffer)
		DWORD m_WBufferIterator;     // 0x24
		bool m_UseBuffers;           // 0x28
	};
}


