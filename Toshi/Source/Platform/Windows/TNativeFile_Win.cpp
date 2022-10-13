#include "ToshiPCH.h"
#include "TNativeFile_Win.h"

#define BUFFER_SIZE 0x800
#define BUFFER_SIZE_ALIGN (0xFFFFFFFF - BUFFER_SIZE + 1)

namespace Toshi
{
    void TFileManager::Initialize()
    {
        auto& fileManager = TFileManager::Instance();

        CHAR currentDir[0x200];
        DWORD dirLength = GetCurrentDirectoryA(sizeof(currentDir), currentDir);
        TASSERT(dirLength > 0, "The directory's length is 0");

        TCString prefix(currentDir);

        TFileSystem* pFileSystem;
        pFileSystem = tnew<TNativeFileSystem>("local");
        pFileSystem->SetPrefix(prefix);
        pFileSystem = tnew<TNativeFileSystem>("abs");
        pFileSystem->SetPrefix(prefix);
        pFileSystem = tnew<TNullFileSystem>("null");
        pFileSystem->SetPrefix(prefix);

        fileManager.SetSystemPath("local");
    }

#pragma region TNativeFileSystem

    TNativeFileSystem::TNativeFileSystem(const char* name) : TFileSystem(name)
    {
        m_Handle = INVALID_HANDLE_VALUE;
        TFileManager::Instance().MountFileSystem(this);
    }

    TFile* TNativeFileSystem::CreateFile(TCString const& fn, uint32_t flags)
    {
        TNativeFile* nativeFile = tmalloc<TNativeFile>();
        new (nativeFile) TNativeFile(this);

        if (!nativeFile->Open(fn, flags))
        {
            tdelete(nativeFile);
            return TNULL;
        }

        return nativeFile;
    }

    void TNativeFileSystem::DestroyFile(TFile* file)
    {
        if (file != TNULL)
        {
            static_cast<TNativeFile*>(file)->Close();
        }
    }

    bool TNativeFileSystem::MakeDirectory(TCString const& string)
    {
        return CreateDirectoryA(string, TNULL);
    }

    bool TNativeFileSystem::GetNextFile(TCString& fileName, uint32_t flags)
    {
        WIN32_FIND_DATAA findFileData;
        if (m_Handle != INVALID_HANDLE_VALUE)
        {
            bool bResult = FindNextFileA(m_Handle, &findFileData);
            if (!bResult)
            {
                m_Handle = INVALID_HANDLE_VALUE;
            }
            else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((flags & 1) != 0)
                {
                    fileName = findFileData.cFileName;
                    return true;
                }
            }
            else if ((flags & 2) != 0)
            {
                fileName = findFileData.cFileName;
                return true;
            }
        }

        return false;
    }

#pragma endregion

    bool TNativeFile::LoadBuffer(DWORD bufferPos)
    {
        // FUN_00689ff0
        DWORD lpNumberOfBytesRead;
        TASSERT(m_pBuffer1 != TNULL, "m_pBuffer1 doesn't exist");

        if (m_PrevBufferPos != bufferPos)
        {
            if (unk2 != bufferPos)
            {
                unk2 = SetFilePointer(hnd, bufferPos, TNULL, FILE_BEGIN);
                if (unk2 != bufferPos) return false;
            }

            if (ReadFile(hnd, m_pBuffer1, BUFFER_SIZE, &lpNumberOfBytesRead, TNULL) == 0)
            {
                return false;
            }

            unk2 += lpNumberOfBytesRead;
            m_LastBufferSize = lpNumberOfBytesRead;
            m_PrevBufferPos = bufferPos;
        }

        return true;
    }

    int TNativeFile::FlushWriteBuffer()
    {
        DWORD lpNumberOfBytesWritten;

        if (m_Position != unk2)
        {
            TASSERT(TFALSE == m_bWriteBuffered || m_iWriteBufferUsed == 0, "");
            unk2 = SetFilePointer(hnd, m_Position, TNULL, FILE_BEGIN);
            
            if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }

            m_Position = unk2;
        }

        if (WriteFile(hnd, m_pBuffer2, m_iWriteBufferUsed, &lpNumberOfBytesWritten, TNULL) == 0)
        {
            return 0;
        }

        unk2 += lpNumberOfBytesWritten;
        m_Position = unk2;
        m_iWriteBufferUsed = 0;
        return lpNumberOfBytesWritten;
    }

    int TNativeFile::ReadUnbuffered(LPVOID dst, size_t size)
    {
        DWORD lpNumberOfBytesRead;
        FlushWriteBuffer();

        if (m_Position != unk2)
        {
            unk2 = SetFilePointer(hnd, m_Position, TNULL, FILE_BEGIN);
            if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }
            m_Position = unk2;
        }

        if (ReadFile(hnd, dst, size, &lpNumberOfBytesRead, TNULL) == 0)
        {
            return 0;
        }

        unk2 += lpNumberOfBytesRead;
        m_Position = unk2;

        return lpNumberOfBytesRead;
    }

    size_t TNativeFile::Read(LPVOID dst, size_t size)
    {
        FlushWriteBuffer();

        if (size < 1) { return 0; }

        if (m_pBuffer1 != TNULL)
        {
            size_t readedCount = 0;
            DWORD startPos = m_Position;
            DWORD curBufferPos = startPos & BUFFER_SIZE_ALIGN;
            DWORD newBufferPos = startPos + size & BUFFER_SIZE_ALIGN;
            LPVOID curPosBuffer = dst;

            if (curBufferPos != newBufferPos)
            {
                if (curBufferPos == m_PrevBufferPos)
                {
                    DWORD readCount = m_LastBufferSize - (startPos - curBufferPos);

                    if (readCount > 0)
                    {
                        memcpy(dst, m_pBuffer1 + startPos - curBufferPos, readCount);
                        
                        curPosBuffer = (char*)dst + readCount;
                        m_Position += readCount;
                        readedCount = readCount;
                    }
                }

                DWORD toReadCount = newBufferPos - m_Position;
                curBufferPos = newBufferPos;

                if (toReadCount > 0)
                {
                    uint32_t readed = ReadUnbuffered(curPosBuffer, toReadCount);
                    curPosBuffer = (char*)curPosBuffer + readed;
                    readedCount += readed;

                    if (readed != toReadCount)
                    {
                        // end of file?
                        return readedCount;
                    }
                }
            }

            if (readedCount != size && LoadBuffer(curBufferPos))
            {
                size -= readedCount;
                DWORD bufferLeftSize = m_Position - curBufferPos;
                DWORD readCount = m_LastBufferSize - bufferLeftSize;
                readCount = TMath::Min(readCount, size);

                if (readCount > 0)
                {
                    memcpy(curPosBuffer, m_pBuffer1 + bufferLeftSize, readCount);
                    m_Position += readCount;
                    readedCount += readCount;
                }
            }

            return readedCount;
        }

        return ReadUnbuffered(dst, size);;
    }

    uint32_t TNativeFile::Tell()
    {
        FlushWriteBuffer();
        return m_Position;
    }


    bool TNativeFile::Seek(int offset, TFile::TSEEK origin)
    {
        FlushWriteBuffer();

        if (origin == TFile::TSEEK_SET)
        {
            if (offset < 0)
            {
                offset = 0;
            }
            m_Position = offset;
        }
        else if (origin == TFile::TSEEK_CUR)
        {
            m_Position += offset;
            if (m_Position < 0)
            {
                m_Position = 0;
                return true;
            }
        }
        else if (origin == TFile::TSEEK_END)
        {
            DWORD r = SetFilePointer(hnd, offset, TNULL, FILE_END);
            unk2 = r;
            if (r == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return false;
            }
            m_Position = unk2;
            return true;
        }

        return true;
    }

    char TNativeFile::GetCChar()
    {
        FlushWriteBuffer();
        if (m_pBuffer1 != TNULL)
        {
            uint32_t unk = (m_Position & BUFFER_SIZE_ALIGN);
            if ((unk == m_PrevBufferPos) && (m_Position - unk <= m_LastBufferSize-1))
            {
                char c = ((char*)m_pBuffer1)[m_Position - unk];
                m_Position++;
                return c;
            }
        }
        //   iVar3 = (*(code *)*this->field_0x0)(&stack0xfffffff4,1);
        //   if (iVar3 != 1) {
        //      return 0xffffffff;
        //   }
        //   return puVar4 & 0xFF;
        return 0;
    }

    int TNativeFile::GetSize()
    {
        unk2 = SetFilePointer(hnd, 0, TNULL, TSEEK_END);
        if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        {
            return 0;
        }
        return unk2;
    }

    _FILETIME TNativeFile::GetDate()
    {
        _FILETIME fCreationTime;
        _FILETIME fLastAccessTime;
        _FILETIME fLastWriteTime;

        if (hnd != TNULL)
        {
            GetFileTime(hnd, &fCreationTime, &fLastAccessTime, &fLastWriteTime);
        }
        
        return fLastWriteTime;
    }

    TNativeFile::TNativeFile(TNativeFileSystem* pFS) : TFile(pFS)
    {
        hnd = INVALID_HANDLE_VALUE;
        m_Position = -1;
        unk2 = -1;
        m_PrevBufferPos = -1;
        m_LastBufferSize = 0;
        m_pBuffer1 = TNULL;
        m_pBuffer2 = TNULL;
        m_iWriteBufferUsed = 0;
        m_bWriteBuffered = true;
    }

    TNativeFile::TNativeFile(const TNativeFile& param_1) : TFile(param_1)
    {
        hnd = param_1.hnd;
        m_Position = param_1.m_Position;
        unk2 = param_1.unk2;
        m_PrevBufferPos = param_1.m_PrevBufferPos;
        m_LastBufferSize = param_1.m_LastBufferSize;
        m_pBuffer1 = param_1.m_pBuffer1;
        m_pBuffer2 = param_1.m_pBuffer2;
        m_iWriteBufferUsed = param_1.m_iWriteBufferUsed;
        m_bWriteBuffered = param_1.m_bWriteBuffered;
    }

    bool TNativeFile::Open(const TCString& a_FileName, uint32_t a_Flags)
    {
        DWORD dwDesiredAccess = 0;
        DWORD dwCreationDisposition = 0;
        DWORD dwShareMode = 0;

        // The guess is that this + 4 is TCString maybe
        // (**(code **)(**(int **)(this + 4) + 0x10))();
       
        // that is probably m_pFileSystem->MakeInternalPath()
        // since (this + 4) is TFile::m_pFileSystem
        // and the first field of TFileSystem is a virtual
        // functions table where MakeInternalPath is placed at
        // offset 0x10

        dwDesiredAccess |= a_Flags & OpenFlags_Read ? GENERIC_READ : 0;
        dwDesiredAccess |= a_Flags & OpenFlags_Write ? GENERIC_WRITE : 0;
        dwDesiredAccess |= a_Flags & OpenFlags_ReadWrite ? (GENERIC_READ | GENERIC_WRITE) : 0;

        if (a_Flags & OpenFlags_CreateNew)
        {
            dwShareMode = FILE_SHARE_READ;
            dwCreationDisposition = CREATE_ALWAYS;

            // add GENERIC_WRITE if it is not yet
            if (~dwDesiredAccess & GENERIC_WRITE)
            {
                dwDesiredAccess |= GENERIC_WRITE;
            }
        }
        else
        {
            dwCreationDisposition = OPEN_EXISTING;
        }

        TASSERT(a_FileName.IsIndexValid(0), "TNativeFile::Open - wrong filename");

        HANDLE handle = CreateFileA(a_FileName.GetString(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, NULL, NULL);

        if (handle != INVALID_HANDLE_VALUE)
        {
            hnd = handle;
            m_Position = 0;
            unk2 = 0;
            m_PrevBufferPos = -1;
            m_LastBufferSize = 0;

            if (a_Flags & OpenFlags_NoBuffer)
            {
                m_bWriteBuffered = false;
            }
            else
            {
                m_pBuffer1 = (char*)tmalloc(BUFFER_SIZE);
                m_pBuffer2 = (char*)tmalloc(BUFFER_SIZE);
                m_bWriteBuffered = true;
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    void TNativeFile::Close()
    {
        FlushWriteBuffer();
        CloseHandle(hnd);

        hnd = INVALID_HANDLE_VALUE;
        m_Position = -1;
        unk2 = -1;
        m_PrevBufferPos = -1;
        m_LastBufferSize = -1;

        if (m_pBuffer1 != TNULL)
        {
            tfree(m_pBuffer1);
            m_pBuffer1 = TNULL;
        }

        if (m_pBuffer2 != TNULL)
        {
            tfree(m_pBuffer2);
            m_pBuffer2 = TNULL;
        }
    }

}