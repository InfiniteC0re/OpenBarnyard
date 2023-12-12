#pragma once
#include "TResource.h"
#include "TVertexFactoryFormat.h"

namespace Toshi {

	class TVertexPoolResourceInterface;

	TOBJECT(TVertexFactoryResourceInterface, TResource, TFALSE)
	{
	public:
		friend TVertexPoolResourceInterface;

	public:
		TVertexFactoryResourceInterface();
		
		virtual TBOOL Create(TVertexFactoryFormat* a_pVertexFormat, TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags);
		virtual TVertexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags) = 0;

		const TVertexFactoryFormat& GetVertexFormat() const { return m_VertexFormat; }
		TUINT16 GetMaxStaticVertices() const { return m_uiMaxStaticVertices; }

	protected:
		TVertexPoolResourceInterface* CreatePool(TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags);

	protected:
		TUINT m_uiNumPools;
		TVertexFactoryFormat m_VertexFormat;
		TUINT16 m_uiMaxStaticVertices;
		TUINT m_uiFlags;
	};

}
