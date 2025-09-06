#include "ToshiPCH.h"
#include "TModelCollision.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TModelCollisionData::TModelCollisionData()
    : m_vecCollGroups( GetGlobalAllocator(), 64, 64 )
    , m_pVertices( TNULL )
    , m_pIndices( TNULL )
    , m_uiNumIndices( 0 )
{
}

// $Barnyard: FUNCTION 006ce390
// $Barnyard: FUNCTION 006ce6d0
TModelCollisionData::~TModelCollisionData()
{
	delete[] m_pVertices;
	m_pVertices     = TNULL;
	m_uiNumVertices = 0;

	delete[] m_pIndices;
	m_pIndices     = TNULL;
	m_uiNumIndices = 0;
}

// $Barnyard: FUNCTION 006cded0
void TModelCollisionData::GetTriangleVertices( TUINT a_uiFaceId, Toshi::TVector3* a_rVertices[ 3 ] )
{
	TASSERT( a_uiFaceId * 3 < m_uiNumIndices );

	TUINT16* pIndex  = m_pIndices + a_uiFaceId * 3;
	a_rVertices[ 0 ] = &m_pVertices[ pIndex[ 0 ] ];
	a_rVertices[ 1 ] = &m_pVertices[ pIndex[ 1 ] ];
	a_rVertices[ 2 ] = &m_pVertices[ pIndex[ 2 ] ];
}

// $Barnyard: FUNCTION 006ce100
TINT TModelCollisionData::GetGroupForTriangleImp( TUINT a_uiFaceId )
{
	TUINT uiFaceCounter = 0;

	for ( TINT i = 0; i < m_vecCollGroups.Size(); i++ )
	{
		uiFaceCounter += m_vecCollGroups[ i ].uiNumFaces;

		if ( a_uiFaceId < uiFaceCounter )
			return i;
	}

	return 0;
}

TModelCollisionGroup::TModelCollisionGroup()
    : vecS1( GetGlobalAllocator(), 64, 64 )
{
}

TModelCollisionGroup::~TModelCollisionGroup()
{
}

TOSHI_NAMESPACE_END
