#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00550a60
TBOOL AInteriorGoal::UpdateHeight()
{
	if ( ATerrainInterface::GetSingleton() )
	{
		Toshi::TVector4 vecPosition( m_fPositionX, 0.0f, m_fPositionZ, 1.0f );
		m_pNetwork->ResolvePointHeight( m_iAreaIndex, vecPosition, 0 );

		if ( m_iAreaIndex != -1 && m_pNetwork->m_pINetwork->m_pAreas[ m_iAreaIndex ].m_eFlags & 0x20U )
			m_fPositionY = vecPosition.y;
		else if ( vecPosition.y != 0.0 )
			m_fPositionY = vecPosition.y;

		return TTRUE;
	}

	return TFALSE;
	
}

// $Barnyard: FUNCTION 00550b70
TBOOL AInteriorGoal::GetPosition( Toshi::TVector4& a_rOutPosition )
{
	a_rOutPosition = Toshi::TVector4( m_fPositionX, 0.0f, m_fPositionZ, 1.0f );

	if ( m_fPositionY == TMath::MAXFLOAT )
	{
		if ( AInteriorGoal::UpdateHeight() )
		{
			a_rOutPosition.y = m_fPositionY;
		}
		else
		{
			a_rOutPosition.y = 0.0;
		}
	}
	else
	{
		a_rOutPosition.y = m_fPositionY;
	}

	return ( m_fPositionY != TMath::MAXFLOAT );
}
