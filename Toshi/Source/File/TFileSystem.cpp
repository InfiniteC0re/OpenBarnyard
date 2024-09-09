#include "ToshiPCH.h"
#include "TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TFileSystem::TFileSystem( const TFileSystem& other )
{
	m_Name   = TString8( other.GetName() );
	m_Prefix = TString8( other.GetPrefix() );
}

// $Barnyard FUNCTION 006be2e0
TFileSystem::TFileSystem( const TCHAR* name ) :
    m_Name( name )
{
}

// $Barnyard FUNCTION 006bdd30
void TFileSystem::SetPrefix( const TString8& a_rcPrefix )
{
	m_Prefix = a_rcPrefix;

	for ( TINT32 i = 0; i < m_Prefix.Length(); i++ )
	{
		if ( m_Prefix[ i ] == '/' )
		{
			m_Prefix[ i ] = '\\';
		}
	}
}

// $Barnyard FUNCTION 006bdd80
TBOOL TFileSystem::ToNative( const TString8& a_rcPath, TNativeFileInfo& a_rOutInfo )
{
	a_rOutInfo.InternalPath = MakeInternalPath( a_rcPath );
	a_rOutInfo.Offset       = 0;
	a_rOutInfo.Length       = 0;
	return TTRUE;
}

TFileSystem& TFileSystem::operator=( TFileSystem& a_rFileSystem )
{
	m_Name   = a_rFileSystem.GetName();
	m_Prefix = a_rFileSystem.GetPrefix();
	return *this;
}

TOSHI_NAMESPACE_END
