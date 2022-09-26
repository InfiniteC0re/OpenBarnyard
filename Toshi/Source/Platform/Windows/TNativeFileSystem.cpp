#include "ToshiPCH.h"
#include "TNativeFileSystem.h"

namespace Toshi
{
    Toshi::TNativeFileSystem::TNativeFileSystem(const char* a_name) : TFileSystem(a_name)
    {
        //sm_unk3 = -1;
        // Toshi::TFileManager::MountFileSystem(TFileManager::s_pFileManager, this); 
    }

    Toshi::TFile* Toshi::TNativeFileSystem::CreateFile(TCString const& fn, uint32_t flags)
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

    void Toshi::TNativeFileSystem::DestroyFile(TFile* file)
    {
        if (file != TNULL)
        {
            static_cast<TNativeFile*>(file)->Close();
        }
    }

    bool TNativeFileSystem::MakeDirectory(TCString const& string)
    {
        return CreateDirectoryA(string.GetString(), NULL);
    }

    bool TNativeFileSystem::GetNextFile(TCString& fileName, uint32_t flags)
    {
        WIN32_FIND_DATAA findFileData;
        if (m_handle != INVALID_HANDLE_VALUE)
        {
            bool bResult = FindNextFileA(m_handle, &findFileData);
            if (!bResult)
            {
                m_handle = INVALID_HANDLE_VALUE;
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
}