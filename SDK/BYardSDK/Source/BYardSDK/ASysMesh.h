#pragma once
#include <Toshi/TString8.h>

#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class ASysMesh : public Toshi::TMesh
{
public:
	struct LockBuffer
	{
		Toshi::TVertexPoolResourceInterface::LockBuffer VertexLock;
		Toshi::TIndexPoolResourceInterface::LockBuffer  IndexLock;
	};

public:
	ASysMesh();

	virtual TBOOL                                Validate() override;
	virtual void                                 Invalidate() override;
	virtual TBOOL                                Create( void* a_pUnk, const char* a_szSomeString );
	virtual TBOOL                                CreatePools( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices );
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );
	virtual void                                 Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices );
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();
	virtual Toshi::TIndexPoolResourceInterface*  GetIndexPool();
	virtual void                                 SetZBias( TINT a_iZBias ) = 0;

	TBOOL IsLocked() const { return m_uiFlags & 0x8000; }

private:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	TUINT16                              m_uiMaxIndices;
	Toshi::TString8                      m_SomeString;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	Toshi::TIndexPoolResourceInterface*  m_pIndexPool;
	TBOOL                                m_bFlag1;
	TBOOL                                m_bFlag2;
};
