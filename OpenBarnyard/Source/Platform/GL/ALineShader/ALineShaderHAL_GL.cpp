#include "pch.h"
#include "ALineShaderHAL_GL.h"

#include <Platform/GL/TRenderInterface_GL.h>
#include <Platform/GL/TRenderContext_GL.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ALineMaterialHAL );

TDEFINE_CLASS( ALineMeshHAL );

ALineMaterialHAL::~ALineMaterialHAL()
{
}

ALineMeshHAL::~ALineMeshHAL()
{
}

TDEFINE_CLASS( ALineShaderHAL );

// The fixed function pipeline for D3DFVF_XYZ | D3DFVF_DIFFUSE vertices, stage 0 modulates with no texture
static const TCHAR s_szLineShader[] = R"(
layout( std140 ) uniform Constants
{
	mat4 g_mModelViewProjection;
	vec4 g_vAlphaTest;
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec3 a_vPosition;
layout( location = 1 ) in vec4 a_vColour;

out vec4 v_vColour;

void main()
{
	gl_Position = ToClipSpace( vec4( a_vPosition, 1.0 ) * g_mModelViewProjection );
	v_vColour   = a_vColour.bgra;
}

#else

in vec4 v_vColour;

void main()
{
	AlphaTest( v_vColour.a, g_vAlphaTest );
	o_vColour = v_vColour;
}

#endif
)";

static const TGLVertexAttribute s_aInputAttributes[] = {
	{ 0, 0, 3, GL_FALSE, GL_FLOAT, 0 },
	{ 1, 0, 4, GL_TRUE, GL_UNSIGNED_BYTE, 12 },
};

static const TGLInputLayout s_oInputLayout = { s_aInputAttributes, TARRAYSIZE( s_aInputAttributes ) };

ALineShaderHAL::ALineShaderHAL()
{
	m_pLineVertexBuffer     = 0;
	m_pTriangleVertexBuffer = 0;
	m_uiProgram             = 0;
	m_uiNumLineVertices     = 0;
	m_uiNumTriangleVertices = 0;
}

ALineShaderHAL::~ALineShaderHAL()
{
}

void ALineShaderHAL::OnDestroy()
{
	Invalidate();
	TShader::OnDestroy();
}

TBOOL ALineShaderHAL::Create()
{
	return ALineShader::Create();
}

TBOOL ALineShaderHAL::Validate()
{
	auto pRender = TRenderGLInterface::Interface();

	if ( !m_pLineVertexBuffer )
	{
		m_pLineVertexBuffer = pRender->CreateBuffer( GL_ARRAY_BUFFER, sizeof( m_aLineVertices ), TNULL, GL_DYNAMIC_DRAW );
		if ( !m_pLineVertexBuffer ) return TFALSE;

		m_uiNumLineVertices = 0;
	}

	if ( !m_pTriangleVertexBuffer )
	{
		m_pTriangleVertexBuffer = pRender->CreateBuffer( GL_ARRAY_BUFFER, sizeof( m_aTriangleVertices ), TNULL, GL_DYNAMIC_DRAW );
		if ( !m_pTriangleVertexBuffer ) return TFALSE;

		m_uiNumTriangleVertices = 0;
	}

	if ( !m_uiProgram )
	{
		m_uiProgram = TRenderGLInterface::CompileProgram( s_szLineShader );

		if ( !m_uiProgram )
		{
			TASSERT( !"Couldn't create the shaders" );
			return TFALSE;
		}
	}

	return TShader::Validate();
}

void ALineShaderHAL::Invalidate()
{
	auto pRender = TRenderGLInterface::Interface();

	pRender->DestroyBuffer( m_pLineVertexBuffer );
	pRender->DestroyBuffer( m_pTriangleVertexBuffer );
	TRenderGLInterface::DestroyProgram( m_uiProgram );

	m_pLineVertexBuffer     = 0;
	m_pTriangleVertexBuffer = 0;
	m_uiProgram             = 0;

	TShader::Invalidate();
}

TBOOL ALineShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ALineShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ALineShaderHAL::DrawVertices( GLuint a_pVertexBuffer, GLenum a_eTopology, const Vertex* a_pVertices, TUINT a_uiNumVertices )
{
	auto pRender = TRenderGLInterface::Interface();

	pRender->UpdateBuffer( a_pVertexBuffer, GL_ARRAY_BUFFER, 0, a_uiNumVertices * sizeof( Vertex ), a_pVertices, a_uiNumVertices * sizeof( Vertex ) );

	const TVector4 vAlphaTest = pRender->GetAlphaTestParams();
	pRender->SetVec4InVSBuffer( 4, &vAlphaTest );

	pRender->DrawNonIndexed( a_eTopology, a_pVertexBuffer, a_uiNumVertices, sizeof( Vertex ), 0, 0 );
}

void ALineShaderHAL::FlushLines()
{
	Validate();

	if ( !m_pLineVertexBuffer && !m_pTriangleVertexBuffer ) return;
	if ( m_uiNumLineVertices == 0 && m_uiNumTriangleVertices == 0 ) return;

	TRenderGLInterface* pRender = TRenderGLInterface::Interface();
	TRenderContextGL* pContext = TRenderContextGL::Upcast( pRender->GetCurrentContext() );

	// World is the model view matrix and view is the identity
	TMatrix44 oModelViewProjection;
	oModelViewProjection.Multiply( pContext->GetProjectionMatrix(), pContext->GetModelViewMatrix() );

	TMatrix44 oModelViewProjectionTransposed;
	oModelViewProjectionTransposed.Transpose( oModelViewProjection );
	pRender->SetVec4InVSBuffer( 0, &oModelViewProjectionTransposed, 4 );

	for ( TUINT i = 0; i < 5; i++ )
		pRender->SetTexture( i, 0 );

	pRender->SetShaders( m_uiProgram, &s_oInputLayout );

	if ( m_pLineVertexBuffer && m_uiNumLineVertices != 0 )
	{
		DrawVertices( m_pLineVertexBuffer, GL_LINES, m_aLineVertices, m_uiNumLineVertices / 2 * 2 );
		m_uiNumLineVertices = 0;
	}

	if ( m_pTriangleVertexBuffer && m_uiNumTriangleVertices != 0 )
	{
		pRender->SetAlphaFunc( GL_ALWAYS );
		pRender->SetBlendOp( GL_FUNC_ADD );
		pRender->SetAlphaBlendEnable( TTRUE );
		pRender->SetSrcBlend( GL_SRC_ALPHA );
		pRender->SetDestBlend( GL_ONE_MINUS_SRC_ALPHA );

		TRenderGLInterface::CULLMODE eCullMode = pRender->GetCullMode();
		pRender->SetCullMode( TRenderGLInterface::CULL_NONE );

		DrawVertices( m_pTriangleVertexBuffer, GL_TRIANGLES, m_aTriangleVertices, m_uiNumTriangleVertices / 3 * 3 );
		m_uiNumTriangleVertices = 0;

		pRender->SetCullMode( eCullMode );
	}
}

void ALineShaderHAL::AddLineDirect( const TVector3& a_rcStart, const TVector3& a_rcEnd, TUINT32 a_uiStartColour, TUINT32 a_uiEndColour )
{
	// NOTE: lines that don't fit are dropped
	if ( m_uiNumLineVertices < MAX_NUM_VERTICES )
	{
		m_aLineVertices[ m_uiNumLineVertices ].uiColour        = a_uiStartColour;
		m_aLineVertices[ m_uiNumLineVertices ].vecPosition     = a_rcStart;
		m_aLineVertices[ m_uiNumLineVertices + 1 ].uiColour    = a_uiEndColour;
		m_aLineVertices[ m_uiNumLineVertices + 1 ].vecPosition = a_rcEnd;
		m_uiNumLineVertices += 2;
	}
}

void ALineShaderHAL::AddTriangle( const TVector3 a_aVertices[ 3 ], TUINT32 a_uiColour )
{
	// NOTE: the original only flushes when the array is full, 40000 isn't a multiple of 3, so the
	// 13334th triangle was written past the end of the array
	if ( m_uiNumTriangleVertices + 3 > MAX_NUM_VERTICES )
		FlushLines();

	m_aTriangleVertices[ m_uiNumTriangleVertices ].uiColour        = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices + 1 ].uiColour    = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices + 2 ].uiColour    = a_uiColour;
	m_aTriangleVertices[ m_uiNumTriangleVertices ].vecPosition     = a_aVertices[ 0 ];
	m_aTriangleVertices[ m_uiNumTriangleVertices + 1 ].vecPosition = a_aVertices[ 1 ];
	m_aTriangleVertices[ m_uiNumTriangleVertices + 2 ].vecPosition = a_aVertices[ 2 ];
	m_uiNumTriangleVertices += 3;
}
