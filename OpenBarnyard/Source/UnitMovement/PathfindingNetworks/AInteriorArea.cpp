#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054fbe0
void AInteriorArea::GetPosition( Toshi::TVector4& a_rOutPosition ) const
{
	GetPositionXZ( a_rOutPosition );
}

// $Barnyard: FUNCTION 0054fc40
void AInteriorArea::GetPositionXZ( Toshi::TVector4& a_rOutPosition ) const
{
	a_rOutPosition = Toshi::TVector4( m_fOriginX, 0.0f, m_fOriginZ, 1.0f );
	
	m_pNetwork->ResolvePointHeight( m_iIndex, a_rOutPosition, 0 );
}

// $Barnyard: FUNCTION 0054fc90
TBOOL AInteriorArea::CheckPointInsideArea( const Toshi::TVector4& a_rcPoint ) const
{
	// We ignore point height for this check and use Y as Z coordinate on purpose
	const TFLOAT fPointX = a_rcPoint.x;
	const TFLOAT fPointZ = a_rcPoint.y;

	// Fast AABB check
	if ( fPointX <= m_fMinX || fPointX >= m_fMaxX ||
	     fPointZ <= m_fMinZ || fPointZ >= m_fMaxZ )
	{
		return TFALSE;
	}

	if ( m_iNumArcBounds < 3 ) return TFALSE;

	// Detailed check if point is in polygon
	TBOOL                     bInside    = TFALSE;
	const AInteriorArcBounds* pPrevBound = m_pArcBounds + m_iNumArcBounds - 1;
	TBOOL                     bPrevAbove = fPointZ <= pPrevBound->m_fZ;

	for ( TINT i = 0; i < m_iNumArcBounds; i++ )
	{
		const AInteriorArcBounds* pCurrBound = &m_pArcBounds[ i ];
		const TBOOL               bCurrAbove = fPointZ <= pCurrBound->m_fZ;

		if ( bPrevAbove == bCurrAbove )
		{
			if ( pPrevBound->m_fZ == fPointZ && pCurrBound->m_fZ == fPointZ )
			{
				const AInteriorArcBounds* pCheckAgainst = pCurrBound;

				if ( pPrevBound->m_fX <= pCurrBound->m_fX )
				{
					pCheckAgainst = pPrevBound;
				}
				if ( pPrevBound->m_fX < pCurrBound->m_fX )
				{
					pPrevBound = pCurrBound;
				}

				if ( pCheckAgainst->m_fX <= fPointX && fPointX <= pPrevBound->m_fX )
				{
					return TTRUE;
				}
			}
		}
		else 
		{
			TFLOAT fDot = ( ( fPointX - pPrevBound->m_fX ) * pPrevBound->m_fNormalX + ( fPointZ - pPrevBound->m_fZ ) * pPrevBound->m_fNormalZ );

			if ( ( fDot >= 0.0f ) != bCurrAbove )
			{
				bInside = !bInside;
			}
		}

		pPrevBound = pCurrBound;
		bPrevAbove = bCurrAbove;
	}

	return bInside;
}
