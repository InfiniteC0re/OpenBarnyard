#include "ToshiPCH.h"
#include "Toshi/Platform.h"
#include "Toshi/TString8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

void ThreadSleep( TUINT dwMilliseconds )
{
	Sleep( 20 );
}

void FixPathSlashes( TString8& a_rPath )
{
	for ( TUINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '/' )
			a_rPath[ i ] = '\\';
	}
}

void FixPathSlashesBack( TString8& a_rPath )
{
	for ( TUINT i = 0; i < a_rPath.Length(); i++ )
	{
		if ( a_rPath[ i ] == '\\' )
			a_rPath[ i ] = '/';
	}
}

TOSHI_NAMESPACE_END