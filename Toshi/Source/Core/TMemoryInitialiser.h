#pragma once
#include "TMemory.h"
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

class TMemoryInitialiser
{
public:
    TMemoryInitialiser()
    {
        Initialise();
    }

    ~TMemoryInitialiser()
    {
        Deinitialise();
    }

    TINT Initialise()
    {
        TMemory::Initialise( 0, 0 );
        return 0;
    }

    void Deinitialise()
    {
        TMemory::Deinitialise();
    }
};

TOSHI_NAMESPACE_END
