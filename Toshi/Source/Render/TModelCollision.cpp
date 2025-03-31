#include "ToshiPCH.h"
#include "TModelCollision.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TModelCollisionMesh::TModelCollisionMesh()
    : m_vecCollTypes( GetGlobalAllocator(), 64, 64 )
    , m_pVertices( TNULL )
    , m_pIndices( TNULL )
    , m_uiNumIndices( 0 )
{
}

TModelCollisionMesh::~TModelCollisionMesh()
{
}

TModelCollisionType::TModelCollisionType()
    : vecS1( GetGlobalAllocator(), 64, 64 )
{
}

TModelCollisionType::~TModelCollisionType()
{
}

TOSHI_NAMESPACE_END
