#pragma once
#include "Render/ALineShader/ALineShader.h"

#include <d3d11.h>

class MODAPI ALineMaterialHAL
    : public ALineMaterial
{
public:
	TDECLARE_CLASS( ALineMaterialHAL, ALineMaterial );

public:
	ALineMaterialHAL() = default;
	~ALineMaterialHAL();
};

class MODAPI ALineMeshHAL
    : public ALineMesh
{
public:
	TDECLARE_CLASS( ALineMeshHAL, ALineMesh );

public:
	ALineMeshHAL() = default;
	~ALineMeshHAL();
};

class MODAPI ALineShaderHAL
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
	void DrawVertices( ID3D11Buffer* a_pVertexBuffer, D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const Vertex* a_pVertices, TUINT a_uiNumVertices );

private:
	ID3D11Buffer*       m_pLineVertexBuffer;
	ID3D11Buffer*       m_pTriangleVertexBuffer;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11PixelShader*  m_pPixelShader;
	ID3D11InputLayout*  m_pInputLayout;
	TUINT               m_uiNumLineVertices;
	TUINT               m_uiNumTriangleVertices;
	Vertex              m_aLineVertices[ MAX_NUM_VERTICES ];
	Vertex              m_aTriangleVertices[ MAX_NUM_VERTICES ];
};
