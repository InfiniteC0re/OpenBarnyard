#include "ToshiPCH.h"
#include "TNullFileSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

namespace Toshi
{

TFile* TNullFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
    TIMPLEMENT();
    return TNULL;
}

void TNullFileSystem::DestroyFile( TFile* )
{
    TIMPLEMENT();
}

TString8 TNullFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
    TIMPLEMENT();
    return TString8();
}

} // namespace Toshi