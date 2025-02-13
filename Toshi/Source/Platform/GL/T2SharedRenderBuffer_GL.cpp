#include "ToshiPCH.h"
#include "T2SharedRenderBuffer_GL.h"
#include "T2Render_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2SharedVertexBuffer::T2SharedVertexBuffer()
{
	m_iNumVertices     = 0;
	m_iVertexSize      = 0;
	m_iNumFreeVertices = 0;
}

T2SharedVertexBuffer::~T2SharedVertexBuffer()
{
	if ( IsCreated() )
		Destroy();
}

void T2SharedVertexBuffer::Create( TINT a_iNumVertices, TINT a_iVertexSize )
{
	TASSERT( !IsCreated() );
	
	m_oVertexBuffer    = T2Render::CreateVertexBuffer( TNULL, a_iNumVertices * a_iVertexSize, GL_STATIC_DRAW );
	m_iNumVertices     = a_iNumVertices;
	m_iVertexSize      = a_iVertexSize;
	m_iNumFreeVertices = a_iNumVertices;
}

void T2SharedVertexBuffer::Destroy()
{
	TASSERT( IsCreated() );

	T2Render::DestroyVertexBuffer( m_oVertexBuffer );
	m_iNumVertices     = 0;
	m_iVertexSize      = 0;
	m_iNumFreeVertices = 0;

	m_oVertexBuffer.Clear();
}

TBOOL T2SharedVertexBuffer::Allocate( TINT a_iNumVertices, SubBuffer& a_rSubBuffer )
{
	if ( m_iNumFreeVertices < a_iNumVertices )
		return TFALSE;

	TINT iPrevOffset     = 0;
	TINT iNextOffset     = 0;
	TINT iHoleSize       = m_iNumFreeVertices;
	TINT iNumAllocations = m_vecAllocations.Size();

	// Find a hole that can fit the allocation
	AllocationInfo* pPrevAllocation = TNULL;
	for ( TINT i = 0; i < iNumAllocations; i++ )
	{
		AllocationInfo* pAllocation = &m_vecAllocations[ i ];
		
		iHoleSize       = pAllocation->offset - iPrevOffset;
		iPrevOffset     = pAllocation->offset;
		pPrevAllocation = pAllocation;

		if ( iHoleSize >= a_iNumVertices )
			break;

		iNextOffset = iPrevOffset + pAllocation->size;
	}

	// If reached the end of the list, check if the allocation can fit at the end of the buffer
	if ( pPrevAllocation && iHoleSize < a_iNumVertices && pPrevAllocation == &m_vecAllocations[ iNumAllocations - 1 ] )
	{
		iHoleSize = m_iNumVertices - ( pPrevAllocation->offset + pPrevAllocation->size );

		if ( iHoleSize < a_iNumVertices )
			return TFALSE;
	}

	TASSERT( iHoleSize >= a_iNumVertices );

	// Fill allocation info and store in the sorted vector
	a_rSubBuffer.vbo    = this;
	a_rSubBuffer.offset = iNextOffset;
	a_rSubBuffer.size   = a_iNumVertices;
	m_vecAllocations.Push( a_rSubBuffer );

	// Update number of free vertices
	m_iNumFreeVertices -= a_iNumVertices;

	return TTRUE;
}

void T2SharedVertexBuffer::Deallocate( SubBuffer& a_rAllocation )
{
	m_vecAllocations.FindAndErase( a_rAllocation );
	m_iNumFreeVertices += a_rAllocation.size;
}

TOSHI_NAMESPACE_END
