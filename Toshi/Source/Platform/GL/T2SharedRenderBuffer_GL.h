#pragma once
#include "T2RenderBuffer_GL.h"

#include <ToshiTools/T2DynamicVector.h>
#include <Toshi/T2SortedVector.h>

TOSHI_NAMESPACE_START

class T2SharedVertexBuffer
{
public:
	struct AllocationInfo
	{
		TINT offset;
		TINT size;

		TBOOL operator==( const AllocationInfo& a_rcOther ) const
		{
			return offset == a_rcOther.offset && size == a_rcOther.size;
		}
	};

	struct AllocationInfoSort
	{
		TINT operator()( const AllocationInfo& a_rcVal1, const AllocationInfo& a_rcVal2 ) const
		{
			return a_rcVal1.offset - a_rcVal2.offset;
		}
	};

	struct SubBuffer
		: public AllocationInfo
	{
		T2SharedVertexBuffer* vbo;

		T2SharedVertexBuffer* operator->() { return vbo; }
	};

public:
	T2SharedVertexBuffer();
	~T2SharedVertexBuffer();

	void Create( TINT a_uiNumVertices, TINT a_uiVertexSize );
	void Destroy();

	TBOOL Allocate( TINT a_uiNumVertices, SubBuffer& a_rSubBuffer );
	void  Deallocate( SubBuffer& a_rAllocation );

	TBOOL IsCreated() const { return m_oVertexBuffer; }

private:
	T2VertexBuffer m_oVertexBuffer;
	TINT           m_iNumVertices;
	TINT           m_iVertexSize;
	TINT           m_iNumFreeVertices;

	T2SortedVector<AllocationInfo, T2DynamicVector<AllocationInfo>, AllocationInfoSort> m_vecAllocations;
};

TOSHI_NAMESPACE_END
