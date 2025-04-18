#include "pch.h"
#include "ABarnSign.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054c720
ABarnSign::ABarnSign()
    : m_pModelInstance( TNULL )
    , m_iUnk1( 0 )
    , m_iUnk2( 0 )
    , m_fUnk3( 1.5f )
    , m_fUnk4( 0.1f )
{
}

ABarnSign::~ABarnSign()
{
}

// $Barnyard: FUNCTION 0054d6f0
void ABarnSign::SpawnAtLocator( const TCHAR* a_pszLocatorName )
{
	AAnimatableObjectType* pModelType = AAnimatableObjectManager::GetSingleton()->FindType( TPString8( "barnsign" ) );
	TASSERT( TNULL != pModelType );

	m_pModelInstance = pModelType->Instantiate();

	// Get locator transform
	TMatrix44 matLocator;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_pszLocatorName, matLocator );

	// Update model instance
	m_pModelInstance->SetTransform( matLocator );
	m_pModelInstance->SetSkeletonUpdating( TTRUE );
}
