#pragma once
#include <Toshi/TString8.h>

#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class ASysMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( ASysMesh, Toshi::TMesh );

	// Flags of CreatePools, they pick the flags of the vertex and index pools (TMesh::FLAG_LOCKED is kept here too)
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_VERTICES_STATIC         = BITFLAG( 0 ), // TVertexPoolResourceInterface::FLAGS_STATIC
		FLAGS_VERTICES_DYNAMIC        = BITFLAG( 1 ), // TVertexPoolResourceInterface::FLAGS_DYNAMIC
		FLAGS_VERTICES_DYNAMIC_APPEND = BITFLAG( 2 ), // TVertexPoolResourceInterface::FLAGS_DYNAMIC_APPEND
		FLAGS_INDICES_STATIC          = BITFLAG( 3 ), // TIndexPoolResourceInterface::FLAGS_STATIC
		FLAGS_INDICES_DYNAMIC         = BITFLAG( 4 ), // TIndexPoolResourceInterface::FLAGS_DYNAMIC
		FLAGS_INDICES_DYNAMIC_APPEND  = BITFLAG( 5 ), // TIndexPoolResourceInterface::FLAGS_DYNAMIC_APPEND
		FLAGS_INDICES_TRISTRIP        = BITFLAG( 6 ), // TIndexPoolResourceInterface::FLAGS_TRISTRIP (FLAGS_TRILIST if not set)

		// Used by every caller
		FLAGS_DEFAULT = FLAGS_VERTICES_DYNAMIC_APPEND | FLAGS_INDICES_DYNAMIC_APPEND | FLAGS_INDICES_TRISTRIP,
	};

public:
	~ASysMesh();

	struct LockBuffer
	{
		Toshi::TVertexPoolResourceInterface::LockBuffer VertexLock;
		Toshi::TIndexPoolResourceInterface::LockBuffer  IndexLock;
	};

	struct Vertex
	{
		Toshi::TVector3 vPosition;
		TUINT32         uiColor;
		Toshi::TVector2 vUV;
	};

public:
	ASysMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL                                Create( void* a_pUnk, const TCHAR* a_szSomeString );
	virtual TBOOL                                CreatePools( FLAGS a_eFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices );
	virtual TBOOL                                Lock( LockBuffer& a_rLockBuffer );
	virtual void                                 Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices );
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();
	virtual Toshi::TIndexPoolResourceInterface*  GetIndexPool();
	virtual void                                 SetZBias( TINT a_iZBias ) = 0;

	TBOOL IsLocked() const { return m_uiFlags & FLAG_LOCKED; }

	TUINT16 GetMaxVertices() const { return m_uiMaxVertices; }
	TUINT16 GetMaxIndices() const { return m_uiMaxIndices; }

private:
	TBOOL CreateResource();
	void  DestroyResources();

private:
	FLAGS                                m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	TUINT16                              m_uiMaxIndices;
	Toshi::TString8                      m_SomeString;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	Toshi::TIndexPoolResourceInterface*  m_pIndexPool;
	TBOOL                                m_bFlag1;
	TBOOL                                m_bFlag2;
};
