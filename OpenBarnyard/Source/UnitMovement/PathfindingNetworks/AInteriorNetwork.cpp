#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"
#include "Units/AUnit.h"
#include "Units/AKinematicsObstruction.h"
#include "Assets/AAssetLoader.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00550ec0, CreateObject 00553c20, CreateObjectInPlace 00553c80
TDEFINE_CLASS( AInteriorNetwork );

// 0x00788a78
Toshi::TDList<AInteriorNetwork::AreaSearchNode> AInteriorNetwork::ms_oFreeAreaSearchNodes;

// $Barnyard: FUNCTION 00553100
AInteriorNetwork::AInteriorNetwork()
{
	m_pTRB              = TNULL;
	m_pINetwork         = TNULL;
	m_strName           = Toshi::TPString8();
	m_bSetHeightFromEnv = TTRUE;

	m_bIsPersistent = TFALSE;

	m_iMaxGroup = 0;

	m_pAreaNameLookup = new Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>();
	m_pGoalNameLookup = new Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>();
}

// $Barnyard: FUNCTION 005551f0
// $Barnyard: FUNCTION 00554b70
AInteriorNetwork::~AInteriorNetwork()
{
	TINT iNumGroups = m_iMaxGroup;
	while ( iNumGroups > 0 )
	{
		m_vecGroups[ iNumGroups - 1 ].Clear();
		iNumGroups -= 1;
	}

	ClearAreaNameLookup();
	ClearGoalNameLookup();

	if ( m_pUpdateGoalInterfaces )
		delete[] m_pUpdateGoalInterfaces;

	// HACK: by default TRB stores raw strings, but we convert them to TPString8 for better performance, so need to destruct those manually
	{
		for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
			delete m_pINetwork->m_pAreas[ i ].m_strName;

		for ( TINT i = 0; i < m_pINetwork->m_iNumGoals; i++ )
		{
			delete m_pINetwork->m_pGoals[ i ].m_strName;
			delete m_pINetwork->m_pGoals[ i ].m_strSecondaryName;
		}
	}

	if ( m_pTRB )
	{
		m_pTRB->Close();
		delete m_pTRB;
		m_pTRB = TNULL;
	}

	if ( m_pAreaNameLookup ) delete m_pAreaNameLookup;
	if ( m_pGoalNameLookup ) delete m_pGoalNameLookup;
}

// $Barnyard: FUNCTION 005518d0
static void* __stdcall NetworkTRBAlloc( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	return GetGlobalAllocator()->Malloc( a_uiSize );
}

// $Barnyard: FUNCTION 005518f0
static void __stdcall NetworkTRBFree( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	GetGlobalAllocator()->Free( a_pMem );
}

// $Barnyard: FUNCTION 00554d90
TBOOL AInteriorNetwork::LoadFromFile( const Toshi::TPString8& a_rcNetworkName, const TCHAR* a_szFilePath )
{
	m_strName = a_rcNetworkName;

	// Try to get the network from the already loaded assets first
	m_pINetwork = AAssetLoader::CastSymbol<INetwork>( a_rcNetworkName.GetString(), "INetwork", AAssetType_AssetPack );

	if ( !m_pINetwork )
	{
		m_pTRB = new TTRB();
		m_pTRB->SetMemoryFunctions( NetworkTRBAlloc, NetworkTRBFree, TNULL );

		if ( m_pTRB->Load( a_szFilePath ) != TTRB::ERROR_OK )
			return TFALSE;

		m_pINetwork = m_pTRB->CastSymbol<INetwork>( "INetwork" );
		m_pTRB->DeleteSymbolTable();
	}

	if ( m_pINetwork )
	{
		// HACK: replace the raw names with the pooled strings (see AInteriorArea and AInteriorGoal)
		for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
		{
			AInteriorArea* pArea = &m_pINetwork->m_pAreas[ i ];
			pArea->m_pNetwork    = this;

			TPString8* pName = new TPString8();
			*pName           = TPString8( pArea->m_szName_Internal );
			pArea->m_strName = pName;
		}

		m_pUpdateGoalInterfaces = new AGoalCollisionUpdateGoalInterface[ m_pINetwork->m_iNumGoals ];

		TINT iNumInterfaces = 0;
		for ( TINT i = 0; i < m_pINetwork->m_iNumGoals; i++ )
		{
			AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ i ];
			pGoal->m_pNetwork    = this;

			TPString8* pName = new TPString8();
			*pName           = TPString8( pGoal->m_szName_Internal );
			pGoal->m_strName = pName;

			TPString8* pSecondaryName = new TPString8();
			*pSecondaryName           = TPString8( pGoal->m_szSecondaryName_Internal );
			pGoal->m_strSecondaryName = pSecondaryName;

			// Goals whose height can't be resolved yet will be updated once their collision group is ready
			if ( !pGoal->UpdateHeight() )
				PushGoalInterface( &m_pUpdateGoalInterfaces[ iNumInterfaces++ ], pGoal );
		}

		BuildAreaNameLookup();
		BuildGoalNameLookup();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00551d90
void AInteriorNetwork::ResolvePointHeight( TINT a_iAreaIndex, Toshi::TVector4& a_rPoint, TUINT a_uiFlags )
{
	TCollisionCommon::TOSHICGROUPFLAG eIgnoreGroups = TCollisionCommon::TOSHICGROUPFLAG( a_uiFlags );

	if ( a_iAreaIndex == -1 )
	{
		// ATerrainInterface::SetToGroundHeightBottom (thunk)
		a_rPoint.y = ATerrainInterface::GetSingleton()->GetGroundHeightBottom( a_rPoint.x, a_rPoint.z, eIgnoreGroups );
	}
	else if ( m_bSetHeightFromEnv )
	{
		const AInteriorArea& rcArea = m_pINetwork->m_pAreas[ a_iAreaIndex ];

		if ( HASANYFLAG( rcArea.m_eFlags, AInteriorArea::FLAGS_HEIGHT_RAYCAST_SHORT ) )
		{
			// Cast a ray down from just above the area
			ARayData oRay( TVector4( a_rPoint.x, -rcArea.m_fBaseHeight - 0.25f, a_rPoint.z, 1.0f ), TVector4::VEC_POSY, 40.0f );
			a_rPoint.y = 40.0f;
			ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &a_rPoint.y, eIgnoreGroups );

			if ( a_rPoint.y < 39.0f )
			{
				a_rPoint.y = oRay.vOrigin.y + a_rPoint.y;
			}
			else
			{
				// ATerrainInterface::SetToGroundHeightBottom (thunk)
				a_rPoint.y = ATerrainInterface::GetSingleton()->GetGroundHeightBottom( a_rPoint.x, a_rPoint.z, eIgnoreGroups );

				if ( a_rPoint.y <= -39.0f )
					a_rPoint.y = 0.0f;
			}
		}
		else if ( HASANYFLAG( rcArea.m_eFlags, AInteriorArea::FLAGS_HEIGHT_RAYCAST_LONG ) )
		{
			// Cast a ray down against everything
			ARayData oRay( TVector4( a_rPoint.x, -rcArea.m_fBaseHeight - 0.25f, a_rPoint.z, 1.0f ), TVector4::VEC_POSY, 100.0f );
			a_rPoint.y = 100.0f;
			ACollisionManager::GetSingleton()->CollideRay( TNULL, oRay, a_rPoint.y, eIgnoreGroups, ACollisionObject::TYPE_UNIT );

			if ( a_rPoint.y < 99.0f )
			{
				a_rPoint.y = oRay.vOrigin.y + a_rPoint.y;
			}
			else
			{
				// ATerrainInterface::SetToGroundHeightBottom (thunk)
				a_rPoint.y = ATerrainInterface::GetSingleton()->GetGroundHeightBottom( a_rPoint.x, a_rPoint.z, eIgnoreGroups );

				if ( a_rPoint.y <= -99.0f )
					a_rPoint.y = 0.0f;
			}
		}
		else if ( HASANYFLAG( rcArea.m_eFlags, AInteriorArea::FLAGS_HEIGHT_FROM_TERRAIN ) )
		{
			// ATerrainInterface::SetToGroundHeightBottom (thunk)
			a_rPoint.y = ATerrainInterface::GetSingleton()->GetGroundHeightBottom( a_rPoint.x, a_rPoint.z, eIgnoreGroups );
		}
		else
		{
			a_rPoint.y = -rcArea.m_fBaseHeight;
		}
	}
	else
	{
		// ATerrainInterface::GetGroundHeightTop (thunk)
		TFLOAT fHeight = ATerrainInterface::GetSingleton()->GetGroundHeightTop( a_rPoint.x, a_rPoint.z, eIgnoreGroups );

		if ( fHeight < a_rPoint.y )
			a_rPoint.y = fHeight;
	}
}

// $Barnyard: FUNCTION 005522c0
TINT AInteriorNetwork::FindAreaIndex( const Toshi::TPString8& a_rcName ) const
{
	auto it = m_pAreaNameLookup->Find( a_rcName );

	return ( it != m_pAreaNameLookup->End() ) ? it->second.pIndices[ 0 ] : -1;
}

// $Barnyard: FUNCTION 00552bc0
AInteriorGoal* AInteriorNetwork::FindGoal( const Toshi::TPString8& a_rcName ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
		return &m_pINetwork->m_pGoals[ it->second.pIndices[ 0 ] ];

	TERROR( "Couldn't find goal %s\n", a_rcName.GetString() );
	TASSERT( TFALSE );
	return &m_pINetwork->m_pGoals[ 0 ];
}

// $Barnyard: FUNCTION 00554280
TINT AInteriorNetwork::CountGoalsContaining( const TCHAR* a_szSubString ) const
{
	TINT iCount = 0;

	for ( auto it = m_pGoalNameLookup->Begin(); it != m_pGoalNameLookup->End(); it++ )
	{
		TPString8 strName = it->first;

		if ( T2String8::FindString( strName.GetString(), a_szSubString ) )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 00552c20
AInteriorGoal* AInteriorNetwork::FindGoalInArea( const Toshi::TPString8& a_rcName, TINT a_iAreaIndex /*= -1*/ ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
	{
		for ( TINT i = 0; i < it->second.iCount; i++ )
		{
			AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ it->second.pIndices[ i ] ];

			if ( pGoal->m_iAreaIndex == a_iAreaIndex || a_iAreaIndex < 0 )
				return pGoal;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00552ca0
TBOOL AInteriorNetwork::HasGoal( const Toshi::TPString8& a_rcName ) const
{
	return m_pGoalNameLookup->Find( a_rcName ) != m_pGoalNameLookup->End();
}

// $Barnyard: FUNCTION 00552ce0
TINT AInteriorNetwork::GetNumGoals( const Toshi::TPString8& a_rcName ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
		return it->second.iCount;

	return 0;
}

// $Barnyard: FUNCTION 00553780
AInteriorGoal* AInteriorNetwork::FindGoalOrFirst( const TCHAR* a_szName ) const
{
	auto it = m_pGoalNameLookup->Find( Toshi::TPString8( a_szName ) );

	if ( it != m_pGoalNameLookup->End() )
		return &m_pINetwork->m_pGoals[ it->second.pIndices[ 0 ] ];

	return &m_pINetwork->m_pGoals[ 0 ];
}

// $Barnyard: FUNCTION 00552e90
AInteriorGoal* AInteriorNetwork::FindRandomGoal( const Toshi::TPString8& a_rcName ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
		return &m_pINetwork->m_pGoals[ it->second[ ARandom::GetSingleton()->m_oRandom.GetInt( it->second.iCount ) ] ];

	return TNULL;
}

// Returns the obstruction closest to a_rcPosition (on the XZ plane) that is closer than a_fRadius, TNULL if there's none
// NOTE: a method of the obstruction list in retail
// $Barnyard: FUNCTION 00573520
static AKinematicsObstruction* FindClosestObstruction( const Toshi::T2Vector<AKinematicsObstruction*, 82>& a_rcObstructions, const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius )
{
	AKinematicsObstruction* pClosest       = TNULL;
	TFLOAT                  fMinDistanceSq = a_fRadius * a_fRadius;

	for ( TINT i = 0; i < a_rcObstructions.Size(); i++ )
	{
		AKinematicsObstruction* pObstruction = a_rcObstructions.At( i );
		const TVector4*         pPosition    = pObstruction->GetPosition();

		TFLOAT fDistanceSq = ( a_rcPosition.z - pPosition->z ) * ( a_rcPosition.z - pPosition->z ) + ( a_rcPosition.x - pPosition->x ) * ( a_rcPosition.x - pPosition->x );

		if ( fMinDistanceSq > fDistanceSq )
		{
			fMinDistanceSq = fDistanceSq;
			pClosest       = pObstruction;
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 00552f00
AInteriorGoal* AInteriorNetwork::FindRandomVisibleGoal( const Toshi::TPString8& a_rcName ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() && it->second.iCount > 0 )
	{
		AInteriorGoal* apFreeGoals[ 50 ];
		TUINT          uiNumFreeGoals = 0;

		for ( TINT i = 0; i < it->second.iCount; i++ )
		{
			AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ it->second[ i ] ];

			TVector4 vecGoalPosition;
			pGoal->GetPosition( vecGoalPosition );

			// Skip goals that have an obstruction closer than 0.3 units to them
			if ( !FindClosestObstruction( m_vecObstructions, vecGoalPosition, 0.3f ) )
				apFreeGoals[ uiNumFreeGoals++ ] = pGoal;
		}

		if ( uiNumFreeGoals != 0 )
			return apFreeGoals[ ARandom::GetSingleton()->m_oRandom.GetInt( uiNumFreeGoals ) ];
	}

	return FindRandomGoal( a_rcName );
}

// $Barnyard: FUNCTION 00552fe0
AInteriorGoal* AInteriorNetwork::FindFarthestGoal( const Toshi::TPString8& a_rcName, const Toshi::TVector4& a_rcPosition ) const
{
	AInteriorGoal* pBestGoal = TNULL;
	TFLOAT         fBestDist = -1.0f;

	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
	{
		for ( TINT i = 0; i < it->second.iCount; i++ )
		{
			AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ it->second[ i ] ];

			Toshi::TVector4 vecGoalPosition;
			pGoal->GetPosition( vecGoalPosition );
			TFLOAT fDist = Toshi::TVector4::Distance( vecGoalPosition, a_rcPosition );

			if ( fBestDist < fDist )
			{
				fBestDist = fDist;
				pBestGoal = pGoal;
			}
		}
	}

	return pBestGoal ? pBestGoal : FindRandomGoal( a_rcName );
}


// $Barnyard: FUNCTION 00551a70
AInteriorGoal* AInteriorNetwork::FindClosestGoal( const TCHAR* a_szName, const Toshi::TVector4& a_rcPosition ) const
{
	AInteriorGoal* pBestGoal   = TNULL;
	TFLOAT         fBestDistSq = Toshi::TMath::MAXFLOAT;

	// NOTE: retail goes through every goal (from the last one) comparing the names, ignoring the case
	for ( TINT i = m_pINetwork->m_iNumGoals - 1; i >= 0; i-- )
	{
		AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ i ];

		if ( pGoal->m_strName->GetString8().CompareNoCase( a_szName ) == 0 )
		{
			Toshi::TVector4 vecGoalPosition;
			pGoal->GetPosition( vecGoalPosition );
			TFLOAT fDistSq = Toshi::TVector4::DistanceSq( a_rcPosition, vecGoalPosition );

			if ( fDistSq < fBestDistSq )
			{
				fBestDistSq = fDistSq;
				pBestGoal   = pGoal;
			}
		}
	}

	return pBestGoal;
}

// $Barnyard: FUNCTION 00551b20
AInteriorGoal* AInteriorNetwork::FindClosestGoalOrFirst( const TCHAR* a_szName, const Toshi::TVector4& a_rcPosition ) const
{
	AInteriorGoal* pGoal = FindClosestGoal( a_szName, a_rcPosition );
	return pGoal ? pGoal : &m_pINetwork->m_pGoals[ 0 ];
}

// $Barnyard: FUNCTION 00554350
AInteriorGoal* AInteriorNetwork::FindClosestGoalContaining( const TCHAR* a_szSubString, const Toshi::TVector4& a_rcPosition ) const
{
	AInteriorGoal* pBestGoal   = TNULL;
	TFLOAT         fBestDistSq = Toshi::TMath::MAXFLOAT;

	if ( m_pGoalNameLookup->Begin() == m_pGoalNameLookup->End() )
		return &m_pINetwork->m_pGoals[ 0 ];

	for ( auto it = m_pGoalNameLookup->Begin(); it != m_pGoalNameLookup->End(); it++ )
	{
		TPString8 strName = it->first;

		if ( strName.GetString8().Find( a_szSubString, 0 ) != -1 )
		{
			auto           itGoal = m_pGoalNameLookup->Find( strName );
			AInteriorGoal* pGoal  = ( itGoal == m_pGoalNameLookup->End() ) ? &m_pINetwork->m_pGoals[ 0 ] : &m_pINetwork->m_pGoals[ itGoal->second[ 0 ] ];

			Toshi::TVector4 vecGoalPosition;
			pGoal->GetPosition( vecGoalPosition );
			TFLOAT fDistSq = Toshi::TVector4::DistanceSq( a_rcPosition, vecGoalPosition );

			if ( fDistSq < fBestDistSq )
			{
				fBestDistSq = fDistSq;
				pBestGoal   = pGoal;
			}
		}
	}

	return pBestGoal ? pBestGoal : &m_pINetwork->m_pGoals[ 0 ];
}

// $Barnyard: FUNCTION 00552d20
void AInteriorNetwork::UpdateGoalCollisionGroup( void* a_pUnused, TINT a_iGroupId )
{
	auto& llGroup = m_vecGroups[ a_iGroupId ];

	T2_FOREACH( llGroup, it )
	{
		it->OnGoalCollisionUpdate( it.Get() );
	}

	llGroup.Clear();
}

// $Barnyard: FUNCTION 00551d30
void AInteriorNetwork::ClearAreaSearchNodes()
{
	ms_oFreeAreaSearchNodes.RemoveAll();
}

// $Barnyard: FUNCTION 005544c0
void AInteriorNetwork::PushGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface, AInteriorGoal* a_pGoal )
{
	a_pGoalInterface->m_pGoal = a_pGoal;

	const TINT iGroupId = a_pGoal->m_iGroupId;
	if ( iGroupId >= 0 )
	{
		EraseGoalInterface( a_pGoalInterface );

		if ( iGroupId + 1 > m_iMaxGroup )
			m_iMaxGroup = iGroupId + 1;

		m_vecGroups[ iGroupId ].PushBack( a_pGoalInterface );
	}
}

// $Barnyard: FUNCTION 00554500
void AInteriorNetwork::InsertGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface, AInteriorGoal* a_pGoal )
{
	const TINT iGroupId = a_pGoal->m_iGroupId;

	if ( iGroupId >= 0 )
	{
		EraseGoalInterface( a_pGoalInterface );

		if ( iGroupId + 1 > m_iMaxGroup )
			m_iMaxGroup = iGroupId + 1;

		m_vecGroups[ iGroupId ].PushBack( a_pGoalInterface );
	}
}

TPSTRING8_DECLARE( AREA_NA );

// $Barnyard: FUNCTION 005548c0
void AInteriorNetwork::BuildAreaNameLookup()
{
	TVALIDPTR( m_pINetwork );
	ClearAreaNameLookup();

	// Count all areas of name
	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		const TPString8& strAreaName = *m_pINetwork->m_pAreas[ i ].m_strName;

		if ( strAreaName != TPS8( AREA_NA ) )
		{
			auto it = m_pAreaNameLookup->Find( strAreaName );

			if ( it == m_pAreaNameLookup->End() )
				m_pAreaNameLookup->Emplace( strAreaName, LookupList{ 1, TNULL } );
			else
				it->second.iCount += 1;
		}
	}

	// Allocate memory for the areas
	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		const TPString8& strAreaName = *m_pINetwork->m_pAreas[ i ].m_strName;

		if ( strAreaName != TPS8( AREA_NA ) )
		{
			auto it = m_pAreaNameLookup->Find( strAreaName );
			TASSERT( it != m_pAreaNameLookup->End() );

			if ( it->second.pIndices == TNULL )
			{
				// Reset number to zero now, so we can count it again while iterating the array
				it->second.pIndices = new TINT[ it->second.iCount ];
				it->second.iCount   = 0;
			}

			it->second.pIndices[ it->second.iCount++ ] = i;
		}
	}
}

// $Barnyard: FUNCTION 00554a40
void AInteriorNetwork::BuildGoalNameLookup()
{
	TVALIDPTR( m_pINetwork );
	ClearGoalNameLookup();

	// Count all goals of name
	for ( TINT i = 0; i < m_pINetwork->m_iNumGoals; i++ )
	{
		const TPString8& strAreaName = *m_pINetwork->m_pGoals[ i ].m_strName;

		if ( strAreaName != TPS8( AREA_NA ) )
		{
			auto it = m_pGoalNameLookup->Find( strAreaName );

			if ( it == m_pGoalNameLookup->End() )
				m_pGoalNameLookup->Emplace( strAreaName, LookupList{ 1, TNULL } );
			else
				it->second.iCount += 1;
		}
	}

	// Allocate memory for the goals
	for ( TINT i = 0; i < m_pINetwork->m_iNumGoals; i++ )
	{
		const TPString8& strAreaName = *m_pINetwork->m_pGoals[ i ].m_strName;

		if ( strAreaName != TPS8( AREA_NA ) )
		{
			auto it = m_pGoalNameLookup->Find( strAreaName );
			TASSERT( it != m_pGoalNameLookup->End() );

			if ( it->second.pIndices == TNULL )
			{
				// Reset number to zero now, so we can count it again while iterating the array
				it->second.pIndices = new TINT[ it->second.iCount ];
				it->second.iCount   = 0;
			}

			it->second.pIndices[ it->second.iCount++ ] = i;
		}
	}
}

// $Barnyard: FUNCTION 00554700
void AInteriorNetwork::ClearAreaNameLookup()
{
	while ( !m_pAreaNameLookup->IsEmpty() )
	{
		auto it = m_pAreaNameLookup->Begin();

		if ( it->second.pIndices ) delete[] it->second.pIndices;
		it->second.pIndices = TNULL;

		m_pAreaNameLookup->Remove( it );
	}
}

// $Barnyard: FUNCTION 005547e0
void AInteriorNetwork::ClearGoalNameLookup()
{
	while ( !m_pGoalNameLookup->IsEmpty() )
	{
		auto it = m_pGoalNameLookup->Begin();

		if ( it->second.pIndices ) delete[] it->second.pIndices;
		it->second.pIndices = TNULL;

		m_pGoalNameLookup->Remove( it );
	}
}


// $Barnyard: FUNCTION 00552df0
TBOOL AInteriorNetwork::EraseGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface )
{
	if ( a_pGoalInterface->IsLinked() )
	{
		for ( TINT i = m_iMaxGroup - 1; i >= 0; i-- )
		{
			if ( EraseGoalInterfaceFromGroup( a_pGoalInterface, i ) )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00552d80
TBOOL AInteriorNetwork::EraseGoalInterfaceFromGroup( AGoalCollisionUpdateInterface* a_pGoalInterface, TINT a_iGroupId )
{
	TASSERT( a_iGroupId < MAX_NUM_GROUPS );

	auto& llGroup = m_vecGroups[ a_iGroupId ];
	T2_FOREACH( llGroup, it )
	{
		if ( it == a_pGoalInterface )
		{
			llGroup.Erase( a_pGoalInterface );
			return TTRUE;
		}
	}

	return TFALSE;
}

// T2Map methods:
// $Barnyard: FUNCTION 00554680
// $Barnyard: FUNCTION 005545c0
// $Barnyard: FUNCTION 00553b50

// $Barnyard: FUNCTION 005517c0
void AInteriorNetwork::UpdateStartAreaFromPath( AInteriorNetworkPath* a_pPath )
{
	// CheckPointInsideArea takes the XZ position packed into x and y
	// NOTE: retail passes a TVector2 here (CheckPointInsideArea only reads x and y)
	Toshi::TVector4 vecPointXZ;
	vecPointXZ.x = a_pPath->m_vStartPosition.x;
	vecPointXZ.y = a_pPath->m_vStartPosition.z;

	TINT iNumPathAreas = a_pPath->m_oPath.m_iNumPathAreas;

	for ( TINT i = 0; i < iNumPathAreas; i++ )
	{
		TINT iArea = a_pPath->m_oPath.GetPathAreaIndex( i );

		if ( GetArea( iArea )->CheckPointInsideArea( vecPointXZ ) )
		{
			a_pPath->m_iStartArea = iArea;
			a_pPath->m_oPath.AdvancePath( i );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00550f20
Toshi::TVector2& AInteriorNetwork::GetPointXZ( const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutPoint ) const
{
	a_rOutPoint.x = a_rcPosition.x;
	a_rOutPoint.y = a_rcPosition.z;
	return a_rOutPoint;
}

// $Barnyard: FUNCTION 00552310
TINT AInteriorNetwork::FindContainingArea( const Toshi::TVector4& a_rcPoint ) const
{
	// CheckPointInsideArea takes the XZ position packed into x and y
	Toshi::TVector4 vecPointXZ;
	vecPointXZ.x = a_rcPoint.x;
	vecPointXZ.y = a_rcPoint.z;

	TINT   iBestArea   = -1;
	TFLOAT fBestHeight = 100.0f;

	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		AInteriorArea* pArea = &m_pINetwork->m_pAreas[ i ];

		if ( !pArea->CheckPointInsideArea( vecPointXZ ) )
			continue;

		Toshi::TVector4 vecGround( a_rcPoint.x, 0.0f, a_rcPoint.z, 1.0f );
		const_cast<AInteriorNetwork*>( this )->ResolvePointHeight( i, vecGround, 0 );

		if ( iBestArea == -1 )
		{
			// The first match wins unless a closer one is found below
			iBestArea = i;

			if ( a_rcPoint.y < vecGround.y + 0.25f )
				fBestHeight = TMath::Abs( a_rcPoint.y - vecGround.y );

			continue;
		}

		if ( !( pArea->m_eFlags & AInteriorArea::FLAGS_HEIGHT_FROM_TERRAIN ) )
		{
			// The point has to be within the area's height range
			Toshi::TVector4 vecAreaPos;
			pArea->GetPosition( vecAreaPos );

			TFLOAT fTop    = vecAreaPos.y + 0.25f;
			TFLOAT fBottom = -pArea->m_fBaseHeight - 0.25f;

			if ( fTop <= fBottom )
			{
				if ( a_rcPoint.y < fTop || a_rcPoint.y > fBottom )
					continue;
			}
			else
			{
				if ( a_rcPoint.y > fTop || a_rcPoint.y < fBottom )
					continue;
			}
		}

		if ( a_rcPoint.y < vecGround.y + 0.25f )
		{
			TFLOAT fHeight = TMath::Abs( a_rcPoint.y - vecGround.y );

			if ( fHeight < fBestHeight )
			{
				iBestArea   = i;
				fBestHeight = fHeight;
			}
		}
	}

	return iBestArea;
}

// $Barnyard: FUNCTION 00551360
TINT AInteriorNetwork::FindAreaAtPosition( const Toshi::TVector4& a_rcPosition, TINT a_iHintArea ) const
{
	TINT iNumAreas = m_pINetwork->m_iNumAreas;

	// Check the hinted area first
	if ( a_iHintArea >= 0 && a_iHintArea < iNumAreas && m_pINetwork->m_pAreas[ a_iHintArea ].CheckPositionInsideArea( a_rcPosition ) )
		return a_iHintArea;

	for ( TINT i = 0; i < iNumAreas; i++ )
	{
		if ( m_pINetwork->m_pAreas[ i ].CheckPositionInsideArea( a_rcPosition ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00551890
TFLOAT AInteriorNetwork::GetSpecialAreaHeight( TINT a_iAreaIndex ) const
{
	for ( TINT i = 0; i < m_pINetwork->m_iNumSpecialAreas; i++ )
	{
		if ( m_pINetwork->m_piSpecialAreas[ i ] == a_iAreaIndex )
			return m_pINetwork->m_pfSpecialAreaHeights[ i ];
	}

	return 0.0f;
}

// $Barnyard: FUNCTION 00551850
const Toshi::TVector4& AInteriorNetwork::GetSpecialAreaPoint( TINT a_iAreaIndex, TINT a_iPoint ) const
{
	for ( TINT i = 0; i < m_pINetwork->m_iNumSpecialAreas; i++ )
	{
		if ( m_pINetwork->m_piSpecialAreas[ i ] == a_iAreaIndex )
			return TSTATICCAST( Toshi::TVector4, m_pINetwork->m_ppSpecialAreaData[ i ] )[ a_iPoint ];
	}

	// NOTE: retail returns the first point of the first special area
	return *TSTATICCAST( Toshi::TVector4, m_pINetwork->m_ppSpecialAreaData[ 0 ] );
}

// $Barnyard: FUNCTION 00551650
TBOOL AInteriorNetwork::SetPathStartArea( AInteriorNetworkPath* a_pPath, TINT a_iAreaIndex, Toshi::TVector4& a_rOutPosition, const Toshi::TVector4& a_rcUnused )
{
	a_pPath->m_oPath.Reset();
	a_rOutPosition = a_pPath->m_vStartPosition;
	a_pPath->SetStart( a_rOutPosition, a_iAreaIndex );

	return TTRUE;
}

// $Barnyard: FUNCTION 00551b50
AInteriorNetwork::CONNECT AInteriorNetwork::ConnectUnit( AInteriorNetworkPath* a_pPath, AUnit* a_pUnit, const Toshi::TVector4& a_rcPosition )
{
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	if ( pObstruction && !m_vecObstructions.FindItem( pObstruction ) )
		m_vecObstructions.PushBack( pObstruction );

	TINT iArea;

	for ( iArea = 0; iArea < m_pINetwork->m_iNumAreas; iArea++ )
	{
		if ( m_pINetwork->m_pAreas[ iArea ].CheckPositionInsideArea( a_rcPosition ) )
			break;
	}

	if ( iArea < m_pINetwork->m_iNumAreas && iArea != -1 )
	{
		// Inside of an area and far enough from its outer edges
		if ( m_pINetwork->m_pAreas[ iArea ].IsCircleAtPositionInsideArea( a_rcPosition, a_pPath->m_fRadius, TTRUE ) )
		{
			a_pPath->SetStart( a_rcPosition, iArea );
			return CONNECT_INSIDE;
		}
	}
	else
	{
		iArea = FindClosestArea( a_rcPosition );
	}

	if ( iArea < 0 )
	{
		a_pPath->SetStart( a_rcPosition, 0 );
		a_pPath->m_oPath.Reset();
		return CONNECT_OUTSIDE;
	}

	// Walk to the area first
	a_pPath->SetStart( a_rcPosition, iArea );

	Toshi::TVector4 vecAreaPosition;
	m_pINetwork->m_pAreas[ iArea ].GetPosition( vecAreaPosition );

	a_pPath->m_oPath.Reset();
	a_pPath->m_oPath.PushPathArea( iArea );
	a_pPath->m_oPath.m_vPosition    = vecAreaPosition;
	a_pPath->m_oPath.m_vDestination = vecAreaPosition;
	a_pPath->m_oPath.m_vForward     = Toshi::TVector4::VEC_ZERO;

	return CONNECT_OUTSIDE;
}

// $Barnyard: FUNCTION 00551cf0
TBOOL AInteriorNetwork::DisconnectUnit( AInteriorNetworkPath* a_pPath, AUnit* a_pUnit )
{
	a_pPath->m_oPath.Reset();

	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	if ( pObstruction && m_vecObstructions.FindItem( pObstruction ) != TNULL )
		m_vecObstructions.FindAndErase( pObstruction );

	return TTRUE;
}

// $Barnyard: FUNCTION 00551470
TINT AInteriorNetwork::FindClosestArea( const Toshi::TVector4& a_rcPosition ) const
{
	TFLOAT fBestDistanceSq = Toshi::TMath::MAXFLOAT;
	TINT   iBestArea       = -1;

	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		Toshi::TVector4 vecAreaPosition;
		m_pINetwork->m_pAreas[ i ].GetPosition( vecAreaPosition );

		TFLOAT fDistanceSq = ( vecAreaPosition.z - a_rcPosition.z ) * ( vecAreaPosition.z - a_rcPosition.z ) + ( vecAreaPosition.x - a_rcPosition.x ) * ( vecAreaPosition.x - a_rcPosition.x );

		if ( fDistanceSq < fBestDistanceSq )
		{
			fBestDistanceSq = fDistanceSq;
			iBestArea       = i;
		}
	}

	return iBestArea;
}

//-----------------------------------------------------------------------------
// Searches
//-----------------------------------------------------------------------------

// 0x007840d0
AInteriorNetwork::AreaSearchNode AInteriorNetwork::ms_aAreaSearchNodes[ AInteriorNetwork::MAX_NUM_AREA_SEARCH_NODES ];

// 0x007845b0
AInteriorNetwork::PathSearch AInteriorNetwork::ms_oPathSearch;

// $Barnyard: FUNCTION 005507c0
AInteriorNetwork::PathSearch::Node::Node()
{
	m_iArea      = -1;
	m_iParent    = -1;
	m_eState     = STATE_NEW;
	m_fCost      = 0.0f;
	m_fHeuristic = 0.0f;
	m_fTotal     = 0.0f;
}

// $Barnyard: FUNCTION 005508c0
AInteriorNetwork::PathSearch::PathSearch()
{
	for ( TINT i = 0; i < AInteriorNetworkPath::MAX_NUM_PATH_AREAS; i++ )
		m_aNodes[ i ].m_iArea = i;
}

// $Barnyard: FUNCTION 00550810
AInteriorNetwork::PathSearch::~PathSearch()
{
	m_llOpenNodes.RemoveAll();
}

// $Barnyard: FUNCTION 00550870
void AInteriorNetwork::PathSearch::Reset()
{
	m_llOpenNodes.RemoveAll();

	for ( TINT i = 0; i < AInteriorNetworkPath::MAX_NUM_PATH_AREAS; i++ )
	{
		m_aNodes[ i ].m_iParent    = -1;
		m_aNodes[ i ].m_eState     = STATE_NEW;
		m_aNodes[ i ].m_fCost      = 0.0f;
		m_aNodes[ i ].m_fHeuristic = 0.0f;
		m_aNodes[ i ].m_fTotal     = 0.0f;
	}
}

// $Barnyard: FUNCTION 00550950
void AInteriorNetwork::PathSearch::OpenStart( TINT a_iArea, TFLOAT a_fHeuristic )
{
	Node& rNode = m_aNodes[ a_iArea ];

	rNode.m_eState     = STATE_OPEN;
	rNode.m_fCost      = 0.0f;
	rNode.m_fHeuristic = a_fHeuristic;
	rNode.m_fTotal     = a_fHeuristic;
	m_llOpenNodes.InsertTail( &rNode );
}

// $Barnyard: FUNCTION 00550910
AInteriorNetwork::PathSearch::Node* AInteriorNetwork::PathSearch::PopBest()
{
	if ( m_llOpenNodes.IsEmpty() )
		return TNULL;

	Node* pNode = m_llOpenNodes.Begin();
	pNode->Remove();
	pNode->m_eState = STATE_CLOSED;

	return pNode;
}

// $Barnyard: FUNCTION 00550990
void AInteriorNetwork::PathSearch::Visit( TINT a_iArea, TINT a_iParent, TFLOAT a_fCost, TFLOAT a_fHeuristic )
{
	Node& rNode   = m_aNodes[ a_iArea ];
	TBOOL bBetter = a_fHeuristic < rNode.m_fHeuristic;
	TBOOL bOpen   = TFALSE;

	if ( rNode.m_eState == STATE_NEW )
	{
		bOpen   = TTRUE;
		bBetter = TTRUE;
	}
	else if ( rNode.m_eState == STATE_CLOSED )
	{
		if ( !bBetter )
			return;

		// Open it again
		bOpen = TTRUE;
	}

	if ( bBetter )
	{
		rNode.m_fCost      = a_fCost;
		rNode.m_fHeuristic = a_fHeuristic;
		rNode.m_fTotal     = a_fCost + a_fHeuristic;
		rNode.m_iParent    = a_iParent;
	}

	if ( bOpen )
	{
		rNode.m_eState = STATE_OPEN;

		// Keep the open nodes sorted by their total cost
		auto it = m_llOpenNodes.Begin();

		while ( it != m_llOpenNodes.End() && it->m_fTotal < rNode.m_fTotal )
			it = it.Next();

		rNode.InsertBefore( it.Get() );
	}
}

// $Barnyard: FUNCTION 005508b0
TINT AInteriorNetwork::PathSearch::GetParent( TINT a_iArea ) const
{
	return m_aNodes[ a_iArea ].m_iParent;
}

// Distance (scaled by nothing) from the area to the closest of the target areas, TMath::MAXFLOAT if a_iArea is a special area
// $Barnyard: FUNCTION 00551910
TFLOAT AInteriorNetwork::GetHeuristicDistance( TINT a_iArea, const TINT* a_pTargetAreas, TINT a_iNumTargetAreas ) const
{
	TFLOAT fMinDistance = TMath::MAXFLOAT;

	for ( TINT i = 0; i < a_iNumTargetAreas; i++ )
	{
		TINT iTarget = a_pTargetAreas[ i ];

		if ( !m_pINetwork->m_pAreas[ a_iArea ].IsSpecialArea( 0 ) )
		{
			const AInteriorArea& rcArea   = m_pINetwork->m_pAreas[ a_iArea ];
			const AInteriorArea& rcTarget = m_pINetwork->m_pAreas[ iTarget ];

			TFLOAT fDistance = sqrtf( TVector2::DistanceSq( TVector2( rcArea.m_fOriginX, rcArea.m_fOriginZ ), TVector2( rcTarget.m_fOriginX, rcTarget.m_fOriginZ ) ) );

			if ( fDistance < fMinDistance )
				fMinDistance = fDistance;
		}
	}

	return fMinDistance;
}

// $Barnyard: FUNCTION 00553220
TBOOL AInteriorNetwork::BuildPathToArea( TINT a_iStartArea, TINT a_iTargetArea, AInteriorNetworkPath::BuildPathState* a_pState )
{
	// NOTE: retail goes through an array of the one target area, it's the same search as BuildPathToAnyArea
	if ( a_iTargetArea == a_iStartArea )
	{
		a_pState->PushPathArea( a_iStartArea );
		return TTRUE;
	}

	ms_oPathSearch.Reset();
	ms_oPathSearch.OpenStart( a_iStartArea, m_pINetwork->m_pAreas[ a_iStartArea ].m_fPathHeuristicScale * GetHeuristicDistance( a_iStartArea, &a_iTargetArea, 1 ) );

	TBOOL bFound     = TFALSE;
	TINT  iFoundArea = -1;

	while ( PathSearch::Node* pNode = ms_oPathSearch.PopBest() )
	{
		TINT                 iArea  = pNode->m_iArea;
		TFLOAT               fCost  = pNode->m_fCost;
		const AInteriorArea& rcArea = m_pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rcArea.m_iNumLinks; i++ )
		{
			const AInteriorAreaLink& rcLink   = rcArea.m_pLinks[ i ];
			TINT                     iLinked  = rcLink.m_iAreaIndex;
			const AInteriorArea&     rcLinked = m_pINetwork->m_pAreas[ iLinked ];

			if ( HASANYFLAG( rcLinked.m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
				continue;

			ms_oPathSearch.Visit( iLinked, iArea, rcLink.m_fCost + fCost, rcLinked.m_fPathHeuristicScale * GetHeuristicDistance( iLinked, &a_iTargetArea, 1 ) );

			if ( iLinked == a_iTargetArea )
			{
				bFound     = TTRUE;
				iFoundArea = iLinked;
				break;
			}

			// NOTE: once the target is found, the remaining open nodes only get their first routable link visited
			if ( bFound )
				break;
		}
	}

	a_pState->ClearPath();

	if ( bFound )
	{
		TINT iArea;

		for ( iArea = iFoundArea; iArea != a_iStartArea; iArea = ms_oPathSearch.GetParent( iArea ) )
			a_pState->PushPathArea( iArea );

		a_pState->PushPathArea( iArea );
		return bFound;
	}

	a_pState->m_uiFlags |= AInteriorNetworkPath::BuildPathState::FLAGS_NOT_FOUND;
	return TFALSE;
}

// $Barnyard: FUNCTION 005520d0
TBOOL AInteriorNetwork::BuildPathToAnyArea( TINT a_iStartArea, const TINT* a_pTargetAreas, TINT a_iNumTargetAreas, AInteriorNetworkPath::BuildPathState* a_pState, TINT* a_pOutArea )
{
	for ( TINT i = 0; i < a_iNumTargetAreas; i++ )
	{
		if ( a_pTargetAreas[ i ] == a_iStartArea )
		{
			*a_pOutArea = a_iStartArea;
			a_pState->PushPathArea( a_iStartArea );
			return TTRUE;
		}
	}

	ms_oPathSearch.Reset();
	ms_oPathSearch.OpenStart( a_iStartArea, m_pINetwork->m_pAreas[ a_iStartArea ].m_fPathHeuristicScale * GetHeuristicDistance( a_iStartArea, a_pTargetAreas, a_iNumTargetAreas ) );

	TBOOL bFound = TFALSE;

	while ( PathSearch::Node* pNode = ms_oPathSearch.PopBest() )
	{
		TINT                 iArea  = pNode->m_iArea;
		TFLOAT               fCost  = pNode->m_fCost;
		const AInteriorArea& rcArea = m_pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rcArea.m_iNumLinks; i++ )
		{
			const AInteriorAreaLink& rcLink   = rcArea.m_pLinks[ i ];
			TINT                     iLinked  = rcLink.m_iAreaIndex;
			const AInteriorArea&     rcLinked = m_pINetwork->m_pAreas[ iLinked ];

			if ( HASANYFLAG( rcLinked.m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
				continue;

			ms_oPathSearch.Visit( iLinked, iArea, rcLink.m_fCost + fCost, rcLinked.m_fPathHeuristicScale * GetHeuristicDistance( iLinked, a_pTargetAreas, a_iNumTargetAreas ) );

			TBOOL bIsTarget = TFALSE;

			for ( TINT k = 0; k < a_iNumTargetAreas; k++ )
			{
				if ( a_pTargetAreas[ k ] == iLinked )
				{
					bFound      = TTRUE;
					bIsTarget   = TTRUE;
					*a_pOutArea = a_pTargetAreas[ k ];
					break;
				}
			}

			// NOTE: once a target is found, the remaining open nodes only get their first routable link visited
			if ( bIsTarget || bFound )
				break;
		}
	}

	a_pState->ClearPath();

	if ( bFound )
	{
		TINT iArea;

		for ( iArea = *a_pOutArea; iArea != a_iStartArea; iArea = ms_oPathSearch.GetParent( iArea ) )
			a_pState->PushPathArea( iArea );

		a_pState->PushPathArea( iArea );
		return bFound;
	}

	a_pState->m_uiFlags |= AInteriorNetworkPath::BuildPathState::FLAGS_NOT_FOUND;
	return TFALSE;
}

// $Barnyard: FUNCTION 00552700
TBOOL AInteriorNetwork::FindPointAwayFrom( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMinDistance, const Toshi::TVector4& a_rcDirection, TFLOAT a_fMinDot, Toshi::TVector4& a_rOutPosition, TBOOL a_bAllowCloser, TINT a_iMaxDepth )
{
	TINT iNumAreas  = m_pINetwork->m_iNumAreas;
	TINT iStartArea = 0;

	if ( iNumAreas <= 0 )
		return TFALSE;

	while ( !m_pINetwork->m_pAreas[ iStartArea ].CheckPositionInsideArea( a_rcPosition ) )
	{
		if ( ++iStartArea >= iNumAreas )
			return TFALSE;
	}

	// The areas of the current depth and of the next one
	TDList<AreaSearchNode> aLists[ 2 ];
	TINT                   iCurrentList = 0;

	ms_oFreeAreaSearchNodes.RemoveAll();

	for ( TINT i = 1; i < MAX_NUM_AREA_SEARCH_NODES; i++ )
		ms_oFreeAreaSearchNodes.InsertTail( &ms_aAreaSearchNodes[ i ] );

	aLists[ 0 ].InsertTail( &ms_aAreaSearchNodes[ 0 ] );
	ms_aAreaSearchNodes[ 0 ].m_pArea = &m_pINetwork->m_pAreas[ iStartArea ];

	TINT   iBestArea     = -1;
	TFLOAT fBestDistance = 0.0f;

	for ( TINT iDepth = a_iMaxDepth; iDepth > 0; iDepth-- )
	{
		TDList<AreaSearchNode>& rList = aLists[ iCurrentList ];

		while ( !rList.IsEmpty() )
		{
			AreaSearchNode* pNode = rList.Head();
			pNode->Remove();

			AInteriorArea* pArea = pNode->m_pArea;

			for ( TINT i = 0; i < pArea->m_iNumLinks; i++ )
			{
				TINT           iLinked = pArea->m_pLinks[ i ].m_iAreaIndex;
				AInteriorArea* pLinked = &m_pINetwork->m_pAreas[ iLinked ];

				if ( HASANYFLAG( pLinked->m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
					continue;

				TVector4 vecAreaPosition;
				pLinked->GetPositionXZ( vecAreaPosition );

				TVector4 vecDirection( vecAreaPosition.x - a_rcPosition.x, 0.0f, vecAreaPosition.z - a_rcPosition.z, vecAreaPosition.w );
				TFLOAT   fDistance = vecDirection.NormaliseMag();

				if ( TVector4::DotProduct3( a_rcDirection, vecDirection ) > a_fMinDot )
				{
					if ( fDistance >= a_fMinDistance )
					{
						a_rOutPosition = vecAreaPosition;

						aLists[ 0 ].RemoveAll();
						aLists[ 1 ].RemoveAll();
						return TTRUE;
					}

					if ( a_bAllowCloser && fDistance > fBestDistance )
					{
						iBestArea     = iLinked;
						fBestDistance = fDistance;
					}

					// Check its links at the next depth
					if ( !ms_oFreeAreaSearchNodes.IsEmpty() )
					{
						AreaSearchNode* pNewNode = ms_oFreeAreaSearchNodes.Head();
						pNewNode->Remove();
						pNewNode->m_pArea = pLinked;
						aLists[ 1 - iCurrentList ].InsertTail( pNewNode );
					}
				}
			}

			ms_oFreeAreaSearchNodes.InsertTail( pNode );
		}

		iCurrentList = 1 - iCurrentList;
	}

	aLists[ 0 ].RemoveAll();
	aLists[ 1 ].RemoveAll();

	if ( a_bAllowCloser && iBestArea != -1 )
	{
		m_pINetwork->m_pAreas[ iBestArea ].GetPositionXZ( a_rOutPosition );
		return TTRUE;
	}

	return TFALSE;
}

//-----------------------------------------------------------------------------
// Paths
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00550fb0
void AInteriorNetwork::ResetPath( AInteriorNetworkPath* a_pPath )
{
	a_pPath->m_oPath.Reset();
}

// $Barnyard: FUNCTION 00550f40
TBOOL AInteriorNetwork::FindSpecialAreaInPath( AInteriorNetworkPath::BuildPathState* a_pState, TINT a_iMaxAreas )
{
	TINT iNumAreas = a_pState->m_iNumPathAreas;

	if ( iNumAreas > a_iMaxAreas )
		iNumAreas = a_iMaxAreas;

	for ( TINT i = 0; i < iNumAreas; i++ )
	{
		if ( m_pINetwork->m_pAreas[ a_pState->GetPathAreaIndex( i ) ].IsSpecialArea( 0 ) )
		{
			a_pState->m_iSpecialAreaIndex = i;
			return TTRUE;
		}
	}

	a_pState->m_iSpecialAreaIndex = -1;
	return TFALSE;
}

// Moves a_rEdgeA and a_rEdgeB (a portal) towards each other by a_fInset (or to the middle if the portal is narrower than twice a_fInset)
// and outputs the point of the shrunk portal closest to a_rcPoint. Returns TFALSE if the portal is narrower than twice a_fMinHalfWidth.
// $Barnyard: FUNCTION 0054f9c0
static TBOOL ShrinkPortal( TVector2& a_rEdgeA, TVector2& a_rEdgeB, const TVector2& a_rcPoint, TVector2& a_rOutClosest, TFLOAT a_fMinHalfWidth, TFLOAT a_fInset )
{
	TVector2 vecDirection( a_rEdgeB.x - a_rEdgeA.x, a_rEdgeB.y - a_rEdgeA.y );
	TFLOAT   fLength = sqrtf( vecDirection.MagnitudeSq() );
	TFLOAT   fInset;

	if ( a_fInset + a_fInset >= fLength )
	{
		if ( a_fMinHalfWidth + a_fMinHalfWidth >= fLength )
			return TFALSE;

		fInset = fLength * 0.5f;
	}
	else
	{
		fInset = a_fInset;
	}

	TFLOAT fInvLength = 1.0f / fLength;
	vecDirection.x    = vecDirection.x * fInvLength;
	vecDirection.y    = fInvLength * vecDirection.y;

	TFLOAT fOffsetX = vecDirection.x * fInset;
	TFLOAT fOffsetY = vecDirection.y * fInset;

	a_rEdgeA.x = fOffsetX + a_rEdgeA.x;
	a_rEdgeA.y = fOffsetY + a_rEdgeA.y;
	a_rEdgeB.x = a_rEdgeB.x - fOffsetX;
	a_rEdgeB.y = a_rEdgeB.y - fOffsetY;

	TFLOAT fAlong = ( a_rcPoint.x - a_rEdgeA.x ) * vecDirection.x + ( a_rcPoint.y - a_rEdgeA.y ) * vecDirection.y;

	if ( fAlong < 0.0f )
	{
		a_rOutClosest = a_rEdgeA;
	}
	else if ( ( a_rcPoint.x - a_rEdgeB.x ) * vecDirection.x + ( a_rcPoint.y - a_rEdgeB.y ) * vecDirection.y <= 0.0f )
	{
		a_rOutClosest.x = vecDirection.x * fAlong + a_rEdgeA.x;
		a_rOutClosest.y = vecDirection.y * fAlong + a_rEdgeA.y;
	}
	else
	{
		a_rOutClosest = a_rEdgeB;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00551010
TBOOL AInteriorNetwork::FindNextPathPoint( AInteriorNetworkPath::BuildPathState* a_pState, TFLOAT a_fRadius, const Toshi::TVector2& a_rcStart, const Toshi::TVector2& a_rcTarget, Toshi::TVector2& a_rOutPoint, TINT* a_pOutArea )
{
	TINT iNumAreas = a_pState->m_iNumPathAreas;
	TINT iArea     = a_pState->GetPathAreaIndex( 0 );

	// NOTE: retail checks if the start is in the first area of the path but ignores the result
	m_pINetwork->m_pAreas[ iArea ].CheckPointInsideArea( TVector4( a_rcStart.x, a_rcStart.y, 0.0f, 0.0f ) );

	AInteriorPathFunnel oFunnel;
	oFunnel.Init( a_rcStart );

	TBOOL bFound = TFALSE;

	if ( iNumAreas - 1 > 0 )
	{
		TINT i = 0;

		while ( TTRUE )
		{
			TINT   iNextArea     = a_pState->GetPathAreaIndex( i + 1 );
			TFLOAT fBestDistance = TMath::MAXFLOAT;

			// The funnel as it was when entering the area
			AInteriorPathFunnel oAreaFunnel;
			oAreaFunnel = oFunnel;

			const AInteriorArea& rcArea       = m_pINetwork->m_pAreas[ iArea ];
			TBOOL                bFoundInArea = TFALSE;

			for ( TINT k = 0; k < rcArea.m_iNumArcBounds; k++ )
			{
				// The portal to the next area
				if ( rcArea.m_pArcBounds[ k ].m_iNeighborArea != iNextArea )
					continue;

				TVector2 vecEdgeA = rcArea.m_pArcBounds[ k ].GetPoint();
				TVector2 vecEdgeB = rcArea.m_pArcBounds[ ( k + 1 ) % rcArea.m_iNumArcBounds ].GetPoint();
				TVector2 vecClosest;

				if ( !ShrinkPortal( vecEdgeA, vecEdgeB, a_rcStart, vecClosest, a_fRadius + 0.1f, a_fRadius * 1.6f ) )
					continue;

				TFLOAT fDistance = sqrtf( TVector2::DistanceSq( vecClosest, a_rcStart ) );

				if ( fDistance >= fBestDistance )
					continue;

				fBestDistance = fDistance;

				if ( !oAreaFunnel.IsPointInside( vecClosest ) )
				{
					oFunnel.m_eState = AInteriorPathFunnel::STATE_CLOSED;
					continue;
				}

				TVector2 vecClipped;

				if ( rcArea.ClipLineToCorners( a_rcStart, vecClosest, a_fRadius, vecClipped, &oFunnel ) )
				{
					// The portal can be seen, continue to the next area
					a_rOutPoint  = vecClosest;
					*a_pOutArea  = iArea;
					bFound       = TTRUE;
					bFoundInArea = TTRUE;
					oFunnel.NarrowToPortal( vecEdgeA, vecEdgeB );
				}
				else if ( oAreaFunnel.IsPointInside( vecClipped ) )
				{
					// A corner is in the way, walk to it
					a_rOutPoint      = vecClipped;
					*a_pOutArea      = iArea;
					bFound           = TTRUE;
					bFoundInArea     = TTRUE;
					oFunnel.m_eState = AInteriorPathFunnel::STATE_CLOSED;
				}
			}

			if ( !bFoundInArea )
				oFunnel.m_eState = AInteriorPathFunnel::STATE_CLOSED;

			if ( !bFoundInArea || oFunnel.m_eState == AInteriorPathFunnel::STATE_CLOSED || i + 1 >= iNumAreas - 1 )
			{
				iArea = iNextArea;
				break;
			}

			i++;
			iArea = iNextArea;
		}
	}

	// Can the target be seen?
	if ( oFunnel.IsPointInside( a_rcTarget ) )
	{
		TVector2 vecClipped;

		if ( m_pINetwork->m_pAreas[ iArea ].ClipLineToCorners( a_rcStart, a_rcTarget, a_fRadius, vecClipped, TNULL ) )
		{
			a_rOutPoint = a_rcTarget;
			*a_pOutArea = iArea;
			return TTRUE;
		}

		if ( oFunnel.IsPointInside( vecClipped ) )
		{
			a_rOutPoint = vecClipped;
			*a_pOutArea = iArea;
			return TTRUE;
		}
	}

	if ( bFound || a_pState->m_iNumPathAreas <= 1 )
		return bFound;

	TINT iLastArea = a_pState->GetPathAreaIndex( a_pState->m_iNumPathAreas - 1 );

	if ( a_pState->GetPathAreaIndex( 0 ) == iLastArea )
		return bFound;

	// Walk to the origin of the second area
	TINT iSecondArea = a_pState->GetPathAreaIndex( 1 );
	*a_pOutArea      = iSecondArea;
	a_rOutPoint.x    = m_pINetwork->m_pAreas[ iSecondArea ].m_fOriginX;
	a_rOutPoint.y    = m_pINetwork->m_pAreas[ iSecondArea ].m_fOriginZ;

	return TTRUE;
}

// $Barnyard: FUNCTION 005524b0
TBOOL AInteriorNetwork::FinishPath( AInteriorNetworkPath* a_pPath, const Toshi::TVector2& a_rcTarget, const Toshi::TVector4* a_pForward )
{
	TVector2 vecStart( a_pPath->m_vStartPosition.x, a_pPath->m_vStartPosition.z );
	TVector2 vecNext;
	TINT     iNextArea;

	if ( !FindNextPathPoint( &a_pPath->m_oPath, a_pPath->m_fRadius, vecStart, a_rcTarget, vecNext, &iNextArea ) )
		return TFALSE;

	TVector4 vecNextPosition( vecNext.x, 0.0f, vecNext.y, 1.0f );
	ResolvePointHeight( iNextArea, vecNextPosition, 0 );

	TVector4 vecDestination( a_rcTarget.x, 0.0f, a_rcTarget.y, 1.0f );
	ResolvePointHeight( a_pPath->m_oPath.GetPathAreaIndex( a_pPath->m_oPath.m_iNumPathAreas - 1 ), vecDestination, 0 );

	a_pPath->m_oPath.m_vDestination = vecDestination;
	a_pPath->m_oPath.m_vPosition    = vecNextPosition;
	a_pPath->m_oPath.m_vForward     = a_pForward ? *a_pForward : TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 005525f0
TBOOL AInteriorNetwork::FinishPath( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget, const Toshi::TVector4* a_pForward )
{
	TVector2 vecStart( a_pPath->m_vStartPosition.x, a_pPath->m_vStartPosition.z );
	TVector2 vecTarget( a_rcTarget.x, a_rcTarget.z );
	TVector2 vecNext;
	TINT     iNextArea;

	if ( !FindNextPathPoint( &a_pPath->m_oPath, a_pPath->m_fRadius, vecStart, vecTarget, vecNext, &iNextArea ) )
		return TFALSE;

	TVector4 vecNextPosition( vecNext.x, 0.0f, vecNext.y, 1.0f );
	ResolvePointHeight( iNextArea, vecNextPosition, 0 );

	a_pPath->m_oPath.m_vDestination = a_rcTarget;
	a_pPath->m_oPath.m_vPosition    = vecNextPosition;
	a_pPath->m_oPath.m_vForward     = a_pForward ? *a_pForward : TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 00553400
TBOOL AInteriorNetwork::BuildPathToPoint( AInteriorNetworkPath* a_pPath, const Toshi::TVector2& a_rcTarget, TINT a_iTargetArea, const Toshi::TVector4* a_pForward )
{
	if ( BuildPathToArea( a_pPath->m_iStartArea, a_iTargetArea, &a_pPath->m_oPath ) && FinishPath( a_pPath, a_rcTarget, a_pForward ) )
		return TTRUE;

	a_pPath->m_oPath.m_vPosition = a_pPath->m_vStartPosition;
	return TFALSE;
}

// $Barnyard: FUNCTION 00553470
TBOOL AInteriorNetwork::BuildPathToPosition( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget, TINT a_iTargetArea, const Toshi::TVector4* a_pForward )
{
	if ( BuildPathToArea( a_pPath->m_iStartArea, a_iTargetArea, &a_pPath->m_oPath ) && FinishPath( a_pPath, a_rcTarget, a_pForward ) )
		return TTRUE;

	a_pPath->m_oPath.m_vPosition = a_pPath->m_vStartPosition;
	return TFALSE;
}

// $Barnyard: FUNCTION 00553880
TBOOL AInteriorNetwork::BuildPathToContainingArea( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget )
{
	TINT iArea = FindContainingArea( a_rcTarget );

	if ( iArea == -1 )
		return TFALSE;

	return BuildPathToPosition( a_pPath, a_rcTarget, iArea, TNULL );
}

// $Barnyard: FUNCTION 005534e0
TBOOL AInteriorNetwork::BuildPathToTypedArea( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strType )
{
	auto it = m_pAreaNameLookup->Find( a_strType );

	if ( it == m_pAreaNameLookup->End() )
		return TFALSE;

	TINT iArea;

	if ( BuildPathToAnyArea( a_pPath->m_iStartArea, it->second.pIndices, it->second.iCount, &a_pPath->m_oPath, &iArea ) )
	{
		TVector4 vecTarget;
		m_pINetwork->m_pAreas[ iArea ].GetRandomPosition( vecTarget, a_pPath->m_fRadius );

		if ( FinishPath( a_pPath, vecTarget, TNULL ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00553590
TBOOL AInteriorNetwork::BuildPathToRandomPointInArea( AInteriorNetworkPath* a_pPath, TINT a_iArea )
{
	if ( BuildPathToArea( a_pPath->m_iStartArea, a_iArea, &a_pPath->m_oPath ) )
	{
		TVector4 vecTarget;
		m_pINetwork->m_pAreas[ a_iArea ].GetRandomPosition( vecTarget, a_pPath->m_fRadius );

		if ( FinishPath( a_pPath, vecTarget, TNULL ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00552aa0
TINT AInteriorNetwork::GetRandomArea( TBOOL a_bNormalOnly )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	if ( !a_bNormalOnly )
		return rRandom.GetInt( m_pINetwork->m_iNumAreas );

	TINT iArea = rRandom.GetInt( m_pINetwork->m_iNumNormalAreas );

	if ( HASANYFLAG( m_pINetwork->m_pAreas[ iArea ].m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
	{
		TINT iNumAreas = m_pINetwork->m_iNumAreas;

		for ( TINT i = 0;; )
		{
			iArea = rRandom.GetInt( iNumAreas );

			if ( !HASANYFLAG( m_pINetwork->m_pAreas[ iArea ].m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
				break;

			if ( ++i >= 20 )
			{
				// Pick one of the routable areas
				TINT16 aRoutableAreas[ 64 ];
				TINT   iNumRoutable = 0;

				for ( TINT k = 0; k < iNumAreas; k++ )
				{
					if ( !HASANYFLAG( m_pINetwork->m_pAreas[ k ].m_eFlags, AInteriorArea::FLAGS_NON_ROUTABLE ) )
					{
#ifdef BARNYARD_COMMUNITY_PATCH
						// NOTE: retail writes past the array if there are more than 64 routable areas
						if ( iNumRoutable >= TARRAYSIZE( aRoutableAreas ) )
							break;
#endif

						aRoutableAreas[ iNumRoutable++ ] = TINT16( k );
					}
				}

				if ( iNumRoutable <= 0 )
					return -1;

				return aRoutableAreas[ rRandom.GetInt( iNumRoutable ) ];
			}
		}
	}

	return iArea;
}

// $Barnyard: FUNCTION 00554180
TBOOL AInteriorNetwork::GetShuffledAreas( TINT* a_piOutIndices ) const
{
	for ( TINT i = 0; i < a_piOutIndices[ 0 ]; i++ )
		a_piOutIndices[ i + 1 ] = 0;

	a_piOutIndices[ 0 ] = 0;

	// NOTE: retail uses a weighted random picker (inlined, 004ad3d0 picks and 004ad350 removes an entry), every area has the weight 1
	struct Entry
	{
		TINT iArea;
		TINT iWeight;
	};

	TRandom* pRandom      = &ARandom::GetSingleton()->m_oRandom;
	TINT     iTotalWeight = 0;
	TINT     iNumEntries  = 0;
	Entry    aEntries[ 64 ];

	for ( TINT i = 0; i < TARRAYSIZE( aEntries ); i++ )
	{
		aEntries[ i ].iArea   = 0;
		aEntries[ i ].iWeight = 0;
	}

	TINT iNumLeft = 0;

	// NOTE: retail writes past the array if the network has more than 64 areas
	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		iTotalWeight++;
		aEntries[ iNumEntries ].iArea     = i;
		aEntries[ iNumEntries++ ].iWeight = 1;
		iNumLeft++;
	}

	if ( iNumEntries == 0 || ARandom::IsWeightZero( iTotalWeight ) )
		return TFALSE;

	for ( ; iNumLeft > 0; iNumLeft-- )
	{
		// Pick an entry
		Entry* pEntry = TNULL;

		if ( iNumEntries != 0 && iTotalWeight != 0 )
		{
			TINT iRandom = pRandom->GetInt( iTotalWeight );
			TINT iSum    = 0;

			for ( TINT i = 0; i < iNumEntries; i++ )
			{
				iSum += aEntries[ i ].iWeight;

				if ( iSum > iRandom )
				{
					pEntry = &aEntries[ i ];
					break;
				}
			}
		}

		a_piOutIndices[ ++a_piOutIndices[ 0 ] ] = pEntry->iArea;

		// Remove it
		for ( TINT i = 0; i < iNumEntries; i++ )
		{
			if ( &aEntries[ i ] == pEntry )
			{
				iTotalWeight -= pEntry->iWeight;

				for ( TINT k = i; k < iNumEntries - 1; k++ )
					aEntries[ k ] = aEntries[ k + 1 ];

				aEntries[ iNumEntries - 1 ].iArea   = 0;
				aEntries[ iNumEntries - 1 ].iWeight = 0;
				iNumEntries--;
				break;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00552e30
Toshi::TVector4 AInteriorNetwork::GetRandomAreaPosition( TBOOL a_bNormalOnly )
{
	TINT iArea = GetRandomArea( a_bNormalOnly );

	TVector4 vecPosition;
	m_pINetwork->m_pAreas[ iArea ].GetRandomPosition( vecPosition, 0.5f );

	return vecPosition;
}

// $Barnyard: FUNCTION 005538b0
TBOOL AInteriorNetwork::BuildPathToRandomArea( AInteriorNetworkPath* a_pPath )
{
	if ( m_pINetwork->m_iNumAreas >= 2 )
	{
		for ( TINT i = 0; i < 50; i++ )
		{
			// NOTE: GetRandomArea is inlined in retail
			TINT                 iArea  = GetRandomArea( TTRUE );
			const AInteriorArea& rcArea = m_pINetwork->m_pAreas[ iArea ];

			if ( !rcArea.IsSpecialArea( 0 ) )
			{
				a_pPath->m_oPath.Reset();

				TVector2 vecTarget;
				rcArea.GetRandomPointXZ( vecTarget, a_pPath->m_fRadius );

				if ( BuildPathToPoint( a_pPath, vecTarget, iArea, TNULL ) )
					return TTRUE;
			}
		}
	}
	else
	{
		TVector2 vecTarget;
		m_pINetwork->m_pAreas[ 0 ].GetRandomPointXZ( vecTarget, a_pPath->m_fRadius );

		if ( BuildPathToPoint( a_pPath, vecTarget, 0, TNULL ) )
			return TTRUE;
	}

	// Try the area the unit is in
	TINT iStartArea = a_pPath->m_iStartArea;

	if ( iStartArea != -1 )
	{
		const AInteriorArea& rcArea = m_pINetwork->m_pAreas[ iStartArea ];
		a_pPath->m_oPath.Reset();

		TVector2 vecTarget;
		rcArea.GetRandomPointXZ( vecTarget, a_pPath->m_fRadius );

		if ( BuildPathToPoint( a_pPath, vecTarget, iStartArea, TNULL ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00553600
TBOOL AInteriorNetwork::BuildPathToGoal( AInteriorNetworkPath* a_pPath, AInteriorGoal* a_pGoal, TBOOL a_bUseFacing )
{
	TVector4 vecGoalPosition;
	a_pGoal->GetPosition( vecGoalPosition );

	TVector4        vecForward;
	const TVector4* pForward = TNULL;

	if ( a_bUseFacing == TTRUE )
	{
		vecForward = a_pGoal->m_vForward;
		pForward   = &vecForward;
	}

	return BuildPathToPosition( a_pPath, vecGoalPosition, a_pGoal->m_iAreaIndex, pForward );
}

// $Barnyard: FUNCTION 00553a70
TBOOL AInteriorNetwork::BuildPathToRandomVisibleGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, TBOOL a_bUseFacing )
{
	AInteriorGoal* pGoal = FindRandomVisibleGoal( a_strGoal );

	if ( pGoal )
		return BuildPathToGoal( a_pPath, pGoal, a_bUseFacing );

	return TFALSE;
}

// $Barnyard: FUNCTION 00553aa0
TBOOL AInteriorNetwork::BuildPathToFarthestGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, const Toshi::TVector4& a_rcPosition )
{
	AInteriorGoal* pGoal = FindFarthestGoal( a_strGoal, a_rcPosition );

	if ( !pGoal )
		return TFALSE;

	TVector4 vecGoalPosition;
	pGoal->GetPosition( vecGoalPosition );

	return BuildPathToPosition( a_pPath, vecGoalPosition, pGoal->m_iAreaIndex, TNULL );
}

// $Barnyard: FUNCTION 00553b00
TBOOL AInteriorNetwork::BuildPathToGoalExit( AInteriorNetworkPath* a_pPath, TINT a_iGoal )
{
	if ( a_iGoal == -1 )
		return TFALSE;

	AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ a_iGoal ];

	if ( !HASANYFLAG( pGoal->m_eFlags, AInteriorGoal::FLAGS_HAS_EXIT ) )
		return TFALSE;

	TVector2 vecExit( pGoal->m_fExitX, pGoal->m_fExitZ );
	TINT     iArea = FindAreaContainingPointXZ( vecExit );

	return BuildPathToPoint( a_pPath, vecExit, iArea, TNULL );
}

// $Barnyard: FUNCTION 00553670
TBOOL AInteriorNetwork::RebuildPath( AInteriorNetworkPath* a_pPath )
{
	TVector4 vecOldPosition = a_pPath->m_oPath.m_vPosition;

	if ( a_pPath->m_oPath.m_iNumPathAreas != 0 )
	{
		TVector4        vecTarget;
		TVector4        vecForward;
		const TVector4* pForward;
		TINT            iTargetArea;

		if ( a_pPath->m_oPath.m_iGoalIndex == -1 )
		{
			vecTarget   = a_pPath->m_oPath.m_vDestination;
			pForward    = TNULL;
			iTargetArea = a_pPath->m_oPath.GetPathAreaIndex( a_pPath->m_oPath.m_iNumPathAreas - 1 );
		}
		else
		{
			AInteriorGoal* pGoal = &m_pINetwork->m_pGoals[ a_pPath->m_oPath.m_iGoalIndex ];
			pGoal->GetPosition( vecTarget );

			vecForward  = pGoal->m_vForward;
			pForward    = &vecForward;
			iTargetArea = FindContainingArea( vecTarget );
		}

		BuildPathToPosition( a_pPath, vecTarget, iTargetArea, pForward );
	}

	return TVector4::DistanceSqXZ( a_pPath->m_oPath.m_vPosition, vecOldPosition ) > 1e-6f;
}

// $Barnyard: FUNCTION 00553cc0
TBOOL AInteriorNetwork::UpdatePath( AInteriorNetworkPath* a_pPath )
{
	TVector4 vecOldPosition = a_pPath->m_oPath.m_vPosition;

	if ( a_pPath->m_oPath.m_iNumPathAreas != 0 || HASANYFLAG( a_pPath->m_oPath.m_uiFlags, AInteriorNetworkPath::BuildPathState::FLAGS_NOT_FOUND ) )
	{
		if ( a_pPath->m_oPath.m_iSpecialAreaIndex != -1 )
			return RebuildPath( a_pPath );

		TVector2 vecDestination( a_pPath->m_oPath.m_vDestination.x, a_pPath->m_oPath.m_vDestination.z );
		TVector2 vecStart( a_pPath->m_vStartPosition.x, a_pPath->m_vStartPosition.z );

		// Skip the areas the unit already went through
		TINT iNumAreas = a_pPath->m_oPath.m_iNumPathAreas;
		TINT i;

		for ( i = 0; i < iNumAreas; i++ )
		{
			TINT iArea = a_pPath->m_oPath.GetPathAreaIndex( i );

			if ( m_pINetwork->m_pAreas[ iArea ].CheckPointInsideArea( TVector4( vecStart.x, vecStart.y, 0.0f, 0.0f ) ) )
			{
				a_pPath->m_iStartArea = iArea;
				a_pPath->m_oPath.AdvancePath( i );
				break;
			}
		}

		// The unit left the path
		if ( iNumAreas == i )
			return RebuildPath( a_pPath );

		TVector2 vecNext;
		TINT     iNextArea;

		if ( FindNextPathPoint( &a_pPath->m_oPath, a_pPath->m_fRadius, vecStart, vecDestination, vecNext, &iNextArea ) )
		{
			TVector4 vecNextPosition( vecNext.x, 0.0f, vecNext.y, 1.0f );
			ResolvePointHeight( iNextArea, vecNextPosition, 0 );

			a_pPath->m_oPath.m_vPosition = vecNextPosition;
		}
	}

	return TVector4::DistanceSqXZ( a_pPath->m_oPath.m_vPosition, vecOldPosition ) > 1e-6f;
}

// $Barnyard: FUNCTION 005519a0
TBOOL AInteriorNetwork::IsVisibleFromCamera( const Toshi::TVector4& a_rcPosition )
{
	const TMatrix44& rcCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();

	TFLOAT fDiffX      = rcCamera.m_f41 - a_rcPosition.x;
	TFLOAT fDiffZ      = rcCamera.m_f43 - a_rcPosition.z;
	TFLOAT fDistanceSq = fDiffX * fDiffX + fDiffZ * fDiffZ;

	if ( fDistanceSq >= 10000.0f )
		return TFALSE;

	if ( fDistanceSq <= 1.0f )
		return TTRUE;

	TVector2 vecToPosition( a_rcPosition.x - rcCamera.m_f41, a_rcPosition.z - rcCamera.m_f43 );

	if ( vecToPosition.y * rcCamera.m_f33 + vecToPosition.x * rcCamera.m_f31 <= 0.0f )
		return TFALSE;

	vecToPosition.Normalize();
	return vecToPosition.y * rcCamera.m_f33 + vecToPosition.x * rcCamera.m_f31 > 0.3f;
}

// $Barnyard: FUNCTION 00553e70
void AInteriorNetwork::UpdateHardAvoidance( AInteriorNetworkPath* a_pPath, Toshi::TVector4& a_rPosition, Toshi::TVector2* a_pOutPush, TBOOL a_bResolveHeight )
{
	TVector2 vecPush;

	if ( !a_pOutPush )
		a_pOutPush = &vecPush;

	AInteriorMoveQuery oQuery;
	oQuery.m_vStart.x  = a_pPath->m_vStartPosition.x;
	oQuery.m_vStart.y  = a_pPath->m_vStartPosition.z;
	oQuery.m_vTarget.x = a_rPosition.x;
	oQuery.m_vTarget.y = a_rPosition.z;
	oQuery.m_fRadius   = a_pPath->m_fRadius;
	*a_pOutPush        = TVector2::VEC_ZERO;
	oQuery.m_iArea     = a_pPath->m_iStartArea;

	oQuery.m_iNumCrossings = 0;

	// Bit set of the areas that were checked
	TUINT32 aVisitedAreas[ 18 ] = {};

	TINT  iNumSlides     = 0;
	TINT  iNumIterations = 0;
	TBOOL bSlid;

	do
	{
		bSlid = TFALSE;
		oQuery.Reset();

		for ( TINT i = 0; i < TARRAYSIZE( aVisitedAreas ); i++ )
			aVisitedAreas[ i ] = 0;

		m_pINetwork->m_pAreas[ oQuery.m_iArea ].CollideMove( oQuery, aVisitedAreas );

		if ( oQuery.m_bHit )
		{
			TVector2 vecNewTarget;

			if ( oQuery.m_bSlide )
			{
				// Slide along the hit edge
				iNumSlides += 1;
				bSlid = TTRUE;

				TFLOAT fDistance = ( oQuery.m_vTarget.x - oQuery.m_vHitPosition.x ) * oQuery.m_vSlideNormal.y - ( oQuery.m_vTarget.y - oQuery.m_vHitPosition.y ) * oQuery.m_vSlideNormal.x;

				vecNewTarget.x = oQuery.m_vHitNormal.y * fDistance + oQuery.m_vHitPosition.x;
				vecNewTarget.y = -oQuery.m_vHitNormal.x * fDistance + oQuery.m_vHitPosition.y;

				a_pOutPush->x = oQuery.m_vHitNormal.x + a_pOutPush->x;
				a_pOutPush->y = oQuery.m_vHitNormal.y + a_pOutPush->y;
			}
			else
			{
				vecNewTarget = oQuery.m_vHitPosition;
			}

			oQuery.m_vStart  = oQuery.m_vHitPosition;
			oQuery.m_vTarget = vecNewTarget;
			oQuery.m_iArea   = oQuery.GetEndArea();
		}
	} while ( bSlid && ++iNumIterations < 5 );

	if ( iNumSlides > 0 )
	{
		TFLOAT fScale = 1.0f / TFLOAT( iNumSlides );
		a_pOutPush->x = fScale * a_pOutPush->x * fScale;
		a_pOutPush->y = fScale * a_pOutPush->y * fScale;
	}

	TINT iArea    = oQuery.GetEndArea();
	a_rPosition.z = oQuery.m_vHitPosition.y;
	a_rPosition.x = oQuery.m_vHitPosition.x;

	if ( a_bResolveHeight && IsVisibleFromCamera( a_rPosition ) )
		ResolvePointHeight( iArea, a_rPosition, a_pPath->m_uiHeightFlags );

	a_pPath->m_iStartArea = iArea;

	if ( a_pPath->m_oPath.m_iNumPathAreas > 1 && a_pPath->m_oPath.GetPathAreaIndex( 1 ) == iArea )
		a_pPath->m_oPath.AdvancePath( 1 );

	a_pPath->m_vStartPosition = a_rPosition;
}

// $Barnyard: FUNCTION 00551790
TBOOL AInteriorNetwork::IsInsideStartArea( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcPosition, TBOOL a_bOnlyOuterEdges, TFLOAT a_fRadiusScale )
{
	return m_pINetwork->m_pAreas[ a_pPath->m_iStartArea ].IsCircleAtPositionInsideArea( a_rcPosition, a_pPath->m_fRadius * a_fRadiusScale, a_bOnlyOuterEdges );
}

// $Barnyard: FUNCTION 005516a0
TBOOL AInteriorNetwork::SnapPathToGoal( AInteriorNetworkPath* a_pPath, AInteriorGoal* a_pGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing )
{
	a_pGoal->GetPosition( a_rOutPosition );

	a_rOutFacing   = a_pGoal->m_vForward;
	a_rOutFacing.y = 0.0f;
	a_rOutFacing.Normalise();

	a_pPath->SetStart( a_rOutPosition, a_pGoal->m_iAreaIndex );
	a_pPath->m_oPath.Reset();
	a_pPath->m_oPath.PushPathArea( a_pGoal->m_iAreaIndex );
	a_pPath->m_oPath.SetState( a_rOutPosition, a_rOutPosition, a_rOutFacing );

	return TTRUE;
}

// $Barnyard: FUNCTION 00553840
TBOOL AInteriorNetwork::SnapPathToRandomVisibleGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing )
{
	AInteriorGoal* pGoal = FindRandomVisibleGoal( a_strGoal );

	if ( pGoal )
		return SnapPathToGoal( a_pPath, pGoal, a_rOutPosition, a_rOutFacing );

	return TFALSE;
}

// $Barnyard: FUNCTION 005515c0
TBOOL AInteriorNetwork::SnapPathToArea( AInteriorNetworkPath* a_pPath, TINT a_iArea, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing )
{
	a_pPath->m_oPath.Reset();

	m_pINetwork->m_pAreas[ a_iArea ].GetPosition( a_rOutPosition );
	a_rOutFacing = TVector4::VEC_POSX;

	a_pPath->SetStart( a_rOutPosition, a_iArea );
	return TTRUE;
}

//-----------------------------------------------------------------------------
// Areas
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00550fc0
TINT AInteriorNetwork::FindAreaContainingPointXZ( const Toshi::TVector2& a_rcPoint ) const
{
	for ( TINT i = 0; i < m_pINetwork->m_iNumAreas; i++ )
	{
		if ( m_pINetwork->m_pAreas[ i ].CheckPointInsideArea( TVector4( a_rcPoint.x, a_rcPoint.y, 0.0f, 0.0f ) ) )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005513e0
TINT AInteriorNetwork::FindAreaFittingCircle( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TINT a_iHintArea ) const
{
	TINT iNumAreas = m_pINetwork->m_iNumAreas;

	if ( a_iHintArea >= 0 && a_iHintArea < iNumAreas && m_pINetwork->m_pAreas[ a_iHintArea ].IsCircleAtPositionInsideArea( a_rcPosition, a_fRadius, TFALSE ) )
		return a_iHintArea;

	for ( TINT i = 0; i < iNumAreas; i++ )
	{
		if ( m_pINetwork->m_pAreas[ i ].IsCircleAtPositionInsideArea( a_rcPosition, a_fRadius, TFALSE ) )
			return i;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Goals and units
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005530a0
AInteriorGoal* AInteriorNetwork::FindGoalByIndex( Toshi::TPString8 a_strName, TINT a_iIndex ) const
{
	auto it = m_pGoalNameLookup->Find( a_strName );

	if ( it == m_pGoalNameLookup->End() || it->second.iCount <= a_iIndex )
		return TNULL;

	return &m_pINetwork->m_pGoals[ it->second.pIndices[ a_iIndex ] ];
}

// $Barnyard: FUNCTION 00551770
TBOOL AInteriorNetwork::CanUnitUseGoal( TINT a_iGoal, AUnit* a_pUnit )
{
	return m_pINetwork->m_pGoals[ a_iGoal ].CanUnitUse( 0, a_pUnit );
}

// $Barnyard: FUNCTION 00551510
void AInteriorNetwork::AddUnitObstruction( AUnit* a_pUnit )
{
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	if ( pObstruction && !m_vecObstructions.FindItem( pObstruction ) )
		m_vecObstructions.PushBack( pObstruction );
}

// $Barnyard: FUNCTION 00551540
void AInteriorNetwork::RemoveUnitObstruction( AUnit* a_pUnit )
{
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	if ( pObstruction && m_vecObstructions.FindItem( pObstruction ) != TNULL )
		m_vecObstructions.FindAndErase( pObstruction );
}

// $Barnyard: FUNCTION 00551570
TBOOL AInteriorNetwork::HasUnitObstruction( AUnit* a_pUnit )
{
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	return m_vecObstructions.FindItem( pObstruction ) != TNULL;
}

// $Barnyard: FUNCTION 00551590
TBOOL AInteriorNetwork::IsUnitConnected( AUnit* a_pUnit )
{
	AKinematicsObstruction* pObstruction = TSTATICCAST( AKinematicsObstruction, a_pUnit->GetObstruction() );

	return pObstruction && m_vecObstructions.FindItem( pObstruction ) != TNULL;
}
