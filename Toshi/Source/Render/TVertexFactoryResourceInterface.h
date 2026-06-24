#pragma once
#include "TResource.h"
#include "TVertexFactoryFormat.h"

TOSHI_NAMESPACE_START

class TVertexPoolResourceInterface;

class TVertexFactoryResourceInterface : public TResource
{
public:
	TDECLARE_CLASS( TVertexFactoryResourceInterface, TResource );

public:
	friend TVertexPoolResourceInterface;

public:
	TVertexFactoryResourceInterface();

	virtual TBOOL                         Create( TVertexFactoryFormat* a_pVertexFormat, TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags );
	virtual TVertexPoolResourceInterface* CreatePoolResource( TUINT16 a_uiMaxStaticVertices, TUINT16 a_uiFlags ) = 0;

	const TVertexFactoryFormat& GetVertexFormat() const { return m_VertexFormat; }
	TUINT16                     GetMaxStaticVertices() const { return m_uiMaxStaticVertices; }

	void AddVertexStream( TUINT16 a_uiVertexSize )
	{
		TASSERT( m_VertexFormat.m_uiNumStreams < TVertexFactoryFormat::MAX_NUM_STREAMS );
		auto& rStream         = m_VertexFormat.m_aStreamFormats[ m_VertexFormat.m_uiNumStreams ];
		rStream.m_uiVertexSize = a_uiVertexSize;
		rStream.m_Unk          = 0;
		m_VertexFormat.m_uiNumStreams++;
	}

protected:
	TVertexPoolResourceInterface* CreatePool( TUINT16 a_uiMaxStaticVertices, TUINT32 a_uiFlags );

protected:
	TUINT                m_uiNumPools;
	TVertexFactoryFormat m_VertexFormat;
	TUINT16              m_uiMaxStaticVertices;
	TUINT                m_uiFlags;
};

TOSHI_NAMESPACE_END
