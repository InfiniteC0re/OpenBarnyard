#include "pch.h"
#include "TNativeFileSystem.h"

Toshi::TFile* Toshi::TNativeFileSystem::CreateFile(TCString const& param_1, unsigned int param_2)
{
    TNativeFile* nativeFile = (TNativeFile*)tmalloc(0x2C);

    if (nativeFile != TNULL)
    {
        nativeFile = new TNativeFile(this);
        if (nativeFile->Open(param_1, param_2) != false)
        {
            return (TFile*)nativeFile;
        }

    }
    return TNULL;
}
