#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
    class TNativeFile : public TFile
    {
    public:
        static constexpr uint32_t BUFFER_SIZE = 0x800;

    public:
        TNativeFile(const TNativeFile& other);

        virtual size_t Read(void* dst, size_t size) override;
        virtual size_t Write(const void* buffer, size_t size) override;
        virtual TBOOL Seek(int offset, TFile::TSEEK seek) override;
        virtual uint32_t Tell() override;
        virtual DWORD GetSize() override;
        virtual _FILETIME GetDate() override;
        virtual char GetCChar() override;
        virtual wchar_t GetWChar() { return 0; }
        virtual int PutCChar(char character) { return 0; }
        virtual int PutWChar(wchar_t character) { return 0; }
        virtual int CPrintf(const char* format, ...);
        virtual int WPrintf(const wchar_t* format, ...);
        virtual int VCPrintf(const char* format, va_list* vargs);
        virtual int VWPrintf(const wchar_t* format, ...) { return 0; }

        TBOOL LoadBuffer(DWORD bufferPos);
        int FlushWriteBuffer();
        int ReadUnbuffered(LPVOID dst, size_t size);

    protected:
        TNativeFile(TNativeFileSystem* pFS);

        TBOOL Open(const TString8& a_FileName, FileMode a_Mode);
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
        DWORD m_WriteBufferUsed;     // 0x24
        TBOOL m_WriteBuffered;        // 0x28
    };
}


