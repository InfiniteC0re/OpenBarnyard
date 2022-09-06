#include "ToshiPCH.h"
#include "TNativeFile.h"

namespace Toshi
{
    bool TNativeFile::LoadBuffer(DWORD param_1)
    {
        DWORD lpNumberOfBytesRead;
        TASSERT(m_pBuffer != TNULL, "");
        if (unk4 != param_1)
        {
            if (unk2 != param_1)
            {
                unk2 = SetFilePointer(hnd, param_1, TNULL, FILE_BEGIN);
                if (unk2 != param_1) return false;
            }
            if (ReadFile(hnd, m_pBuffer, 0x800, &lpNumberOfBytesRead, TNULL) == 0)
            {
                return false;
            }
            unk2 += lpNumberOfBytesRead;
            unk5 = lpNumberOfBytesRead;
            unk4 = param_1;
        }
        return true;
    }

    int TNativeFile::FlushWriteBuffer()
    {
        DWORD lpNumberOfBytesWritten;
        if (m_position != unk2)
        {
            TASSERT(TFALSE == m_bWriteBuffered || m_iWriteBufferUsed == 0, "");
            unk2 = SetFilePointer(hnd, m_position, TNULL, FILE_BEGIN);
            if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }
            m_position = unk2;
        }
        if (WriteFile(hnd, buffer, m_iWriteBufferUsed, &lpNumberOfBytesWritten, TNULL) == 0)
        {
            return 0;
        }
        unk2 += lpNumberOfBytesWritten;
        m_position = unk2;
        m_iWriteBufferUsed = 0;
        return lpNumberOfBytesWritten;
    }

    int TNativeFile::ReadUnbuffered(LPVOID buffer, int param_2)
    {
        DWORD lpNumberOfBytesRead;
        FlushWriteBuffer();
        if (m_position != unk2)
        {
            unk2 = SetFilePointer(hnd, m_position, TNULL, FILE_BEGIN);
            if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }
            m_position = unk2;
        }
        if (ReadFile(hnd, buffer, param_2, &lpNumberOfBytesRead, TNULL) == 0)
        {
            return 0;
        }
        unk2 += lpNumberOfBytesRead;
        m_position = unk2;
        return lpNumberOfBytesRead;
    }

    int TNativeFile::Read(LPVOID param_1, int param_2)
    {
        FlushWriteBuffer();

        if (param_2 < 1)
        {
            return 0;
        }

        if (m_pBuffer == TNULL)
        {
            return ReadUnbuffered(param_1, param_2);
        }

        if (m_pBuffer != 0)
        {
            int v1 = m_position;
            int v2 = m_position & 0xFFFFF800;
            int v3 = m_position + param_2 & 0xFFFFF800;
            int v4 = 0;
            int v5 = 0;
            int v6 = 0;
            if (v2 != v3)
            {
                if (v2 == unk4)
                {
                    v4 = unk5 - (v1 - v2);
                    if (0 < v4)
                    {
                        memcpy(param_1, ((char*)m_pBuffer + (v1 - v2)), v4);
                        m_position += v4;
                        v6 += v4;
                        v5 = v4;
                    }
                }
                v2 = v3 - m_position;
                if (0 < v2)
                {
                    int read = ReadUnbuffered((char*)param_1 + v4, v2);
                    v5 += read;
                    v2 += read;
                    if (read != v2) return v5;
                }
            }
            if (v6 != param_2)
            {
                if (!LoadBuffer(v2)) return v6;
                int iVar5 = m_position - v2;
                int v8 = param_2 - v6;
                int v9 = unk5 - iVar5;
                int v10 = v9;
                if (v8 <= v9) v10 = v8;

                if (0 < v10)
                {
                    memcpy(param_1, ((char*)m_pBuffer + iVar5), v10);
                    m_position += v10;
                    v5 += v10;
                }
            }
            return v5;
        }

        return ReadUnbuffered(buffer, param_2);
    }

    int TNativeFile::Tell()
    {
        FlushWriteBuffer();
        return m_position;
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
            m_position = offset;
        }
        else if (origin == TFile::TSEEK_CUR)
        {
            m_position += offset;
            if (m_position < 0)
            {
                m_position = 0;
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
            m_position = unk2;
            return true;
        }

        return true;
    }

    char TNativeFile::GetCChar()
    {
        FlushWriteBuffer();
        if (m_pBuffer != TNULL)
        {
            int unk = (m_position & 0xFFFFF800);
            if ((unk == unk4) && (m_position - unk <= unk5-1))
            {
                char c = ((char*)m_pBuffer)[m_position - unk];
                m_position++;
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

    TNativeFile::TNativeFile(class TNativeFileSystem* param_1) : TFile((TFileSystem*)param_1)
    {
        hnd = INVALID_HANDLE_VALUE;
        m_position = -1;
        unk2 = -1;
        unk4 = -1;
        unk5 = 0;
        m_pBuffer = TNULL;
        buffer = TNULL;
        m_iWriteBufferUsed = 0;
        m_bWriteBuffered = true;
    }

    TNativeFile::TNativeFile(const TNativeFile& param_1) : TFile(param_1)
    {
        hnd = param_1.hnd;
        m_position = param_1.m_position;
        unk2 = param_1.unk2;
        unk4 = param_1.unk4;
        unk5 = param_1.unk5;
        m_pBuffer = param_1.m_pBuffer;
        buffer = param_1.buffer;
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

        HANDLE handle = CreateFileA(a_FileName, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, NULL, NULL);

        if (handle != INVALID_HANDLE_VALUE)
        {
            hnd = handle;
            m_position = 0;
            unk2 = 0;
            unk4 = -1;
            unk5 = 0;

            if (a_Flags & OpenFlags_NoBuffer)
            {
                m_bWriteBuffered = false;
            }
            else
            {
                m_pBuffer = tmalloc(0x800);
                buffer = tmalloc(0x800);
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
        m_position = -1;
        unk2 = -1;
        unk4 = -1;
        unk5 = -1;

        if (m_pBuffer != TNULL)
        {
            tfree((void*)m_pBuffer);
            m_pBuffer = TNULL;
        }
        if (buffer != TNULL)
        {
            tfree((void*)buffer);
            buffer = TNULL;
        }
    }

}