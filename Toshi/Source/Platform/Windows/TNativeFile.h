#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/File/TFile.h"
#include "TNativeFileSystem.h"

namespace Toshi
{
	class TNativeFile : public TFile
	{
	public:
		TNativeFile(const TNativeFile&);

		virtual int Read(LPVOID, int) override;
		virtual int Write(LPVOID, int) { return 0; }
		virtual bool Seek(int, TFile::TSEEK) override;
		virtual int Tell() override;
		virtual int GetSize() override;
		virtual _FILETIME GetDate() override;
		virtual char GetCChar() override;
		virtual wchar_t GetWChar() { return 0; }
		virtual int PutCChar(char character) { return 0; }
		virtual int PutWChar(wchar_t character) { return 0; }
		virtual int VCPrintf(const char* format, ...) { return 0; }
		virtual int VWPrintf(const wchar_t* format, ...) { return 0; }
		
		bool LoadBuffer(DWORD);
		int FlushWriteBuffer();
		int ReadUnbuffered(LPVOID, int);

	protected:
		TNativeFile(class TNativeFileSystem*);

		bool Open(const TCString& a_FileName, uint32_t a_Flags);
		void Close();

		friend TNativeFileSystem;

	private:
		HANDLE hnd; // 0x8
		int m_position; // 0xC
		DWORD unk2; //0x10
		int unk4; // 0x14
		int unk5; // 0x18
		LPVOID m_pBuffer; // 0x1C
		LPVOID buffer; // 0x20
		int m_iWriteBufferUsed; // 0x24
		bool m_bWriteBuffered; // 0x28
	};
}


