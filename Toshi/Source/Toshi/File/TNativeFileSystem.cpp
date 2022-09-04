#include "ToshiPCH.h"
#include "TNativeFileSystem.h"

Toshi::TNativeFileSystem::TNativeFileSystem(const char* a_name) : TFileSystem(a_name)
{
    //sm_unk3 = -1;
    // Toshi::TFileManager::MountFileSystem(TFileManager::s_pFileManager, this); 
}

Toshi::TFile* Toshi::TNativeFileSystem::CreateFile(TCString const& param_1, unsigned int param_2)
{
    TNativeFile* nativeFile = tnew<TNativeFile>(this);

    if (nativeFile->Open(param_1, param_2))
    {
        return nativeFile;
    }

    return TNULL;
}

void Toshi::TNativeFileSystem::DestroyFile(TFile* file)
{
    if (file != TNULL)
    {
        ((TNativeFile*)file)->Close();
    }
}
