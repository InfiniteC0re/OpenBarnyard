#include "pch.h"
#include <Render/TCameraObject.h>
#include "ABikeRaceMicroGame.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACameraHelper.h"
#include "Helpers/ABikeHelpers.h"
#include "Locale/ALocaleManager.h"
#include "Render/AShadowManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Physics/ACollisionObjectMesh.h"

#include <File/TFile.h>
#include <Math/TMath.h>
#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( br_gen_cow_anims );
TPSTRING8_DECLARE( br_gen_goat_anims );
TPSTRING8_DECLARE( br_gen_pig_anims );
TPSTRING8_DECLARE( chilliberry );

// The variant with the charity actors next to the course
static constexpr TINT CHARITY_VARIANT = 3;

// Colour of the wrong way popup
static constexpr TUINT32 WRONG_WAY_COLOUR = 0xffff0000;

// How the rivals pick the point they seek to (0x007838f0, never changed from -1: the next waypoint)
static TINT s_iSeekMode = -1;

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// Position of a terrain locator in the world
static TVector4 GetLocatorWorldPosition( const ATerrainLocator* a_pLocator, const TMatrix44& a_rcWorld )
{
	TVector4 vecPosition( a_pLocator->vecPosition.x, a_pLocator->vecPosition.y, a_pLocator->vecPosition.z, 1.0f );
	TMatrix44::TransformVector( vecPosition, a_rcWorld, vecPosition );
	return vecPosition;
}

// $Barnyard: FUNCTION 005274f0
TBOOL ABikeRaceMicroGame::IsAhead( TINT a_iBiker, TINT a_iOther )
{
	const Biker& rBiker = m_aBikers[ a_iBiker ];
	const Biker& rOther = m_aBikers[ a_iOther ];

	if ( rBiker.iLap > rOther.iLap ) return TTRUE;
	if ( rBiker.iLap < rOther.iLap ) return TFALSE;

	if ( rBiker.iWaypoint > rOther.iWaypoint ) return TTRUE;
	if ( rBiker.iWaypoint < rOther.iWaypoint ) return TFALSE;

	// Same segment of the course, compare how far along it they are
	TINT iPrevious = ( rBiker.iWaypoint != 0 ) ? rBiker.iWaypoint - 1 : m_iNumWaypoints - 1;

	const TVector4& rcEnd   = m_aWaypoints[ rBiker.iWaypoint ];
	const TVector4& rcStart = m_aWaypoints[ iPrevious ];

	TFLOAT fBiker = TMath::DistanceAlongSegment( *GetBikerUnit( a_iBiker )->GetSteer()->GetPosition(), rcStart, rcEnd );
	TFLOAT fOther = TMath::DistanceAlongSegment( *GetBikerUnit( a_iOther )->GetSteer()->GetPosition(), rcStart, rcEnd );

	return fOther < fBiker;
}

// $Barnyard: FUNCTION 005275f0
TBOOL ABikeRaceMicroGame::HasCrossedFinishLine( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW )
{
	TMatrix44 matLine;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( "bikefinishline", matLine );

	const TVector4& rcAxis        = matLine.AsBasisVector4( BASISVECTOR_RIGHT );
	const TVector4& rcNormal      = matLine.AsBasisVector4( BASISVECTOR_UP );
	const TVector4& rcTranslation = matLine.AsBasisVector4( BASISVECTOR_TRANSLATION );

	// Must be behind the line
	if ( rcNormal.z * a_fZ + rcNormal.y * a_fY + rcNormal.x * a_fX - TVector4::DotProduct3( rcNormal, rcTranslation ) >= 0.0f )
		return TFALSE;

	// And within its width: 2 to one side and 10 to the other one
	TVector4 vecOffset;
	vecOffset.Multiply( rcAxis, 2.0f );

	TVector4 vecStart = rcTranslation;
	vecStart.x += vecOffset.x;
	vecStart.y += vecOffset.y;
	vecStart.z += vecOffset.z;

	TVector4 vecToPoint( a_fX - vecStart.x, a_fY - vecStart.y, a_fZ - vecStart.z, a_fW );

	TVector4 vecLine;
	vecLine.Multiply( rcAxis, -12.0f );

	TFLOAT fT = TVector4::DotProduct3( vecToPoint, vecLine ) / vecLine.MagnitudeSq();
	return fT > 0.0f && fT < 1.0f;
}

// $Barnyard: FUNCTION 00527810
void ABikeRaceMicroGame::UpdateProgress()
{
	for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
	{
		Biker&  rBiker = m_aBikers[ i ];
		ASteer* pSteer = GetBikerUnit( i )->GetSteer();

		if ( !pSteer->IsA( &TGetClass( ABikeSteer ) ) )
			continue;

		TVector4 vecPos;
		pSteer->CopyPosition( vecPos );

		// Reached a waypoint of the next segment?
		TINT iWaypoint = rBiker.iSegment * SEGMENT_SIZE;
		TINT iEnd      = TMath::Min( m_iNumWaypoints, iWaypoint + SEGMENT_SIZE );

		for ( ; iWaypoint < iEnd; iWaypoint++ )
		{
			const TVector4& rcWaypoint = m_aWaypoints[ iWaypoint ];
			TFLOAT          fZ         = rcWaypoint.z - vecPos.z;
			TFLOAT          fX         = rcWaypoint.x - vecPos.x;

			if ( fZ * fZ + fX * fX < 25.0f )
			{
				rBiker.iSegment++;

				if ( i == 0 && m_aBikers[ 0 ].iWaypoint < m_aBikers[ 0 ].iSegment * SEGMENT_SIZE )
					m_aBikers[ 0 ].iWaypoint = iWaypoint + 1;

				break;
			}
		}

		// The lap is done once the last segment is reached and the finish line crossed
		if ( rBiker.iSegment >= m_iNumSegments - 1 && HasCrossedFinishLine( vecPos.x, vecPos.y, vecPos.z, vecPos.w ) )
		{
			rBiker.iSegment  = 0;
			rBiker.iWaypoint = 0;
			rBiker.iLap++;

			if ( rBiker.iLap >= m_pTrack->iNumLaps )
			{
				if ( i != 0 )
				{
					m_aRivals[ i - 1 ].eState = RIVALSTATE_FINISHED;
					GetBikerSteer( i )->Stop( TFALSE );
				}
				else
				{
					// The player won unless a rival finished first
					m_bGameOver = TTRUE;
					m_bWon      = TTRUE;

					for ( TINT j = 0; j < m_pTrack->iNumRivals; j++ )
					{
						if ( m_aRivals[ j ].eState == RIVALSTATE_FINISHED )
						{
							m_bWon = TFALSE;
							break;
						}
					}

					m_fGameOverTime = 0.0f;
				}
			}
		}
	}

	// Waypoints of the player: moves to the next one once the player is past the current one
	Biker& rPlayer   = m_aBikers[ 0 ];
	TINT   iPrevious = ( rPlayer.iWaypoint != 0 ) ? rPlayer.iWaypoint - 1 : m_iNumWaypoints - 1;

	TVector4 vecToPlayer;
	vecToPlayer.Substract( *GetBikerUnit( 0 )->GetSteer()->GetPosition(), m_aWaypoints[ iPrevious ] );

	const TVector4& rcDir = m_aWaypointDirs[ rPlayer.iWaypoint ];
	TFLOAT          fDot  = rcDir.z * vecToPlayer.z + vecToPlayer.x * rcDir.x;

	if ( fDot > 0.0f && rcDir.MagnitudeSqXZ() <= fDot )
	{
		vecToPlayer.Substract( *GetBikerUnit( 0 )->GetSteer()->GetPosition(), m_aWaypoints[ rPlayer.iWaypoint ] );

		const TVector4& rcNextDir = m_aWaypointDirs[ ( rPlayer.iWaypoint + 1 ) % m_iNumWaypoints ];

		if ( rcNextDir.z * vecToPlayer.z + vecToPlayer.x * rcNextDir.x > 0.0f )
			rPlayer.iWaypoint++;
	}
}

// $Barnyard: FUNCTION 00527bd0
TVector4 ABikeRaceMicroGame::GetSeekTarget( TINT a_iBiker )
{
	const Biker& rBiker = m_aBikers[ a_iBiker ];
	ASteer*      pSteer = GetBikerUnit( a_iBiker )->GetSteer();

	if ( s_iSeekMode == 0 )
	{
		// The next waypoint, skipped once close to it
		TVector4 vecPos    = *pSteer->GetPosition();
		TINT     iWaypoint = rBiker.iWaypoint;

		if ( pSteer->GetSpeed() > 9.7222223f )
		{
			TVector4 vecDiff;
			vecDiff.Substract( m_aWaypoints[ iWaypoint ], vecPos );

			if ( vecDiff.MagnitudeXZ() < 1.0f && ++iWaypoint >= m_iNumWaypoints )
				iWaypoint = 0;
		}

		return m_aWaypoints[ iWaypoint ];
	}
	else if ( s_iSeekMode == 1 )
	{
		// A point ahead on the course, further when faster
		TVector4 vecPos     = *pSteer->GetPosition();
		TFLOAT   fLookAhead = TMath::LERP( 5.0f, 10.0f, TMath::Abs( pSteer->GetSpeed() / 60.0f ) );

		TINT iCurrent  = rBiker.iWaypoint;
		TINT iPrevious = iCurrent - 1;
		TINT iNext     = iCurrent + 1;

		if ( iPrevious < 0 ) iPrevious = m_iNumWaypoints - 1;
		if ( iNext >= m_iNumWaypoints ) iNext = 0;

		TVector4 vecPrevious = m_aWaypoints[ iPrevious ];
		TVector4 vecCurrent  = m_aWaypoints[ iCurrent ];
		TVector4 vecNext     = m_aWaypoints[ iNext ];
		vecPrevious.y        = vecPos.y;
		vecCurrent.y         = vecPos.y;
		vecNext.y            = vecPos.y;

		TFLOAT fAlong = TMath::DistanceAlongSegment( vecPos, vecPrevious, vecCurrent );

		TVector4 vecDiff;
		vecDiff.Substract( vecPrevious, vecCurrent );
		TFLOAT fLength = vecDiff.Magnitude();

		vecDiff.Substract( vecCurrent, vecNext );
		TFLOAT fNextLength = vecDiff.Magnitude();

		TFLOAT   fDistance = fAlong + fLookAhead;
		TVector4 vecTarget;

		if ( fDistance <= fLength )
		{
			TFLOAT fT = fDistance / fLength;
			TMath::Clip( fT, 0.0f, 1.0f );
			vecTarget.Lerp4( vecPrevious, vecCurrent, fT );
		}
		else
		{
			TFLOAT fT = ( fDistance - fLength ) / fNextLength;
			TMath::Clip( fT, 0.0f, 1.0f );
			vecTarget.Lerp4( vecCurrent, vecNext, fT );
		}

		return vecTarget;
	}

	return m_aWaypoints[ rBiker.iWaypoint ];
}

// $Barnyard: FUNCTION 00528080
TINT ABikeRaceMicroGame::FindClosestWaypoint( const TVector4& a_rcPosition )
{
	TFLOAT fClosest = TMath::MAXFLOAT;
	TINT   iClosest = -1;

	for ( TINT i = 0; i < m_iNumWaypoints; i++ )
	{
		TVector4 vecDiff;
		vecDiff.Substract( a_rcPosition, m_aWaypoints[ i ] );

		TFLOAT fDistance = vecDiff.MagnitudeSq();

		if ( fDistance < fClosest )
		{
			fClosest = fDistance;
			iClosest = i;
		}
	}

	return iClosest;
}

// $Barnyard: FUNCTION 00528140
TBOOL ABikeRaceMicroGame::GetRespawnPoint( AUnit* a_pUnit, TVector4& a_rOutPosition, TVector4& a_rOutDirection, TINT a_iWaypoint )
{
	TINT iWaypoint = a_iWaypoint;

	if ( a_iWaypoint == -1 )
	{
		TINT iBiker = -1;

		for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
		{
			if ( GetBikerUnit( i ) == a_pUnit )
			{
				iBiker = i;
				break;
			}
		}

		// The waypoint behind the biker, or the closest one if it's not on the loaded terrain
		iWaypoint = m_aBikers[ iBiker ].iWaypoint - 1;

		if ( iWaypoint < 0 )
			iWaypoint = m_iNumWaypoints - 1;

		if ( !ATerrainInterface::GetSingleton()->FindSectionAtPoint( m_aWaypoints[ iWaypoint ], TTRUE ) )
			iWaypoint = FindClosestWaypoint( *a_pUnit->GetSteer()->GetPosition() );
	}

	a_rOutPosition = m_aWaypoints[ iWaypoint ];

	TINT iNext = iWaypoint + 1;

	if ( iNext >= m_iNumWaypoints )
		iNext = 0;

	const TVector4& rcNext = m_aWaypoints[ iNext ];
	a_rOutDirection.x      = rcNext.x - a_rOutPosition.x;
	a_rOutDirection.y      = rcNext.y - a_rOutPosition.y;
	a_rOutDirection.z      = rcNext.z - a_rOutPosition.z;
	a_rOutDirection.w      = rcNext.w;
	a_rOutDirection.Normalise();

	return ABikeHelpers::AlignToGround( a_rOutPosition, a_rOutDirection, 40.0f, 80.0f );
}

// $Barnyard: FUNCTION 00528e90
void ABikeRaceMicroGame::LoadWaypoints()
{
	m_iNumWaypoints = 0;

	TCHAR szName[ 8 ];
	TStringManager::String8Format( szName, sizeof( szName ), "wp_%.3i", 0 );

	ATerrainLocator* pLocator = ATerrainInterface::GetSingleton()->FindLocator( szName );

	if ( pLocator )
	{
		// The waypoints are locators of the terrain
		while ( pLocator && m_iNumWaypoints < 255 )
		{
			// NOTE: retail copies the matrix of the transform object directly
			TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, ATerrainInterface::GetSingleton()->GetLocatorList()->GetWorldTransform() );

			m_aWaypoints[ m_iNumWaypoints ] = GetLocatorWorldPosition( pLocator, matWorld );
			// NOTE: retail drops the height, AlignToGround puts respawned bikes back on the ground (when the terrain is loaded)
			m_aWaypoints[ m_iNumWaypoints ].y = 0.0f;
			m_iNumWaypoints++;

			TStringManager::String8Format( szName, sizeof( szName ), "wp_%.3i", TUINT8( m_iNumWaypoints ) );
			pLocator = ATerrainInterface::GetSingleton()->FindLocator( szName );
		}
	}
	else
	{
		// The waypoints are goals of the network of the race
		TPString8 strName = TPS8D( szName );

		while ( m_pNetwork->HasGoal( strName ) && m_iNumWaypoints < 255 )
		{
			m_pNetwork->FindGoal( strName )->GetPosition( m_aWaypoints[ m_iNumWaypoints ] );
			m_aWaypoints[ m_iNumWaypoints ].y = 0.0f;
			m_iNumWaypoints++;

			TStringManager::String8Format( szName, sizeof( szName ), "wp_%.3i", TUINT8( m_iNumWaypoints ) );
			strName = TPS8D( szName );
		}
	}

	m_iNumSegments = m_iNumWaypoints / SEGMENT_SIZE + 1;

	// The course is a loop
	m_aWaypoints[ m_iNumWaypoints ] = m_aWaypoints[ 0 ];

	for ( TINT i = 1; i <= m_iNumWaypoints; i++ )
	{
		m_aWaypointDirs[ i ].x = m_aWaypoints[ i ].x - m_aWaypoints[ i - 1 ].x;
		m_aWaypointDirs[ i ].y = m_aWaypoints[ i ].y - m_aWaypoints[ i - 1 ].y;
		m_aWaypointDirs[ i ].z = m_aWaypoints[ i ].z - m_aWaypoints[ i - 1 ].z;
		m_aWaypointDirs[ i ].w = m_aWaypoints[ i ].w;
	}

	m_aWaypointDirs[ 0 ] = m_aWaypointDirs[ m_iNumWaypoints ];

	// Find the places where the course crosses itself (bridges), going the wrong way there is fine
	for ( TINT i = 0; i < m_iNumWaypoints; i++ )
		m_aiCrossings[ i ] = -1;

	for ( TINT i = 0; i < m_iNumWaypoints; i++ )
	{
		if ( m_aiCrossings[ i ] != -1 )
			continue;

		TINT iPrevious = i - 1;

		if ( iPrevious < 0 )
			iPrevious = m_iNumWaypoints - 1;

		TVector2 vecStart( m_aWaypoints[ iPrevious ].x, m_aWaypoints[ iPrevious ].z );
		TVector2 vecEnd( m_aWaypoints[ i ].x, m_aWaypoints[ i ].z );

		for ( TINT k = i + 2; k < m_iNumWaypoints; k++ )
		{
			if ( m_aiCrossings[ k ] != -1 || k == iPrevious )
				continue;

			TVector2 vecOtherStart( m_aWaypoints[ k - 1 ].x, m_aWaypoints[ k - 1 ].z );
			TVector2 vecOtherEnd( m_aWaypoints[ k ].x, m_aWaypoints[ k ].z );

			if ( TMath::SegmentVsSegment2D( vecStart, vecEnd, vecOtherStart, vecOtherEnd ) )
			{
				m_aiCrossings[ i ] = k;
				m_aiCrossings[ k ] = i;
				break;
			}
		}
	}
}

// $Barnyard: FUNCTION 00529710
void ABikeRaceMicroGame::SetupTrack()
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	// The race has its own locators
	TStringManager::String8Format( m_szSkeleton, sizeof( m_szSkeleton ), "Data\\Terrain\\%s\\%s_skel.trb", m_pTrack->szName, m_pTrack->szName );

	TFile* pFile = TFile::Create( TString8( m_szSkeleton ), TFILEMODE_READ );

	if ( pFile )
	{
		pFile->Destroy();
		m_szOldSkeleton = pTerrain->GetSkeletonFilePath();
		pTerrain->ReloadSkeleton( m_szSkeleton );
	}

	// The gate of the veggie patch
	if ( pTerrain->FindLocator( "fncveggate" ) )
	{
		m_pGateModel = AModelRepos::GetSingleton()->InstantiateNewModel( TPS8D( "data\\models\\fncveggate" ), TNULL );

		TMatrix44 matGate;
		pTerrain->GetLocatorTransform( "fncveggate", matGate );
		m_pGateModel->SetTransform( matGate );
		m_pGateModel->SetSkeletonUpdating( TTRUE );

		m_pGateCollision = new ACollisionObjectMesh();
		m_pGateCollision->Create( m_pGateModel->GetSceneObject() );
	}
	else
	{
		m_pGateModel     = TNULL;
		m_pGateCollision = TNULL;
	}

	LoadWaypoints();

	// Put the bikers at the start, facing the second waypoint
	for ( TINT i = 0; i <= m_pTrack->iNumRivals; i++ )
	{
		TCHAR szGoal[ 16 ];
		TStringManager::String8Format( szGoal, sizeof( szGoal ), "player%dstart", i );

		TVector4 vecStart;
		m_pNetwork->FindGoalOrFirst( szGoal )->GetPosition( vecStart );

		ABikeSteer* pSteer = GetBikerSteer( i );
		pSteer->SetPosition( vecStart );

		const TVector4& rcFace = m_aWaypoints[ 1 ];
		pSteer->FacePosition( rcFace.x, rcFace.y, rcFace.z, rcFace.w );
	}

	ACameraManager::GetSingleton()->GetCurrentCameraHelper()->ResetPosition( TTRUE );

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
		GetBikerSteer( i )->SetSeekTarget( GetSeekTarget( i ) );

	// The pick ups and the jumps
	m_iNumJumps      = 0;
	m_iNumCoins      = 0;
	m_iNumSpectators = 0;
	m_iNumBerries    = 0;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pCoinType      = pObjectManager->FindType( TPS8( barnbuck ) );

	// NOTE: the spectators were cut, their types are looked up but not used
	pObjectManager->FindType( TPS8( br_gen_cow_anims ) );
	pObjectManager->FindType( TPS8( br_gen_goat_anims ) );
	pObjectManager->FindType( TPS8( br_gen_pig_anims ) );

	AAnimatableObjectType* pBerryType = pObjectManager->FindType( TPS8( chilliberry ) );

	ATerrainLocatorList* pLocatorList = pTerrain->GetLocatorList();

	// NOTE: retail copies the matrix of the transform object directly
	TMatrix44 matWorld = TREINTERPRETCAST( const TMatrix44&, pLocatorList->GetWorldTransform() );

	for ( TINT i = 0; i < pLocatorList->GetNumLocators(); i++ )
	{
		ATerrainLocator* pLocator = const_cast<ATerrainLocator*>( pLocatorList->GetLocator( i ) );
		const TCHAR*     szName   = pLocatorList->GetLocatorName( i );

		if ( T2String8::Compare( szName, "bikejump1", 9 ) == 0 || T2String8::Compare( szName, "emojump_01", 10 ) == 0 )
		{
			m_apJumps[ m_iNumJumps++ ] = pLocator;
		}
		else if ( T2String8::Compare( szName, "coin", 4 ) == 0 )
		{
			if ( T2String8::Compare( szName, "coin_04", 7 ) != 0 )
			{
				// A barnbuck put on the ground
				Coin& rCoin   = m_aCoins[ m_iNumCoins ];
				rCoin.pObject = pObjectManager->CreateAnimatableObject( pCoinType, TNULL, 0 );
				rCoin.pObject->SetSkeletonUpdating( TTRUE, TTRUE );

				TVector4 vecPosition = GetLocatorWorldPosition( pLocator, matWorld );
				rCoin.pObject->GetModelInstance()->SetTransform( vecPosition, TVector4::VEC_POSX );
				rCoin.iState = COINSTATE_NOT_GROUNDED;

				AShadowManager::GetSingleton()->AddModelShadow( rCoin.pObject->GetModelInstance(), 0.5f, -1, -0.1f, AShadowManager::FLAGS( 32 ), AShadowManager::TYPE( 0 ) );
				AShadowManager::GetSingleton()->SetShadowStatic( rCoin.pObject->GetModelInstance(), TTRUE );
				m_iNumCoins++;
			}
			else
			{
				// A chilliberry
				Berry& rBerry  = m_aBerries[ m_iNumBerries ];
				rBerry.pObject = pObjectManager->CreateAnimatableObject( pBerryType, TNULL, 0 );
				rBerry.pObject->SetSkeletonUpdating( TTRUE, TTRUE );

				TVector4 vecPosition = GetLocatorWorldPosition( pLocator, matWorld );
				rBerry.pObject->GetModelInstance()->SetTransform( vecPosition, TVector4::VEC_POSX );
				rBerry.fTimer    = -1.0f;
				rBerry.vPosition = vecPosition;
				rBerry.pObject->DestroyAnimControllers();

				AShadowManager::GetSingleton()->AddModelShadow( rBerry.pObject->GetModelInstance(), 0.5f, -1, -0.1f, AShadowManager::FLAGS( 32 ), AShadowManager::TYPE( 0 ) );
				AShadowManager::GetSingleton()->SetShadowStatic( rBerry.pObject->GetModelInstance(), TTRUE );
				m_iNumBerries++;
			}
		}
		else if ( T2String8::Compare( szName, "aircoin", 7 ) == 0 )
		{
			// A barnbuck in the air
			Coin& rCoin   = m_aCoins[ m_iNumCoins ];
			rCoin.pObject = pObjectManager->CreateAnimatableObject( pCoinType, TNULL, 0 );
			rCoin.pObject->SetSkeletonUpdating( TTRUE, TTRUE );

			TVector4 vecPosition = GetLocatorWorldPosition( pLocator, matWorld );
			rCoin.pObject->GetModelInstance()->SetTransform( vecPosition, TVector4::VEC_POSX );
			rCoin.iState = COINSTATE_AIR;
			m_iNumCoins++;
		}
	}

	m_fPickupTime = 0.0f;

	// Everet and Daisy stand next to the start
	if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == CHARITY_VARIANT )
	{
		TVector4 vecEveretPos( 82.0f, -8.0f, 180.0f, 1.0f );
		vecEveretPos.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vecEveretPos, 0 ) - 0.7f;

		const TVector4& rcPlayerPos = *GetBikerUnit( 0 )->GetSteer()->GetPosition();

		TVector4 vecEveretDir( rcPlayerPos.x - vecEveretPos.x, 0.0f, rcPlayerPos.z - vecEveretPos.z, rcPlayerPos.w );
		vecEveretDir.Normalise();

		TVector4 vecDaisyPos( 83.0f, -8.0f, 179.5f, 1.0f );
		vecDaisyPos.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vecDaisyPos, 0 );

		TVector4 vecDaisyDir( rcPlayerPos.x - vecDaisyPos.x, 0.0f, rcPlayerPos.z - vecDaisyPos.z, rcPlayerPos.w );
		vecDaisyDir.Normalise();

		PlaceCharityActors( vecEveretPos, vecDaisyPos, vecEveretDir, vecDaisyDir );
	}
}

// $Barnyard: FUNCTION 0052a3c0
void ABikeRaceMicroGame::UpdateHUD( TFLOAT a_fDeltaTime )
{
	m_oMiniMap.SetPlayer( m_aBikers[ 0 ].pSlot->m_pUnitProxy );
	m_oWrongWayPopup.Update( a_fDeltaTime );

	if ( IsGameOver() )
		return;

	// The placing of the player
	TINT iPlacing = 0;

	for ( TINT i = 1; i <= m_pTrack->iNumRivals; i++ )
	{
		if ( !IsAhead( 0, i ) )
			iPlacing++;
	}

	m_oPlacingHud.SetPlacing( iPlacing );

	// The lap
	TINT iLap = m_aBikers[ 0 ].iLap + 1;
	TMath::Clip( iLap, 1, m_pTrack->iNumLaps );

	TStringManager::String16Format( m_wszLapText, TARRAYSIZE( m_wszLapText ), ALocaleManager::Interface()->GetString( LOCALE_LAP ), iLap, m_pTrack->iNumLaps );
	m_oLapText.SetTextAndFitWidth( m_wszLapText );

	// Going the wrong way? Find the part of the course the player is on
	ASteer*  pSteer       = GetBikerUnit( 0 )->GetSteer();
	TVector4 vecDirection = *pSteer->GetDirection();
	TVector4 vecPos       = *pSteer->GetPosition();

	TFLOAT fT;
	TFLOAT fClosest = TMath::DistancePointToSegmentSq( vecPos, m_aWaypoints[ m_iNumWaypoints - 1 ], m_aWaypoints[ 0 ], fT );
	TINT   iClosest = 0;

	for ( TINT j = 0; j < m_iNumWaypoints - 1; j++ )
	{
		TFLOAT fDistance = TMath::DistancePointToSegmentSq( vecPos, m_aWaypoints[ j ], m_aWaypoints[ j + 1 ], fT );

		if ( fDistance < fClosest )
		{
			fClosest = fDistance;
			iClosest = j + 1;
		}
	}

	TVector4 vecCourseDir = m_aWaypointDirs[ iClosest ];
	NormaliseXZ( vecCourseDir );

	if ( vecCourseDir.GetAngleBetweenVector( vecDirection ) <= TMath::PI * 0.5f || m_aiCrossings[ iClosest ] != -1 )
	{
		m_fWrongWayTime                = 0.0f;
		m_oWrongWayPopup.m_fPulseSpeed = 1.0f;
		m_oWrongWayPopup.m_fStartScale = 1.0f;
		m_oWrongWayPopup.m_fDuration   = 1.0f;
		m_oWrongWayPopup.m_fScale      = 1.2f;
		m_oWrongWayPopup.m_eMode       = AGUI2AnythingPopup::MODE_FADE;
		m_oWrongWayPopup.SetText( TNULL, WRONG_WAY_COLOUR, TNULL );
		return;
	}

	m_fWrongWayTime += a_fDeltaTime;

	if ( m_fWrongWayTime > 2.0f )
	{
		const TWCHAR* wszText = ALocaleManager::Interface()->GetString( LOCALE_WRONG_WAY );

		if ( !wszText || !m_oWrongWayPopup.m_wszPopupText )
		{
			m_oWrongWayPopup.m_fPulseSpeed = 1.0f;
			m_oWrongWayPopup.m_fStartScale = 1.0f;
			m_oWrongWayPopup.m_fDuration   = 1.0f;
			m_oWrongWayPopup.m_fScale      = 1.2f;
			m_oWrongWayPopup.m_eMode       = AGUI2AnythingPopup::MODE_FADE;
			m_oWrongWayPopup.SetText( wszText, WRONG_WAY_COLOUR, TNULL );
		}
	}
}
