#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"
#include "VideoCommon/TextureDecoder.h"
#include "Toshi/File/TTRB.h"

using namespace Toshi;

int TMain(int argc, char** argv)
{
    TTRB* trb = new TTRB();
    trb->Load(argv[1]);

    void* addr = trb->GetSymbolAddress("ttex");

    return 0;
}