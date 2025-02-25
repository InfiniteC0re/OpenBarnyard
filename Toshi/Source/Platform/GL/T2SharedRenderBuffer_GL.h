#pragma once
#include "T2Render_GL.h"

#include <ToshiTools/T2DynamicVector.h>
#include <Toshi/T2SortedVector.h>

TOSHI_NAMESPACE_START

template <class RenderBuffer>
class T2SharedRenderBuffer
{
public:
	struct SubBuffer
	{
		TINT                  offset;
		TUINT                 size;
		T2SharedRenderBuffer* owner;

		void SetData( const void* a_pData, GLsizeiptr a_iSize )
		{
			TASSERT( a_iSize <= GLsizeiptr( size ) );
			glNamedBufferSubData( owner->m_oRenderBuffer.GetId(), offset, a_iSize, a_pData );
		}

		void Deallocate();
	};

	struct SubBufferSort
	{
		TINT operator()( SubBuffer* const& a_rcVal1, SubBuffer* const& a_rcVal2 ) const
		{
			return a_rcVal1->offset - a_rcVal2->offset;
		}
	};

public:
	T2SharedRenderBuffer()
	{
		m_iTotalSize = 0;
		m_iFreeSize  = 0;
	}

	~T2SharedRenderBuffer()
	{
		if ( IsCreated() )
			Destroy();
	}

	void Create( RenderBuffer&& a_rRenderBuffer, TINT a_iSize )
	{
		TASSERT( !IsCreated() );

		m_oRenderBuffer = std::move( a_rRenderBuffer );
		m_oRenderBuffer.Bind();
		m_oRenderBuffer.SetData( TNULL, a_iSize, GL_DYNAMIC_DRAW );
		m_oRenderBuffer.Unbind();

		m_iTotalSize = a_iSize;
		m_iFreeSize  = a_iSize;
	}

	void Destroy()
	{
		TASSERT( IsCreated() );

		GLuint uiId = m_oRenderBuffer.GetId();
		glDeleteBuffers( 1, &uiId );
		m_iTotalSize = 0;
		m_iFreeSize  = 0;

		m_oRenderBuffer.Clear();
	}

	SubBuffer* Allocate( TINT a_iSize )
	{
		if ( m_iFreeSize < a_iSize )
			return TNULL;

		TINT iPrevOffset     = 0;
		TINT iNextOffset     = 0;
		TINT iHoleSize       = m_iFreeSize;
		TINT iPrevSize       = 0;
		TINT iNumAllocations = m_vecAllocations.Size();

		// Find a hole that can fit the allocation
		SubBuffer* pPrevAllocation = TNULL;
		for ( TINT i = 0; i < iNumAllocations; i++ )
		{
			SubBuffer* pAllocation = m_vecAllocations[ i ];

			iHoleSize       = pAllocation->offset - iPrevOffset - iPrevSize;
			iPrevOffset     = pAllocation->offset;
			iPrevSize       = pAllocation->size;
			pPrevAllocation = pAllocation;

			if ( iHoleSize >= a_iSize )
				break;

			iNextOffset = iPrevOffset + pAllocation->size;
		}

		// If reached the end of the list, check if the allocation can fit at the end of the buffer
		if ( pPrevAllocation && iHoleSize < a_iSize && pPrevAllocation == m_vecAllocations[ iNumAllocations - 1 ] )
		{
			iHoleSize = m_iTotalSize - ( pPrevAllocation->offset + pPrevAllocation->size );

			if ( iHoleSize < a_iSize )
				return TNULL;
		}

		TASSERT( iHoleSize >= a_iSize );

		// Fill allocation info and store in the sorted vector
		SubBuffer* pSubBuffer = new SubBuffer();
		pSubBuffer->owner     = this;
		pSubBuffer->offset    = iNextOffset;
		pSubBuffer->size      = a_iSize;
		m_vecAllocations.Push( pSubBuffer );

		// Update number of free vertices
		m_iFreeSize -= a_iSize;

		return pSubBuffer;
	}

	void Deallocate( SubBuffer* a_pAllocation )
	{
		if ( a_pAllocation )
		{
			m_vecAllocations.FindAndErase( a_pAllocation );
			m_iFreeSize += a_pAllocation->size;

			delete a_pAllocation;
		}
	}

	RenderBuffer& GetRenderBuffer() { return m_oRenderBuffer; }
	TBOOL IsCreated() const { return m_oRenderBuffer; }

private:
	RenderBuffer   m_oRenderBuffer;
	TINT           m_iTotalSize;
	TINT           m_iFreeSize;

	T2SortedVector<SubBuffer*, T2DynamicVector<SubBuffer*>, SubBufferSort> m_vecAllocations;
};

template <class RenderBuffer>
inline void T2SharedRenderBuffer<RenderBuffer>::SubBuffer::Deallocate()
{
	owner->Deallocate( this );
}

using T2SharedVertexBuffer = T2SharedRenderBuffer<T2VertexBuffer>;
using T2SharedIndexBuffer = T2SharedRenderBuffer<T2IndexBuffer>;

TOSHI_NAMESPACE_END
