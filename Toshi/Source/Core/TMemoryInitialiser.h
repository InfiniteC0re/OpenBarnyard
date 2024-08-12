#pragma once
#include "TMemory.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{

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

} // namespace Toshi
