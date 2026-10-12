#include "pch.h"
#include "AChasingChicksGame.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0050d630
AUnit* AChasingChicksGame::FindPlayerNear( const TVector4& a_rcPosition, TFLOAT& a_rfRadius, TINT& a_riPlayer )
{
	a_riPlayer = -1;

	AUnit* pFound  = TNULL;
	TFLOAT fBestSq = a_rfRadius * a_rfRadius;

	for ( TINT i = 0; i < iNumPlayers; i++ )
	{
		AUnit* pUnit = aPlayers[ i ].oUnit->GetUnit();

		if ( pUnit )
		{
			TVector4 vToPlayer;
			pUnit->GetSteer()->CopyPosition( vToPlayer );
			vToPlayer.x -= a_rcPosition.x;
			vToPlayer.y -= a_rcPosition.y;
			vToPlayer.z -= a_rcPosition.z;

			TFLOAT fDistanceSq = vToPlayer.MagnitudeSq();

			// NOTE: retail compares the squared distance with the unsquared radius
			// (so the chick's 64.0 means 8 m) and not with the best distance so far,
			// so the last player in range wins instead of the nearest one.
			if ( fDistanceSq < a_rfRadius )
			{
				fBestSq    = fDistanceSq;
				a_riPlayer = i;
				pFound     = pUnit;
			}
		}
	}

	if ( a_riPlayer != -1 )
		a_rfRadius = TMath::Sqrt( fBestSq );

	return pFound;
}

// $Barnyard: FUNCTION 0050d780
AChasingChicksHotSpot* AChasingChicksGame::FindClosestPen( const TVector4& a_rcPosition, TFLOAT a_fMaxDistance )
{
	TFLOAT                 fBestSq = a_fMaxDistance * a_fMaxDistance;
	AChasingChicksHotSpot* pBest   = TNULL;

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		T2_FOREACH( aOpenPens[ i ], it )
		{
			TFLOAT fDZ       = a_rcPosition.z - it->vPosition.z;
			TFLOAT fDX       = a_rcPosition.x - it->vPosition.x;
			TFLOAT fDistance = fDZ * fDZ + fDX * fDX;

			if ( fDistance < fBestSq )
			{
				fBestSq = fDistance;
				pBest   = it;
			}
		}
	}

	return pBest;
}

// $Barnyard: FUNCTION 0050d4a0
AChasingChicksHotSpot* AChasingChicksGame::PickPenOfBestKind()
{
	TINT iKind = 0;

	if ( aOpenPens[ 0 ].IsEmpty() )
	{
		for ( iKind = 3; aOpenPens[ iKind ].IsEmpty(); )
		{
			if ( --iKind <= 0 )
				return TNULL;
		}
	}

	if ( ARandom::GetSingleton()->m_oRandom.GetInt( 2 ) != 0 )
		return aOpenPens[ iKind ].Front();
	else
		return aOpenPens[ iKind ].Back();
}

// $Barnyard: FUNCTION 0050d720
AChasingChicksHotSpot* AChasingChicksGame::PickRandomPen()
{
	T2DList<AChasingChicksHotSpot>& rList = aOpenPens[ ARandom::GetSingleton()->m_oRandom.GetInt( 4 ) ];
	TINT                            iPen  = ARandom::GetSingleton()->m_oRandom.GetInt( rList.Size() );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: when the list is empty, retail returns its root (not a pen) and the AI
	// then walks to garbage; the callers check the result against TNULL only
	if ( rList.IsEmpty() )
		return TNULL;
#endif

	auto it = rList.Begin();

	for ( ; iPen > 0; iPen-- )
		it++;

	return it;
}
