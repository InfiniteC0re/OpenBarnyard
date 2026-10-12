#include "pch.h"
#include "ALockedMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00608120, CreateObject 00608130, CreateObjectInPlace 00608190
TDEFINE_CLASS( ALockedMesh );

// $Barnyard: FUNCTION 005edf40
ALockedMesh::ALockedMesh()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_aUnk ); i++ )
		m_aUnk[ i ] = 0;
}

// $Barnyard: FUNCTION 0057fd50
ALockedMesh::~ALockedMesh()
{
}
