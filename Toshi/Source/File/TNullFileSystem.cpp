#include "ToshiPCH.h"
#include "TNullFileSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

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

TOSHI_NAMESPACE_END
