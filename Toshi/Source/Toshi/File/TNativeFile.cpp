#include "pch.h"
#include "TNativeFile.h"

namespace Toshi
{
    bool TNativeFile::LoadBuffer(LPVOID param_1, DWORD param_2)
    {
        DWORD lpNumberOfBytesWritten;
        FlushWriteBuffer();
        if (unk2 != m_position)
        {
            unk2 = SetFilePointer(hnd, m_position, TNULL, FILE_BEGIN);
            if (unk2 == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return false;
            }
            m_position = unk2;
        }
        if (ReadFile(hnd, param_1, param_2, &lpNumberOfBytesWritten, TNULL) == 0)
        {
            return false;
        }
        unk2 += lpNumberOfBytesWritten;
        m_position = unk2;
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

    int TNativeFile::Read(LPVOID param_1, int param_2)
    {
        FlushWriteBuffer();

        if (param_2 < 1)
        {
            return 0;
        }

        if (m_pBuffer != 0)
        {
            int v1 = m_position;
            int v2 = m_position & 0xFFFFF800;
            int v3 = m_position + param_2 & 0xFFFFF800;
            int v5 = 0;
            int v6 = 0;
            if (v2 != v3)
            {
                if (v2 == unk4)
                {
                    int v4 = unk5 - (v1 - v2);
                    if (0 < v4)
                    {
                        memcpy(param_1, ((char*)m_pBuffer - (v1 - v2)), v4);
                        m_position += v4;
                        v6 += v4;
                        v5 = v4;
                    }
                }
                param_1 = (void*)(v3 + m_position);
            }
        }

        return 0;
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

    int TNativeFile::GetCChar()
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

    TNativeFile::TNativeFile(class TNativeFileSystem* param_1) : TFile(param_1)
    {
        hnd = TNULL;
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

    bool TNativeFile::Open(const TCString& a_fileName, unsigned int param_2)
    {
        DWORD dwDesiredAccess = 0;
        DWORD dwCreationDisposition = 0;
        DWORD dwShareMode = 0;

        // The guess is that this + 4 is TCString maybe
        // (**(code **)(**(int **)(this + 4) + 0x10))();


        if ((param_2 & 1) != 0)
        {
            dwDesiredAccess = GENERIC_READ;
        }
        if ((param_2 & 2) != 0)
        {
            dwDesiredAccess |= GENERIC_WRITE;
        }
        if ((param_2 & 4) != 0)
        {
            dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        }
        if ((param_2 & 8) == 0)
        {
            dwCreationDisposition = OPEN_EXISTING;
        }
        else
        {
            dwShareMode = FILE_SHARE_READ;
            dwCreationDisposition = CREATE_ALWAYS;
            if (dwDesiredAccess == 0)
            {
                dwDesiredAccess = GENERIC_WRITE;
            }
        }

        TASSERT(a_fileName.IsIndexValid(0), "");

        HANDLE handle = CreateFileA(a_fileName.GetString(0), dwDesiredAccess, dwShareMode, TNULL, dwCreationDisposition, 0, TNULL);

        if (handle != INVALID_HANDLE_VALUE)
        {
            hnd = handle;
            m_position = 0;
            unk2 = 0;
            unk4 = -1;
            unk5 = 0;

            if ((param_2 & 0x10) == 0)
            {
                void* mem = tmalloc(0x800);
                m_pBuffer = mem;
                mem = tmalloc(0x800);
                buffer = mem;
                m_bWriteBuffered = true;
            }
            else
            {
                m_bWriteBuffered = false;
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

        // Don't know why they -1 it, that's not possible, instead TNULL
        //*(undefined4 *)(param_1 + 8) = 0xffffffff; 
        hnd = TNULL;
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