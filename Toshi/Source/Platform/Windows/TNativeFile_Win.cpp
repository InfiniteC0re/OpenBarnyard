#include "ToshiPCH.h"
#include "TNativeFile_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

#pragma region TNativeFileSystem

    TNativeFileSystem::TNativeFileSystem(const TCHAR* name) : TFileSystem(name)
    {
        m_Handle = INVALID_HANDLE_VALUE;
        TFileManager::GetSingletonSafe()->MountFileSystem(this);
    }

    TFile* TNativeFileSystem::CreateFile(const TString8& a_rcFileName, TFILEMODE a_eFileMode)
    {
        TNativeFile* nativeFile = new TNativeFile(this);

        if (!nativeFile->Open(a_rcFileName, a_eFileMode))
        {
            delete nativeFile;
            return TNULL;
        }

        return nativeFile;
    }

    void TNativeFileSystem::DestroyFile(TFile* file)
    {
        if (file != TNULL)
        {
            TSTATICCAST(TNativeFile*, file)->Close();
            delete file;
        }
    }

    TBOOL TNativeFileSystem::MakeDirectory(TString8 const& string)
    {
        return CreateDirectoryA(string, TNULL);
    }

    TBOOL TNativeFileSystem::GetNextFile(TString8& a_rOutFileName, TUINT32 a_uiFlags)
    {
        WIN32_FIND_DATAA findFileData;

        if (m_Handle != INVALID_HANDLE_VALUE)
        {
            TBOOL bResult = FindNextFileA(m_Handle, &findFileData);
            if (!bResult)
            {
                m_Handle = INVALID_HANDLE_VALUE;
            }
            else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((a_uiFlags & 1) != 0)
                {
                    a_rOutFileName = findFileData.cFileName;
                    return TTRUE;
                }
            }
            else if ((a_uiFlags & 2) != 0)
            {
                a_rOutFileName = findFileData.cFileName;
                return TTRUE;
            }
        }

        return TFALSE;
    }

#pragma endregion

    TNativeFile::TNativeFile(TNativeFileSystem* pFS) : TFile(pFS)
    {
        m_Handle = INVALID_HANDLE_VALUE;
        m_Position = -1;
        m_RBufferPosition = -1;
        m_PrevBufferPos = -1;
        m_LastBufferSize = 0;
        m_RBuffer = TNULL;
        m_WBuffer = TNULL;
        m_WriteBufferUsed = 0;
        m_WriteBuffered = TTRUE;
    }

    TNativeFile::TNativeFile(const TNativeFile& other) : TFile(other)
    {
        m_Handle = other.m_Handle;
        m_Position = other.m_Position;
        m_RBufferPosition = other.m_RBufferPosition;
        m_PrevBufferPos = other.m_PrevBufferPos;
        m_LastBufferSize = other.m_LastBufferSize;
        m_RBuffer = other.m_RBuffer;
        m_WBuffer = other.m_WBuffer;
        m_WriteBufferUsed = other.m_WriteBufferUsed;
        m_WriteBuffered = other.m_WriteBuffered;
    }

    TBOOL TNativeFile::LoadBuffer(DWORD bufferPos)
    {
        // FUN_00689ff0
        DWORD lpNumberOfBytesRead;
        TASSERT(m_RBuffer != TNULL, "m_RBuffer doesn't exist");

        if (m_PrevBufferPos != bufferPos)
        {
            if (m_RBufferPosition != bufferPos)
            {
                m_RBufferPosition = SetFilePointer(m_Handle, bufferPos, TNULL, FILE_BEGIN);
                if (m_RBufferPosition != bufferPos) return TFALSE;
            }

            if (ReadFile(m_Handle, m_RBuffer, BUFFER_SIZE, &lpNumberOfBytesRead, TNULL) == 0)
            {
                return TFALSE;
            }

            m_RBufferPosition += lpNumberOfBytesRead;
            m_LastBufferSize = lpNumberOfBytesRead;
            m_PrevBufferPos = bufferPos;
        }

        return TTRUE;
    }

    TINT TNativeFile::FlushWriteBuffer()
    {
        DWORD lpNumberOfBytesWritten;

        if (m_Position != m_RBufferPosition)
        {
            TASSERT(TFALSE == m_WriteBuffered || m_WriteBufferUsed == 0);
            m_RBufferPosition = SetFilePointer(m_Handle, m_Position, NULL, FILE_BEGIN);
            
            if (m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }

            m_Position = m_RBufferPosition;
        }

        if (WriteFile(m_Handle, m_WBuffer, m_WriteBufferUsed, &lpNumberOfBytesWritten, TNULL) == 0)
        {
            return 0;
        }

        m_RBufferPosition += lpNumberOfBytesWritten;
        m_Position = m_RBufferPosition;
        m_WriteBufferUsed = 0;
        return lpNumberOfBytesWritten;
    }

    TINT TNativeFile::ReadUnbuffered(LPVOID dst, TUINT size)
    {
        DWORD lpNumberOfBytesRead;
        FlushWriteBuffer();

        if (m_Position != m_RBufferPosition)
        {
            m_RBufferPosition = SetFilePointer(m_Handle, m_Position, TNULL, FILE_BEGIN);

            if (m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }

            m_Position = m_RBufferPosition;
        }

        if (ReadFile(m_Handle, dst, size, &lpNumberOfBytesRead, TNULL) == 0)
        {
            return 0;
        }

        m_RBufferPosition += lpNumberOfBytesRead;
        m_Position = m_RBufferPosition;

        return lpNumberOfBytesRead;
    }

    TUINT TNativeFile::Read(void* a_pDst, TUINT a_uiSize)
    {
        FlushWriteBuffer();

        if (a_uiSize < 1) return 0;

        if (m_RBuffer != TNULL)
        {
            DWORD readedCount = 0;
            DWORD startPos = m_Position;
            DWORD curBufferPos = startPos / BUFFER_SIZE * BUFFER_SIZE;
            DWORD newBufferPos = (startPos + a_uiSize) / BUFFER_SIZE * BUFFER_SIZE;
            LPVOID curPosBuffer = a_pDst;

            if (curBufferPos != newBufferPos)
            {
                if (curBufferPos == m_PrevBufferPos)
                {
                    DWORD readCount = m_LastBufferSize - (startPos - curBufferPos);

                    if (readCount > 0)
                    {
                        TUtil::MemCopy(a_pDst, m_RBuffer + startPos - curBufferPos, readCount);
                        
                        curPosBuffer = (TCHAR*)a_pDst + readCount;
                        m_Position += readCount;
                        readedCount = readCount;
                    }
                }

                DWORD toReadCount = newBufferPos - m_Position;
                curBufferPos = newBufferPos;

                if (toReadCount > 0)
                {
                    TUINT32 readed = ReadUnbuffered(curPosBuffer, toReadCount);
                    curPosBuffer = (TCHAR*)curPosBuffer + readed;
                    readedCount += readed;

                    if (readed != toReadCount)
                    {
                        // end of file?
                        return readedCount;
                    }
                }
            }

            if (readedCount != a_uiSize && LoadBuffer(curBufferPos))
            {
                a_uiSize -= readedCount;
                DWORD bufferLeftSize = m_Position - curBufferPos;
                DWORD readCount = m_LastBufferSize - bufferLeftSize;
                readCount = TMath::Min<DWORD>(readCount, a_uiSize);

                if (readCount > 0)
                {
                    TUtil::MemCopy(curPosBuffer, m_RBuffer + bufferLeftSize, readCount);
                    m_Position += readCount;
                    readedCount += readCount;
                }
            }

            return readedCount;
        }

        return ReadUnbuffered(a_pDst, a_uiSize);
    }

    TUINT TNativeFile::Write(const void* buffer, TUINT size)
    {
        if (m_RBufferPosition != m_Position)
        {
            TASSERT(TFALSE == m_WriteBuffered || m_WriteBufferUsed == 0);
            m_RBufferPosition = SetFilePointer(m_Handle, m_Position, NULL, FILE_BEGIN);

            if (m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return 0;
            }

            m_Position = m_RBufferPosition;
        }

        if (m_WriteBuffered == TFALSE)
        {
            DWORD written;
            BOOL bRes = WriteFile(m_Handle, buffer, size, &written, NULL);

            if (bRes == FALSE)
            {
                return 0;
            }

            m_RBufferPosition += written;
            m_Position = m_RBufferPosition;
            return written;
        }

        // Flush write buffer if data doesn't fit it
        if (m_WriteBufferUsed + size >= BUFFER_SIZE)
        {
            FlushWriteBuffer();
        }

        if (size < BUFFER_SIZE)
        {
            // Data fits the write buffer so append it to it
            TUtil::MemCopy(m_WBuffer + m_WriteBufferUsed, buffer, size);
            m_WriteBufferUsed += size;
            return size;
        }
        else
        {
            // Data doesn't fit the write buffer at all so write it right now
            DWORD written;
            BOOL bRes = WriteFile(m_Handle, buffer, size, &written, NULL);

            if (bRes != FALSE)
            {
                m_RBufferPosition += size;
                m_Position = m_RBufferPosition;
                return written;
            }
        }

        return 0;
    }

    TUINT32 TNativeFile::Tell()
    {
        FlushWriteBuffer();
        return m_Position;
    }

    TBOOL TNativeFile::Seek(TINT a_iOffset, TSEEK a_eOrigin)
    {
        FlushWriteBuffer();

        if (a_eOrigin == TSEEK_SET)
        {
            m_Position = a_iOffset;
        }
        else if (a_eOrigin == TSEEK_CUR)
        {
            m_Position += a_iOffset;
        }
        else if (a_eOrigin == TSEEK_END)
        {
            m_RBufferPosition = SetFilePointer(m_Handle, a_iOffset, TNULL, FILE_END);
            
            if (m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
            {
                return TFALSE;
            }

            m_Position = m_RBufferPosition;
        }

        if (m_Position < 0) m_Position = 0;

        return TTRUE;
    }

    TCHAR TNativeFile::GetCChar()
    {
        FlushWriteBuffer();

        if (m_RBuffer != TNULL)
        {
            TUINT32 curBufferPos = m_Position / BUFFER_SIZE * BUFFER_SIZE;
            if ((curBufferPos == m_PrevBufferPos) && (m_Position - curBufferPos <= m_LastBufferSize - 1))
            {
                TCHAR c = m_RBuffer[m_Position - curBufferPos];
                m_Position += sizeof(c);
                return c;
            }
        }

		TCHAR result;
		if (Read(&result, sizeof(result)) != sizeof(result))
			return -1;

        return result;
    }

	TWCHAR TNativeFile::GetWChar()
	{
		FlushWriteBuffer();

		if (m_RBuffer != TNULL)
		{
			TUINT32 curBufferPos = m_Position / BUFFER_SIZE * BUFFER_SIZE;
			if ((curBufferPos == m_PrevBufferPos) && (m_Position - curBufferPos <= m_LastBufferSize - 1))
			{
				TWCHAR c = *TREINTERPRETCAST(TWCHAR*, &m_RBuffer[m_Position - curBufferPos]);
				m_Position += sizeof(c);
				return c;
			}
		}

		TWCHAR result;
		if (Read(&result, sizeof(result)) != sizeof(result))
			return L'\xFFFF';

		return result;
	}

	TINT TNativeFile::PutCChar(TCHAR a_cCharacter)
	{
        if (Write(&a_cCharacter, 1) != 1)
        {
            return -1;
        }

        return a_cCharacter & 0xFF;
	}

	TINT TNativeFile::PutWChar(TWCHAR a_wcCharacter)
	{
		if (Write(&a_wcCharacter, 2) != 2)
		{
			return -1;
		}

		return a_wcCharacter & 0xFFFF;
	}

	TINT TNativeFile::CPrintf(const TCHAR* a_szFormat, ...)
    {
        va_list args; 
        va_start(args, a_szFormat);

        TCHAR str[512];

        TINT iResult = T2String8::FormatV(str, TARRAYSIZE(str), a_szFormat, args);

        va_end(args);

        Write(str, iResult);

        return iResult;
    }

    TINT TNativeFile::WPrintf(const TWCHAR* a_wszFormat, ...)
    {
        va_list args;
        va_start(args, a_wszFormat);

        TWCHAR str[512];

        TINT iResult = T2String16::FormatV(str, TARRAYSIZE(str), a_wszFormat, args);

        Write(str, iResult * sizeof(TWCHAR));

        va_end(args);

        return iResult;
    }

    TINT TNativeFile::VCPrintf(const TCHAR* a_szFormat, va_list a_vargs)
    {
        TCHAR str[512];

        TINT iResult = T2String8::FormatV(str, TARRAYSIZE(str), a_szFormat, a_vargs);

        Write(str, iResult);

        return iResult;
    }

	TINT TNativeFile::VWPrintf(const TWCHAR* a_wszFormat, va_list a_vargs)
	{
        TWCHAR str[512];

		TINT iResult = T2String16::FormatV(str, TARRAYSIZE(str), a_wszFormat, a_vargs);

		Write(str, iResult * sizeof(TWCHAR));

		return iResult;
	}

	TUINT TNativeFile::GetSize()
    {
        m_RBufferPosition = SetFilePointer(m_Handle, 0, TNULL, TSEEK_END);
        
        if (m_RBufferPosition == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        {
            return 0;
        }

        return m_RBufferPosition;
    }

    _FILETIME TNativeFile::GetDate()
    {
        _FILETIME fLastWriteTime;
        fLastWriteTime.dwLowDateTime = 0;
        fLastWriteTime.dwHighDateTime = 0;

        if (m_Handle != TNULL)
        {
            GetFileTime(m_Handle, NULL, NULL, &fLastWriteTime);
        }
        
        return fLastWriteTime;
    }

    TBOOL TNativeFile::Open(const TString8& a_FileName, TFILEMODE a_Mode)
    {
        TASSERT(a_FileName.IsIndexValid(0), "TNativeFile::Open - wrong filename");

        DWORD dwCreationDisposition = OPEN_EXISTING;
        DWORD dwDesiredAccess = 0;
        DWORD dwShareMode = 0;

        dwDesiredAccess |= (a_Mode & TFILEMODE_READ)  ? GENERIC_READ  : dwDesiredAccess;
        dwDesiredAccess |= (a_Mode & TFILEMODE_WRITE) ? GENERIC_WRITE : dwDesiredAccess;
        dwDesiredAccess |= (a_Mode & TFILEMODE_READWRITE) ? (GENERIC_READ | GENERIC_WRITE) : dwDesiredAccess;

        if (a_Mode & TFILEMODE_CREATENEW)
        {
            dwShareMode = FILE_SHARE_READ;
            dwCreationDisposition = CREATE_ALWAYS;
            dwDesiredAccess |= GENERIC_WRITE;
        }

        if (dwDesiredAccess == 0)
        {
            TWARN("WARNING: File created with no access mode, assuming WRITEONLY\n");
            dwDesiredAccess = GENERIC_WRITE;
        }

        m_Handle = CreateFileA(a_FileName.GetString(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, NULL, NULL);

        if (m_Handle != INVALID_HANDLE_VALUE)
        {
            m_Position = 0;
            m_RBufferPosition = 0;
            m_PrevBufferPos = -1;
            m_LastBufferSize = 0;

            if (a_Mode & TFILEMODE_NOBUFFER)
            {
                m_WriteBuffered = TFALSE;
            }
            else
            {
                m_RBuffer = (TCHAR*)TMalloc(BUFFER_SIZE);
                m_WBuffer = (TCHAR*)TMalloc(BUFFER_SIZE);
                m_WriteBuffered = TTRUE;
            }
        }
        else
        {
            return TFALSE;
        }

        return TTRUE;
    }

    void TNativeFile::Close()
    {
        FlushWriteBuffer();
        CloseHandle(m_Handle);

        m_Handle = INVALID_HANDLE_VALUE;
        m_Position = -1;
        m_RBufferPosition = -1;
        m_PrevBufferPos = -1;
        m_LastBufferSize = -1;

        if (m_RBuffer != TNULL)
        {
            TFree(m_RBuffer);
            m_RBuffer = TNULL;
        }

        if (m_WBuffer != TNULL)
        {
            TFree(m_WBuffer);
            m_WBuffer = TNULL;
        }
    }
}