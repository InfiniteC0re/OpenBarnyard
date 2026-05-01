#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AInteriorNetwork );

// $Barnyard: FUNCTION 00553100
AInteriorNetwork::AInteriorNetwork()
{
	m_pTRB              = TNULL;
	m_pINetwork         = TNULL;
	m_strName           = Toshi::TPString8();
	m_bSetHeightFromEnv = TTRUE;

	m_Unk1 = 0;
	m_Unk2 = TFALSE;

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
		delete m_pUpdateGoalInterfaces;

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

// $Barnyard: FUNCTION 00554ff0
TBOOL AInteriorNetwork::LoadFromFile( const Toshi::TPString8& a_rcNetworkName, const TCHAR* a_szFilePath )
{
	TASSERT( !"To be implemented" );
	return TFALSE;
}

// $Barnyard: FUNCTION 00551d90
void AInteriorNetwork::ResolvePointHeight( TINT a_iAreaIndex, Toshi::TVector4& a_rPoint, TUINT a_uiFlags )
{
	TASSERT( !"To be implemented" );
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

// $Barnyard: FUNCTION 00552c20
AInteriorGoal* AInteriorNetwork::FindGoalInArea( const Toshi::TPString8& a_rcName, TINT a_iAreaIndex /*= -1*/ ) const
{
	auto it = m_pGoalNameLookup->Find( a_rcName );

	if ( it != m_pGoalNameLookup->End() )
	{
		for ( TINT i = 0; i < it->second.iCount; i++ )
		{
			if ( a_iAreaIndex < 0 || m_pINetwork->m_pGoals[ i ].m_iAreaIndex == a_iAreaIndex )
				return &m_pINetwork->m_pGoals[ i ];
		}
	}

	TERROR( "Couldn't find goal %s in area %d\n", a_rcName.GetString(), a_iAreaIndex );
	return TNULL;
}

// $Barnyard: FUNCTION 00552ca0
TBOOL AInteriorNetwork::HasGoal( const Toshi::TPString8& a_rcName ) const
{
	return m_pGoalNameLookup->Find( a_rcName ) != m_pGoalNameLookup->End();
}

// $Barnyard: FUNCTION 00552e90
AInteriorGoal* AInteriorNetwork::FindRandomGoal( const Toshi::TPString8& a_rcName ) const
{
	TASSERT( !"To be implemented" );
	return TNULL;
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
				it->second.iCount = 0;
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
				it->second.iCount  = 0;
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
			m_vecGroups->Erase( a_pGoalInterface );
			return TTRUE;
		}
	}

	return TFALSE;
}

// T2Map methods:
// $Barnyard: FUNCTION 00554680
// $Barnyard: FUNCTION 005545c0
// $Barnyard: FUNCTION 00553b50

// T2SList methods:
// $Barnyard: FUNCTION 00553b50
