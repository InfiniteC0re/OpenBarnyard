#include "pch.h"
#include "ASimAnimModelHelperManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASimAnimModelHelperManager );

// $Barnyard: FUNCTION 0054b280
// $Barnyard: FUNCTION 0054b300
ASimAnimModelHelperManager::ASimAnimModelHelperManager()
    : m_iNumHelpers( 0 )
{
}

// $Barnyard: FUNCTION 0054b240
// $Barnyard: FUNCTION 0054b370
ASimAnimModelHelperManager::~ASimAnimModelHelperManager()
{
}

// $Barnyard: FUNCTION 0054b3f0
TBOOL ASimAnimModelHelperManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_iNumHelpers; i++ )
		m_apHelpers[ i ]->OnUpdate( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054b260
TINT ASimAnimModelHelperManager::AddModelHelper( ASimAnimModelHelper* a_pModelHelper )
{
	TASSERT( m_iNumHelpers < MAX_NUM_HELPERS );

	TINT iIndex           = m_iNumHelpers++;
	m_apHelpers[ iIndex ] = a_pModelHelper;

	return iIndex;
}
