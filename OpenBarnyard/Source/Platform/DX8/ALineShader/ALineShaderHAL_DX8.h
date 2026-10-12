#pragma once
#include "Render/ALineShader/ALineShader.h"

#include <d3d8.h>

class ALineMaterialHAL
    : public ALineMaterial
{
public:
	TDECLARE_CLASS( ALineMaterialHAL, ALineMaterial );

public:
	ALineMaterialHAL() = default;
	~ALineMaterialHAL();
};

class ALineMeshHAL
    : public ALineMesh
{
public:
	TDECLARE_CLASS( ALineMeshHAL, ALineMesh );

public:
	ALineMeshHAL() = default;
	~ALineMeshHAL();
};

class ALineShaderHAL
    : public ALineShader
{
public:
	TDECLARE_CLASS( ALineShaderHAL, ALineShader );

	static constexpr TUINT MAX_NUM_VERTICES = 40000;

	struct Vertex
	{
		Toshi::TVector3 vecPosition;
		TUINT32         uiColour;
	};

public:
	ALineShaderHAL();
	~ALineShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() override;
	virtual void  Flush() override {}      // DOES_NOTHING
	virtual void  StartFlush() override {} // DOES_NOTHING
	virtual void  EndFlush() override {}   // DOES_NOTHING
	virtual TBOOL Create() override;
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) override {} // DOES_NOTHING

	//-----------------------------------------------------------------------------
	// ALineShader
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) override { return TNULL; } // RETURNS_ZERO
	virtual Toshi::TMesh*     CreateMesh( const TCHAR* a_szName ) override { return TNULL; }     // RETURNS_ZERO
	virtual void              FlushLines() override;
	virtual void              AddLineDirect( const Toshi::TVector3& a_rcStart, const Toshi::TVector3& a_rcEnd, TUINT32 a_uiStartColour, TUINT32 a_uiEndColour ) override;
	virtual void              AddTriangle( const Toshi::TVector3 a_aVertices[ 3 ], TUINT32 a_uiColour ) override;

private:
	IDirect3DVertexBuffer8* m_pLineVertexBuffer;                     // 0x0C
	IDirect3DVertexBuffer8* m_pTriangleVertexBuffer;                 // 0x10
	TUINT                   m_uiNumLineVertices;                     // 0x14
	TUINT                   m_uiNumTriangleVertices;                 // 0x18
	Vertex                  m_aLineVertices[ MAX_NUM_VERTICES ];     // 0x1C
	Vertex                  m_aTriangleVertices[ MAX_NUM_VERTICES ]; // 0x9C41C
};
