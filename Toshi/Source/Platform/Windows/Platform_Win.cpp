#include "ToshiPCH.h"
#include "Toshi/Platform.h"
#include "Toshi/TString8.h"

#include <Windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void ThreadSleep( TUINT dwMilliseconds )
{
	Sleep( dwMilliseconds );
}

void FixPathSlashes( TString8& a_rPath )
{
	for ( TINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '/' )
			a_rPath[ i ] = '\\';
	}
}

void FixPathSlashesBack( TString8& a_rPath )
{
	for ( TINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '\\' )
			a_rPath[ i ] = '/';
	}
}

TOSHI_NAMESPACE_END
