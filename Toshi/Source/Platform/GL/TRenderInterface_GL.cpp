#include "ToshiPCH.h"
#include "TModel_GL.h"
#include "TRenderCapture_GL.h"
#include "TRenderContext_GL.h"
#include "TRenderAdapter_GL.h"
#include "TRenderInterface_GL.h"
#include "TDebugText_GL.h"
#include "TTextureFactoryHAL_GL.h"
#include "TTextureResourceHAL_GL.h"

#include "Render/TShader.h"
#include "Input/TInputInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TRenderGLInterface );

#ifdef TOSHI_SKU_WASM
static const TCHAR s_szShaderVersion[] = "#version 300 es\n";
#else
static const TCHAR s_szShaderVersion[] = "#version 330 core\n";
#endif

// Prepended to every shader
static const TCHAR s_szShaderCommon[] = R"(
precision highp float;
precision highp int;

#define saturate( X ) clamp( X, 0.0, 1.0 )

#ifdef VERTEX_SHADER

layout( std140 ) uniform ViewportConstants
{
	vec4 g_vHalfPixel;
};

// D3D8's pixel centres are on integer coordinates and its depth range is 0 to 1
vec4 ToClipSpace( vec4 a_vPosition )
{
	return vec4( a_vPosition.xy + g_vHalfPixel.xy * a_vPosition.w, a_vPosition.z * 2.0 - a_vPosition.w, a_vPosition.w );
}

#else

out vec4 o_vColour;

#ifdef NO_ALPHA_TEST

#define AlphaTest( FALPHA, VPARAMS )

#else

// Discards the pixel when a_fAlpha fails the alpha test of TRenderGLInterface::GetAlphaTestParams
void AlphaTest( float a_fAlpha, vec4 a_vParams )
{
	if ( a_vParams.x == 0.0 ) return;

	float fRef  = a_vParams.y;
	int   iFunc = int( a_vParams.z );

	bool bPass = ( iFunc == 8 ) || ( iFunc == 2 && a_fAlpha < fRef ) || ( iFunc == 3 && a_fAlpha == fRef ) ||
	    ( iFunc == 4 && a_fAlpha <= fRef ) || ( iFunc == 5 && a_fAlpha > fRef ) || ( iFunc == 6 && a_fAlpha != fRef ) ||
	    ( iFunc == 7 && a_fAlpha >= fRef );

	if ( !bPass ) discard;
}

#endif

#endif
)";

static const TCHAR s_szFixedFunctionShader[] = R"(
layout( std140 ) uniform Constants
{
	layout( row_major ) mat4 g_mWorldViewProjection;
	vec4 g_vParams; // alpha test (xyz), modulate with the texture (w)
};

#ifdef VERTEX_SHADER

layout( location = 0 ) in vec3 a_vPosition;
layout( location = 1 ) in vec4 a_vColour;
layout( location = 2 ) in vec2 a_vUV;

out vec4 v_vColour;
out vec2 v_vUV;

void main()
{
	gl_Position = ToClipSpace( vec4( a_vPosition, 1.0 ) * g_mWorldViewProjection );
	v_vColour   = a_vColour.bgra;
	v_vUV       = a_vUV;
}

#else

in vec4 v_vColour;
in vec2 v_vUV;

uniform sampler2D g_Texture0;

void main()
{
	vec4 vColour = v_vColour;

	if ( g_vParams.w != 0.0 )
		vColour *= texture( g_Texture0, v_vUV );

	AlphaTest( vColour.a, g_vParams );
	o_vColour = vColour;
}

#endif
)";

struct FixedFunctionVertex
{
	TFLOAT  aPosition[ 3 ];
	TUINT32 uiColour;
	TFLOAT  aUV[ 2 ];
};

static const TGLVertexAttribute s_aFixedFunctionAttributes[] = {
	{ 0, 0, 3, GL_FALSE, GL_FLOAT, 0 },
	{ 1, 0, 4, GL_TRUE, GL_UNSIGNED_BYTE, 12 },
	{ 2, 0, 2, GL_FALSE, GL_FLOAT, 16 },
};

const TGLInputLayout TRenderGLInterface::ms_oFixedFunctionLayout  = { s_aFixedFunctionAttributes, 3 };
const TGLInputLayout TRenderGLInterface::ms_oPositionColourLayout = { s_aFixedFunctionAttributes, 2 };

static T2Map<GLuint, GLint> s_oProgramBlockSizes;

// Tiled GPUs lose their hidden surface removal on a shader that can discard, even if it never does. The programs that
// can't get the same one without, used for the textures that always pass the alpha test
static T2Map<GLuint, GLuint> s_oNoAlphaTestPrograms;

// Textures we decoded ourselves that have no pixels at or below this alpha
static constexpr TUINT8 ALPHA_TEST_MAX_REF = 128;
static TUINT8           s_aOpaqueTextures[ 0x2000 ];

static TBOOL IsTextureOpaque( GLuint a_uiTexture )
{
	return a_uiTexture < sizeof( s_aOpaqueTextures ) * 8 && ( s_aOpaqueTextures[ a_uiTexture >> 3 ] & ( 1 << ( a_uiTexture & 7 ) ) );
}

static void SetTextureOpaque( GLuint a_uiTexture, TBOOL a_bOpaque )
{
	if ( a_uiTexture >= sizeof( s_aOpaqueTextures ) * 8 )
		return;

	if ( a_bOpaque )
		s_aOpaqueTextures[ a_uiTexture >> 3 ] |= 1 << ( a_uiTexture & 7 );
	else
		s_aOpaqueTextures[ a_uiTexture >> 3 ] &= ~( 1 << ( a_uiTexture & 7 ) );
}

static GLuint FindNoAlphaTestProgram( GLuint a_uiProgram )
{
	auto it = s_oNoAlphaTestPrograms.Find( a_uiProgram );
	return it != s_oNoAlphaTestPrograms.End() ? it->GetSecond() : 0;
}

void TRenderInterface::BeginEndSceneHAL()
{
}

TRenderGLInterface::TRenderGLInterface()
{
	m_fPixelAspectRatio                  = 1.0f;
	m_pDevice                            = TNULL;
	m_oDisplayParams.uiWidth             = 640; // Default width
	m_oDisplayParams.uiHeight            = 480; // Default height
	m_oDisplayParams.uiColourDepth       = 32;  // Default color depth
	m_oDisplayParams.eDepthStencilFormat = DEPTHSTENCILFORMAT_D16;
	m_oDisplayParams.bWindowed           = TTRUE;
	m_fBrightness                        = 0.5f;
	m_fSaturate                          = 0.5f;
	m_bExited                            = TFALSE;
	m_bFailed                            = TFALSE;
	m_fnOnDeviceLostCallback             = TNULL;
	m_fnOnDeviceFoundCallback            = TNULL;
	m_fContrast                          = 0.583012f;
	m_fGamma                             = 0.420849f;
	m_bEnableColourCorrection            = TTRUE;

	m_pPendingCapture    = TNULL;
	m_uiBackBufferWidth  = 0;
	m_uiBackBufferHeight = 0;
	m_uiVertexArray      = 0;
	m_pVertexArrays       = TNULL;
	m_uiNumVertexArrays   = 0;
	m_pAppliedVertexArray = TNULL;
	m_bImmediateDraw      = TFALSE;
	TUtil::MemClear( m_aViewport, sizeof( m_aViewport ) );

	m_uiCurrentProgram     = 0;
	m_uiCurrentNoAlphaTestProgram = 0;
	m_pCurrentLayout       = TNULL;
	m_uiCurrentIndexBuffer = 0;
	m_eCurrentIndexFormat  = GL_UNSIGNED_SHORT;
	m_uiCurrentIndexOffset = 0;
	TUtil::MemClear( m_auiCurrentVertexBuffers, sizeof( m_auiCurrentVertexBuffers ) );
	TUtil::MemClear( m_auiCurrentStrides, sizeof( m_auiCurrentStrides ) );
	TUtil::MemClear( m_auiCurrentOffsets, sizeof( m_auiCurrentOffsets ) );
	TUtil::MemClear( m_auiCurrentTextures, sizeof( m_auiCurrentTextures ) );
	TUtil::MemClear( m_auiCurrentSamplers, sizeof( m_auiCurrentSamplers ) );

	m_oCurrentBlend.BlendOp           = GL_FUNC_ADD;
	m_oCurrentBlend.BlendOpAlpha      = GL_FUNC_ADD;
	m_oCurrentBlend.SrcBlend          = GL_ONE;
	m_oCurrentBlend.DestBlend         = GL_ZERO;
	m_oCurrentBlend.SrcBlendAlpha     = GL_ONE;
	m_oCurrentBlend.DestBlendAlpha    = GL_ZERO;
	m_oCurrentBlend.bBlendEnable      = TFALSE;
	m_oCurrentBlend.bColorWrite       = TTRUE;
	m_oCurrentBlend.bAlphaWrite       = TTRUE;
	m_oCurrentBlend.fBlendFactorAlpha = 1.0f;

	m_oCurrentDepth.bDepthEnable       = TTRUE;
	m_oCurrentDepth.bDepthWrite        = TTRUE;
	m_oCurrentDepth.DepthFunc          = GL_LEQUAL;
	m_oCurrentDepth.bStencilEnable     = TFALSE;
	m_oCurrentDepth.StencilMask        = 0xFF;
	m_oCurrentDepth.StencilFunc        = GL_ALWAYS;
	m_oCurrentDepth.StencilFailOp      = GL_KEEP;
	m_oCurrentDepth.StencilDepthFailOp = GL_KEEP;
	m_oCurrentDepth.StencilPassOp      = GL_KEEP;
	m_oCurrentDepth.uiStencilRef       = 0;

	m_oCurrentRasterizer.eCullMode             = CULL_NONE;
	m_oCurrentRasterizer.bScissorEnable        = TFALSE;
	m_oCurrentRasterizer.iDepthBias            = 0;
	m_oCurrentRasterizer.fSlopeScaledDepthBias = 0.0f;

	m_bStateCacheValid = TFALSE;

	m_pConstantBufferData       = TNULL;
	m_bConstantBufferChanged    = TFALSE;
	m_uiConstantUploadLimit     = 0;
	m_uiNextConstantBuffer      = 0;
	m_uiViewportBuffer          = 0;
	m_uiUploadedConstantSize    = 0;
	m_uiUploadedConstantProgram = 0;

	m_uiNextStripBatchBuffer         = 0;
	m_pStripBatchIndices             = TNULL;
	m_uiStripBatchCount              = 0;
	m_uiStripBatchVertexBuffer       = 0;
	m_uiStripBatchStride             = 0;
	m_uiNextSmallDrawBuffer          = 0;
	m_uiImmediateVertexBuffer        = 0;
	m_uiImmediateVertexCurrentOffset = 0;
	m_uiImmediateIndexBuffer         = 0;
	m_uiImmediateIndexCurrentOffset  = 0;

	TUtil::MemClear( m_auiSamplerStates, sizeof( m_auiSamplerStates ) );

	for ( TINT i = 0; i < NUM_TEXTURE_STAGES; i++ )
	{
		m_aAddressU[ i ]   = ADDRESSINGMODE_WRAP;
		m_aAddressV[ i ]   = ADDRESSINGMODE_WRAP;
		m_aMinFilters[ i ] = TEXTUREFILTER_POINT;
		m_aMagFilters[ i ] = TEXTUREFILTER_POINT;
		m_aMipFilters[ i ] = TEXTUREFILTER_NONE;
	}

	m_uiDirtySamplers        = ( 1u << NUM_TEXTURE_STAGES ) - 1;
	m_uiFixedFunctionProgram = 0;
	m_bFixedFunction         = TFALSE;
	m_eTextureOp             = TEXTUREOP_MODULATE;
	m_bAlphaTestEnable       = TFALSE;
	m_eAlphaFunc             = GL_ALWAYS;
	m_uiAlphaRef             = 0;
	m_bFogEnable             = TFALSE;
	m_uiFogColour            = 0;
	m_fFogStart              = 0.0f;
	m_fFogEnd                = 1.0f;
	m_bS3TCSupported         = TFALSE;

	for ( TINT i = 0; i < TRANSFORM_NUMOF; i++ )
		m_aTransforms[ i ] = TMatrix44::IDENTITY;
}

TRenderGLInterface::~TRenderGLInterface()
{
	Destroy();
}

TBOOL TRenderGLInterface::CreateDisplay( const DISPLAYPARAMS& a_rParams )
{
	if ( !TRenderInterface::CreateDisplay() )
	{
		OnInitializationFailureDisplay();
		return TFALSE;
	}

	m_pDevice        = TSTATICCAST( TGLAdapter::Mode::Device, FindDevice( a_rParams ) );
	m_oDisplayParams = a_rParams;

	if ( m_pDevice )
	{
		m_uiBackBufferWidth  = m_oDisplayParams.uiWidth;
		m_uiBackBufferHeight = m_oDisplayParams.uiHeight;

		m_Window.SetDisplayMode( m_uiBackBufferWidth, m_uiBackBufferHeight, m_oDisplayParams.bWindowed, m_oDisplayParams.bBorderless );

		if ( !ResizeBackBuffer() )
		{
			OnInitializationFailureDevice();
			return TFALSE;
		}

		SetDeviceDefaultStates();
		m_Window.CenterCursor( m_uiBackBufferWidth, m_uiBackBufferHeight );

		// Create invalid texture pattern
		TUINT invalidTextureData[ 32 ];
		for ( TINT i = 0; i < 32; i++ )
		{
			invalidTextureData[ i ] = 0xff0fff0f;
		}

		auto pTextureFactory = GetSystemResource<TTextureFactoryHAL>( SYSRESOURCE_TEXTUREFACTORY );
		m_pInvalidTexture    = pTextureFactory->CreateTextureFromMemory( invalidTextureData, sizeof( invalidTextureData ), TTexture::TEXTUREFLAGS_UNK0 | TTexture::TEXTUREFLAGS_LAYOUT_4444, 8, 8 );

		EnableColourCorrection( TTRUE );
		m_bDisplayCreated = TTRUE;

		return TTRUE;
	}

	OnInitializationFailureDisplay();
	return TFALSE;
}

TBOOL TRenderGLInterface::DestroyDisplay()
{
	TASSERT( IsDisplayCreated() );
	FlushDyingResources();

	if ( IsDisplayCreated() )
	{
		m_Window.RestoreDisplayMode();
		return TRenderInterface::DestroyDisplay();
	}

	return TTRUE;
}

TBOOL TRenderGLInterface::Update( TFLOAT a_fDeltaTime )
{
	TRenderInterface::Update( a_fDeltaTime );
	m_Window.Update( a_fDeltaTime );

	return !m_bExited;
}

TBOOL TRenderGLInterface::BeginScene()
{
	TASSERT( !IsInScene() );

	if ( TRenderInterface::BeginScene() )
	{
		ResetStateCache();
		SetViewport( 0, 0, m_uiBackBufferWidth, m_uiBackBufferHeight );

		UpdateColourSettings();

		m_bInScene = TTRUE;
		m_BeginSceneEmitter.Throw( 0 );
	}

	return TTRUE;
}

TBOOL TRenderGLInterface::EndScene()
{
	TASSERT( IsInScene() );

	if ( IsInScene() )
	{
		m_EndSceneEmitter.Throw( TNULL );

		if ( m_pPendingCapture )
		{
			m_pPendingCapture->CaptureBackBuffer( m_uiBackBufferWidth, m_uiBackBufferHeight );
			m_pPendingCapture = TNULL;
		}

		m_Window.SwapBuffers();

		m_bInScene = TFALSE;
	}

	return TTRUE;
}

TBOOL TRenderGLInterface::ResizeBackBuffer()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glViewport( 0, 0, m_uiBackBufferWidth, m_uiBackBufferHeight );

	glDepthMask( GL_TRUE );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glStencilMask( 0xFF );
	glDisable( GL_SCISSOR_TEST );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClearDepthf( 1.0f );
	glClearStencil( 0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	InvalidateStateCache();

	return glGetError() == GL_NO_ERROR;
}

TBOOL TRenderGLInterface::ChangeDisplay( TUINT32 a_uiWidth, TUINT32 a_uiHeight, TBOOL a_bWindowed, TBOOL a_bBorderless )
{
	if ( !IsDisplayCreated() )
		return TFALSE;

	m_oDisplayParams.uiWidth     = a_uiWidth;
	m_oDisplayParams.uiHeight    = a_uiHeight;
	m_oDisplayParams.bWindowed   = a_bWindowed;
	m_oDisplayParams.bBorderless = a_bBorderless;
	m_uiBackBufferWidth          = a_uiWidth;
	m_uiBackBufferHeight         = a_uiHeight;

	m_Window.SetDisplayMode( a_uiWidth, a_uiHeight, a_bWindowed, a_bBorderless );

	return ResizeBackBuffer();
}

TRenderAdapter::Mode::Device* TRenderGLInterface::GetCurrentDevice()
{
	return m_pDevice;
}

TRenderInterface::DISPLAYPARAMS* TRenderGLInterface::GetCurrentDisplayParams()
{
	return &m_oDisplayParams;
}

TBOOL TRenderGLInterface::Create()
{
	return Create( TClassObjectName.GetName() );
}

void TRenderGLInterface::FlushShaders()
{
	Interface()->FlushOrderTables();

	for ( auto it = TShader::sm_oShaderList.GetRootShader(); it != TNULL; it = it->GetNextShader() )
	{
		it->Flush();
	}
}

static GLuint CompileShaderStage( GLenum a_eStage, const TCHAR* a_szStageDefine, const TCHAR* a_szDefines, const TCHAR* a_szSource )
{
	const TCHAR* aSources[] = {
		s_szShaderVersion,
		a_szStageDefine,
		a_szDefines ? a_szDefines : "",
		s_szShaderCommon,
		a_szSource
	};

	GLuint uiShader = glCreateShader( a_eStage );
	glShaderSource( uiShader, TARRAYSIZE( aSources ), aSources, TNULL );
	glCompileShader( uiShader );

	GLint iCompiled = GL_FALSE;
	glGetShaderiv( uiShader, GL_COMPILE_STATUS, &iCompiled );

	if ( iCompiled != GL_TRUE )
	{
		TCHAR szLog[ 2048 ];
		glGetShaderInfoLog( uiShader, sizeof( szLog ), TNULL, szLog );
		TERROR( "Shader compilation failed: %s\n", szLog );
		TASSERT( TFALSE );

		glDeleteShader( uiShader );
		return 0;
	}

	return uiShader;
}

static GLuint LinkProgram( const TCHAR* a_szSource, const TCHAR* a_szDefines )
{
	GLuint uiVertexShader   = CompileShaderStage( GL_VERTEX_SHADER, "#define VERTEX_SHADER\n", a_szDefines, a_szSource );
	GLuint uiFragmentShader = CompileShaderStage( GL_FRAGMENT_SHADER, "#define FRAGMENT_SHADER\n", a_szDefines, a_szSource );

	if ( !uiVertexShader || !uiFragmentShader )
		return 0;

	GLuint uiProgram = glCreateProgram();
	glAttachShader( uiProgram, uiVertexShader );
	glAttachShader( uiProgram, uiFragmentShader );
	glLinkProgram( uiProgram );

	glDeleteShader( uiVertexShader );
	glDeleteShader( uiFragmentShader );

	GLint iLinked = GL_FALSE;
	glGetProgramiv( uiProgram, GL_LINK_STATUS, &iLinked );

	if ( iLinked != GL_TRUE )
	{
		TCHAR szLog[ 2048 ];
		glGetProgramInfoLog( uiProgram, sizeof( szLog ), TNULL, szLog );
		TERROR( "Shader link failed: %s\n", szLog );
		TASSERT( TFALSE );

		glDeleteProgram( uiProgram );
		return 0;
	}

	GLint iBlockSize = 0;
	GLuint uiBlock   = glGetUniformBlockIndex( uiProgram, "Constants" );

	if ( uiBlock != GL_INVALID_INDEX )
	{
		glUniformBlockBinding( uiProgram, uiBlock, 0 );
		glGetActiveUniformBlockiv( uiProgram, uiBlock, GL_UNIFORM_BLOCK_DATA_SIZE, &iBlockSize );
	}

	s_oProgramBlockSizes.Insert( uiProgram, iBlockSize );

	GLuint uiViewportBlock = glGetUniformBlockIndex( uiProgram, "ViewportConstants" );

	if ( uiViewportBlock != GL_INVALID_INDEX )
		glUniformBlockBinding( uiProgram, uiViewportBlock, 1 );

	// ES 3.0 has no layout( binding ) qualifier
	glUseProgram( uiProgram );

	for ( TINT i = 0; i < TRenderGLInterface::NUM_TEXTURE_STAGES; i++ )
	{
		TCHAR szName[ 16 ];
		TStringManager::String8Format( szName, sizeof( szName ), "g_Texture%d", i );

		GLint iLocation = glGetUniformLocation( uiProgram, szName );

		if ( iLocation >= 0 )
			glUniform1i( iLocation, i );
	}

	TRenderGLInterface::Interface()->InvalidateStateCache();

	return uiProgram;
}

GLuint TRenderGLInterface::CompileProgram( const TCHAR* a_szSource, const TCHAR* a_szDefines )
{
	const GLuint uiProgram = LinkProgram( a_szSource, a_szDefines );

	// Only without S3TC, that's where the textures get decoded and checked
	if ( uiProgram && !Interface()->m_bS3TCSupported )
	{
		TCHAR szDefines[ 128 ];
		TStringManager::String8Format( szDefines, sizeof( szDefines ), "%s#define NO_ALPHA_TEST\n", a_szDefines ? a_szDefines : "" );

		const GLuint uiNoAlphaTest = LinkProgram( a_szSource, szDefines );

		if ( uiNoAlphaTest )
			s_oNoAlphaTestPrograms.Insert( uiProgram, uiNoAlphaTest );
	}

	return uiProgram;
}

void TRenderGLInterface::DestroyProgram( GLuint a_uiProgram )
{
	if ( a_uiProgram )
	{
		const GLuint uiNoAlphaTest = FindNoAlphaTestProgram( a_uiProgram );

		if ( uiNoAlphaTest )
		{
			auto itProgram = s_oNoAlphaTestPrograms.Find( a_uiProgram );
			s_oNoAlphaTestPrograms.Remove( itProgram );

			auto itSize = s_oProgramBlockSizes.Find( uiNoAlphaTest );

			if ( itSize != s_oProgramBlockSizes.End() )
				s_oProgramBlockSizes.Remove( itSize );

			glDeleteProgram( uiNoAlphaTest );
		}

		auto it = s_oProgramBlockSizes.Find( a_uiProgram );

		if ( it != s_oProgramBlockSizes.End() )
			s_oProgramBlockSizes.Remove( it );

		glDeleteProgram( a_uiProgram );
		Interface()->InvalidateStateCache();
	}
}

TBOOL TRenderGLInterface::Destroy()
{
	// NOTE: no IsCreated() assert, the destructor calls this again after ARenderer destroyed the renderer
	if ( IsCreated() )
	{
		FlushDyingResources();
		DestroyAllShaderResources();
		FlushDyingResources();

		DestroySystemResources();
		DestroyDisplay();
		DestroyDeviceObjects();

		m_Window.DestroyContext();
		m_Window.UnregisterWindowClass();

		return TRenderInterface::Destroy();
	}

	return TTRUE;
}

void TRenderGLInterface::RenderIndexPrimitive( TINT a_ePrimitiveType, TINT a_iNumVertices, TINT a_iNumPrimitives, TINT a_pIndices, TINT a_pVertices, TINT a_iVertexStride )
{
	enum PRIMITIVE_
	{
		PRIMITIVE_POINTLIST,
		PRIMITIVE_LINELIST,
		PRIMITIVE_LINESTRIP,
		PRIMITIVE_TRIANGLELIST,
		PRIMITIVE_TRIANGLESTRIP,
		PRIMITIVE_TRIANGLEFAN
	};

	static const GLenum s_aPrimitiveTypes[] = {
		GL_POINTS,
		GL_LINES,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN
	};

	TRenderContextGL* pContext = TRenderContextGL::Upcast( GetCurrentContext() );

	SetTransform( TRANSFORM_VIEW, TMatrix44::IDENTITY );
	SetTransform( TRANSFORM_PROJECTION, pContext->GetProjectionMatrix() );
	SetTransform( TRANSFORM_WORLD, pContext->GetModelViewMatrix() );

	// Position and colour
	SetFixedFunctionShader();
	SetPositionColourLayout();

	const TUINT16* pIndices = TREINTERPRETCAST( const TUINT16*, a_pIndices );
	TUINT          uiNumIndices;

	switch ( a_ePrimitiveType )
	{
		case PRIMITIVE_POINTLIST: uiNumIndices = a_iNumPrimitives; break;
		case PRIMITIVE_LINELIST: uiNumIndices = a_iNumPrimitives * 2; break;
		case PRIMITIVE_LINESTRIP: uiNumIndices = a_iNumPrimitives + 1; break;
		case PRIMITIVE_TRIANGLELIST: uiNumIndices = a_iNumPrimitives * 3; break;
		default: uiNumIndices = a_iNumPrimitives + 2; break;
	}

	DrawImmediately( s_aPrimitiveTypes[ a_ePrimitiveType ], uiNumIndices, pIndices, GL_UNSIGNED_SHORT, TREINTERPRETCAST( const void*, a_pVertices ), a_iVertexStride, a_iNumVertices );
}

TFLOAT TRenderGLInterface::GetPixelAspectRatio()
{
	return m_fPixelAspectRatio;
}

TBOOL TRenderGLInterface::SetPixelAspectRatio( TFLOAT a_fPixelAspectRatio )
{
	m_fPixelAspectRatio = a_fPixelAspectRatio;
	return TTRUE;
}

void TRenderGLInterface::FlushOrderTables()
{
	TASSERT( TTRUE == IsInScene() );

	for ( auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++ )
	{
		it->Flush();
	}
}

TRenderContext* TRenderGLInterface::CreateRenderContext()
{
	return new TRenderContextGL( this );
}

TRenderCapture* TRenderGLInterface::CreateCapture()
{
	return new TRenderCaptureGL();
}

void TRenderGLInterface::DestroyCapture( TRenderCapture* a_pRenderCapture )
{
	if ( m_pPendingCapture == a_pRenderCapture )
		m_pPendingCapture = TNULL;

	a_pRenderCapture->Destroy();
	delete a_pRenderCapture;
}

void* TRenderGLInterface::CreateUnknown( const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 )
{
	return TNULL;
}

TModel* TRenderGLInterface::CreateModelTMD( TTMD* a_pTMD, TBOOL a_bLoad )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_pTMD, a_bLoad ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

TModel* TRenderGLInterface::CreateModelTMDFile( const TCHAR* a_szFilePath, TBOOL a_bLoad )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_szFilePath, a_bLoad ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

TModel* TRenderGLInterface::CreateModelTRB( const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	auto pModel = new TModelHAL();

	if ( pModel )
	{
		if ( !pModel->Create( a_szFilePath, a_bLoad, a_pAssetTRB, a_ui8FileNameLen ) )
		{
			pModel->Delete();
			return TNULL;
		}
	}

	return pModel;
}

TDebugText* TRenderGLInterface::CreateDebugText()
{
	m_pDebugText = InitDebugText( this, 0 );
	return m_pDebugText;
}

void TRenderGLInterface::DestroyDebugText()
{
	if ( m_pDebugText )
	{
		m_pDebugText->Destroy();
		m_pDebugText = TNULL;
	}
}

TBOOL TRenderGLInterface::RecreateDisplay( const DISPLAYPARAMS& a_rDisplayParams )
{
	if ( IsCreated() )
	{
		if ( IsDisplayCreated() )
		{
			OnDeviceLost();
			DestroyDisplay();

			if ( CreateDisplay( a_rDisplayParams ) )
			{
				OnDeviceFound();
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

void TRenderGLInterface::SetContrast( TFLOAT a_fConstrast )
{
	TMath::Clip( a_fConstrast, 0.0f, 1.0f );
	m_fContrast = a_fConstrast;
}

void TRenderGLInterface::SetBrightness( TFLOAT a_fBrightness )
{
	TMath::Clip( a_fBrightness, 0.0f, 1.0f );
	m_fBrightness = a_fBrightness;
}

void TRenderGLInterface::SetGamma( TFLOAT a_fGamma )
{
	TMath::Clip( a_fGamma, 0.0f, 1.0f );
	m_fGamma = a_fGamma;
}

void TRenderGLInterface::SetSaturate( TFLOAT a_fSaturate )
{
	TMath::Clip( a_fSaturate, 0.0f, 1.0f );
	m_fSaturate = a_fSaturate;
}

TFLOAT TRenderGLInterface::GetContrast() const
{
	return m_fContrast;
}

TFLOAT TRenderGLInterface::GetBrightness() const
{
	return m_fBrightness;
}

TFLOAT TRenderGLInterface::GetGamma() const
{
	return m_fGamma;
}

TFLOAT TRenderGLInterface::GetSaturate() const
{
	return m_fSaturate;
}

void TRenderGLInterface::UpdateColourSettings()
{
}

TBOOL TRenderGLInterface::IsCapableColourCorrection()
{
	return TTRUE;
}

void TRenderGLInterface::EnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable && IsCapableColourCorrection();
}

void TRenderGLInterface::ForceEnableColourCorrection( TBOOL a_bEnable )
{
	m_bEnableColourCorrection = a_bEnable;
}

TBOOL TRenderGLInterface::IsColourCorrection()
{
	return m_bEnableColourCorrection;
}

void TRenderGLInterface::OnDeviceLost()
{
	g_oSystemManager.Pause( TTRUE );

	if ( TInputInterface* pInputInterface = TInputInterface::GetSingleton() )
	{
		pInputInterface->UnacquireAll();
		pInputInterface->FlushAll();
		pInputInterface->StopAllRepeats();
	}

	if ( m_bFailed )
		return;

	m_bFailed = TTRUE;

	TResource::Recurse(
	    []( TResource* a_pResource, void* a_pUserData ) {
		    a_pResource->TryInvalidate();
		    return TTRUE;
	    },
	    m_Resources.GetRoot()->Child(), TTRUE, TNULL
	);

	if ( m_pDebugText )
		TSTATICCAST( TDebugGLText, m_pDebugText )->OnDeviceLost();

	for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
		pShader->TryInvalidate();

	if ( m_fnOnDeviceLostCallback )
		m_fnOnDeviceLostCallback();

	FlushDyingResources();
	g_oSystemManager.Pause( TTRUE );
}

void TRenderGLInterface::OnDeviceFound()
{
	if ( m_bFailed == TTRUE )
	{
		TResource::Recurse( []( TResource* a_pResource, void* a_pUserData ) {
			a_pResource->TryValidate();
			return TTRUE;
		},
		                    m_Resources.GetRoot()->Child(), TTRUE, TNULL );

		if ( m_pDebugText )
			TSTATICCAST( TDebugGLText, m_pDebugText )->OnDeviceFound();

		for ( TShader* pShader = TShader::sm_oShaderList.GetRootShader(); pShader != TNULL; pShader = pShader->GetNextShader() )
			pShader->TryValidate();

		if ( m_fnOnDeviceFoundCallback )
			m_fnOnDeviceFoundCallback();

		SetDeviceDefaultStates();
		g_oSystemManager.Pause( TFALSE );
		m_bFailed = TFALSE;
	}

	if ( TInputInterface* pInputInterface = TInputInterface::GetSingleton() )
	{
		pInputInterface->UnacquireAll();
		pInputInterface->FlushAll();
		pInputInterface->StopAllRepeats();
	}

	g_oSystemManager.Pause( TFALSE );
}

TDebugGLText* TRenderGLInterface::InitDebugText( TRenderGLInterface* a_pRenderer, TINT a_iBufferSize )
{
	if ( a_iBufferSize == 0 )
		a_iBufferSize = 5120;

	// The text buffer follows the object
	void* pMem = TMalloc( a_iBufferSize + 28 );

	if ( !pMem )
		return TNULL;

	TDebugGLText* pDebugText = new ( pMem ) TDebugGLText();
	pDebugText->m_pRenderer   = a_pRenderer;
	pDebugText->m_iBufferSize = a_iBufferSize;
	pDebugText->Reset();

	pDebugText->m_pFont = new TGLFont( "Arial", 10, TGLFont::GLFONT_BOLD );
	pDebugText->m_pFont->InitDeviceObjects( a_pRenderer );
	pDebugText->m_pFont->RestoreDeviceObjects();

	return pDebugText;
}

void TRenderGLInterface::SetDeviceDefaultStates()
{
	SetZEnable( TTRUE );

	// D3DCULL_CW
	SetCullMode( CULL_FRONT );

	SetTextureMinFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureMagFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureMipFilter( 0, TEXTUREFILTER_LINEAR );
	SetTextureAddress( 0, ADDRESSINGMODE_WRAP );
}

void TRenderGLInterface::SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate )
{
	TASSERT( a_iStage >= 0 && a_iStage < NUM_TEXTURE_STAGES );

	if ( a_eAddressing != ADDRESSINGMODE_WRAP && a_eAddressing != ADDRESSINGMODE_MIRROR &&
	     a_eAddressing != ADDRESSINGMODE_CLAMP && a_eAddressing != ADDRESSINGMODE_BORDER )
	{
		TERROR( "TRenderGLInterface::SetTextureAddress: Invalid addressing mode specified!\n" );
		return;
	}

	if ( a_eTextureCoordinate == TEXCOORD_U )
	{
		m_aAddressU[ a_iStage ] = a_eAddressing;
	}
	else if ( a_eTextureCoordinate == TEXCOORD_V )
	{
		m_aAddressV[ a_iStage ] = a_eAddressing;
	}
	else
	{
		TASSERT( a_eTextureCoordinate == TEXCOORD_UV );
		m_aAddressU[ a_iStage ] = a_eAddressing;
		m_aAddressV[ a_iStage ] = a_eAddressing;
	}

	m_uiDirtySamplers |= 1 << a_iStage;
}

void TRenderGLInterface::ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, CLEARFLAGS a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil )
{
	GLbitfield uiMask = 0;

	if ( a_eClearFlags & CLEARFLAGS_TARGET )
	{
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		glClearColor( a_uiColorR * ( 1.0f / 255.0f ), a_uiColorG * ( 1.0f / 255.0f ), a_uiColorB * ( 1.0f / 255.0f ), 1.0f );
		uiMask |= GL_COLOR_BUFFER_BIT;
	}

	if ( a_eClearFlags & CLEARFLAGS_DEPTH )
	{
		TMath::Clip( a_fZ, 0.0f, 1.0f );
		glDepthMask( GL_TRUE );
		glClearDepthf( a_fZ );
		uiMask |= GL_DEPTH_BUFFER_BIT;
	}

	if ( a_eClearFlags & CLEARFLAGS_STENCIL )
	{
		glStencilMask( 0xFF );
		glClearStencil( a_uiStencil );
		uiMask |= GL_STENCIL_BUFFER_BIT;
	}

	if ( uiMask != 0 )
	{
		glEnable( GL_SCISSOR_TEST );
		glScissor( a_iX, TINT( m_uiBackBufferHeight ) - a_iY - a_iHeight, a_iWidth, a_iHeight );
		glClear( uiMask );
		glDisable( GL_SCISSOR_TEST );

		// The masks and the scissor test were changed behind the state cache
		m_oAppliedBlend.bColorWrite = !m_oCurrentBlend.bColorWrite;
		m_oAppliedDepth.bDepthWrite = !m_oCurrentDepth.bDepthWrite;
		m_oAppliedDepth.StencilMask = ~m_oCurrentDepth.StencilMask;
		m_oAppliedRasterizer.bScissorEnable = TFALSE;
	}
}

TBOOL TRenderGLInterface::IsTextureFormatSupported( TTEXTURERESOURCEFORMAT a_eTextureFormat )
{
	switch ( a_eTextureFormat )
	{
		case TTEXTURERESOURCEFORMAT::R8G8B8A8:
		case TTEXTURERESOURCEFORMAT::R8G8B8:
		case TTEXTURERESOURCEFORMAT::R5G5B5A1:
		case TTEXTURERESOURCEFORMAT::DDS:
			return TTRUE;
		default:
			return TFALSE;
	}
}

TBOOL TRenderGLInterface::Supports32BitTextures()
{
	return TTRUE;
}

void TRenderGLInterface::OnInitializationFailureDevice()
{
	TERROR( "Failed to initialize OpenGL. Please ensure your video card supports OpenGL 3.3\n" );
#ifdef TOSHI_SKU_WINDOWS
	MessageBoxA( NULL, "Failed to initialize OpenGL. Please ensure your video card supports OpenGL 3.3", "Initialization failure", 0 );
#endif
}

void TRenderGLInterface::OnInitializationFailureDisplay()
{
	TERROR( "Failed to create the display\n" );
#ifdef TOSHI_SKU_WINDOWS
	MessageBoxA( NULL, "Failed to create the display", "Initialization failure", 0 );
#endif
}

TBOOL TRenderGLInterface::Create( const TCHAR* a_szWindowName )
{
	TASSERT( TFALSE == IsCreated() );

	if ( TRenderInterface::Create() )
	{
		// The GL context lives on the window
		if ( !m_Window.Create( this, a_szWindowName ) || !m_Window.CreateContext() )
		{
			OnInitializationFailureDevice();
			return TFALSE;
		}

		BuildAdapterDatabase();
		CreateDeviceObjects();

		if ( m_bCreateSystemResources )
			CreateSystemResources();

		return TTRUE;
	}

	return TFALSE;
}

void TRenderGLInterface::BuildAdapterDatabase()
{
	auto pAdapter = new TGLAdapter();
	pAdapter->SetAdapterIndex( 0 );

	const TCHAR* szRenderer = TREINTERPRETCAST( const TCHAR*, glGetString( GL_RENDERER ) );
	const TCHAR* szVersion  = TREINTERPRETCAST( const TCHAR*, glGetString( GL_VERSION ) );

	pAdapter->SetDriver( szVersion ? szVersion : "OpenGL" );
	pAdapter->SetDescription( szRenderer ? szRenderer : "" );
	pAdapter->EnumerateOutputs( this );

	TINFO( "OpenGL: %s, %s\n", szRenderer ? szRenderer : "?", szVersion ? szVersion : "?" );

	GetAdapterList()->InsertTail( pAdapter );
}

void TRenderGLInterface::RegisterOrderTable( TOrderTable* a_pOrderTable )
{
	m_OrderTables.Insert( a_pOrderTable );
}

GLuint TRenderGLInterface::GetSamplerState( TEXTUREFILTER a_eMinFilter, TEXTUREFILTER a_eMagFilter, TEXTUREFILTER a_eMipFilter, ADDRESSINGMODE a_eAddressU, ADDRESSINGMODE a_eAddressV )
{
	GLuint& rSampler = m_auiSamplerStates[ a_eMinFilter ][ a_eMagFilter ][ a_eMipFilter ][ a_eAddressU ][ a_eAddressV ];

	if ( rSampler == 0 )
	{
		static const GLenum s_aMinFilters[ TEXTUREFILTER_NUMOF ][ TEXTUREFILTER_NUMOF ] = {
			// Mip filter: none, point, linear
			{ GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR }, // no min filter takes the point one
			{ GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR },
			{ GL_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR },
		};

		static const GLenum s_aAddressing[ ADDRESSINGMODE_NUMOF ] = {
			GL_REPEAT,
			GL_MIRRORED_REPEAT,
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_EDGE,
		};

		glGenSamplers( 1, &rSampler );
		glSamplerParameteri( rSampler, GL_TEXTURE_MIN_FILTER, s_aMinFilters[ a_eMinFilter ][ a_eMipFilter ] );
		glSamplerParameteri( rSampler, GL_TEXTURE_MAG_FILTER, a_eMagFilter == TEXTUREFILTER_LINEAR ? GL_LINEAR : GL_NEAREST );
		glSamplerParameteri( rSampler, GL_TEXTURE_WRAP_S, s_aAddressing[ a_eAddressU ] );
		glSamplerParameteri( rSampler, GL_TEXTURE_WRAP_T, s_aAddressing[ a_eAddressV ] );
	}

	return rSampler;
}

void TRenderGLInterface::CreateDeviceObjects()
{
	glGenVertexArrays( 1, &m_uiVertexArray );
	glBindVertexArray( m_uiVertexArray );

	m_pVertexArrays = TSTATICCAST( VERTEXARRAY, TMalloc( sizeof( VERTEXARRAY ) * VERTEX_ARRAY_TABLE_SIZE ) );
	TUtil::MemClear( m_pVertexArrays, sizeof( VERTEXARRAY ) * VERTEX_ARRAY_TABLE_SIZE );

	// Everything is clockwise, like D3D's default
	glFrontFace( GL_CW );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );

	m_bS3TCSupported = m_Window.IsExtensionSupported( "GL_EXT_texture_compression_s3tc" ) || m_Window.IsExtensionSupported( "WEBGL_compressed_texture_s3tc" );

	m_pConstantBufferData = TSTATICCAST( TUINT8, TMalloc( CONSTANT_BUFFER_SIZE ) );
	TUtil::MemClear( m_pConstantBufferData, CONSTANT_BUFFER_SIZE );

	for ( TUINT32 i = 0; i < NUM_CONSTANT_BUFFERS; i++ )
		m_auiConstantBuffers[ i ] = CreateBuffer( GL_UNIFORM_BUFFER, CONSTANT_BUFFER_SIZE, TNULL, GL_DYNAMIC_DRAW );

	m_uiImmediateVertexBuffer = CreateBuffer( GL_ARRAY_BUFFER, IMMEDIATE_VERTEX_BUFFER_SIZE, TNULL, GL_DYNAMIC_DRAW );
	m_uiImmediateIndexBuffer  = CreateBuffer( GL_ELEMENT_ARRAY_BUFFER, IMMEDIATE_INDEX_BUFFER_SIZE, TNULL, GL_DYNAMIC_DRAW );

	for ( TUINT32 i = 0; i < NUM_SMALL_DRAW_BUFFERS; i++ )
		m_auiSmallDrawBuffers[ i ] = CreateBuffer( GL_ARRAY_BUFFER, SMALL_DRAW_BUFFER_SIZE, TNULL, GL_DYNAMIC_DRAW );

	for ( TUINT32 i = 0; i < NUM_STRIP_BATCH_BUFFERS; i++ )
		m_auiStripBatchBuffers[ i ] = CreateBuffer( GL_ELEMENT_ARRAY_BUFFER, STRIP_BATCH_MAX_INDICES * sizeof( TUINT32 ), TNULL, GL_DYNAMIC_DRAW );

	m_pStripBatchIndices = TSTATICCAST( TUINT32, TMalloc( STRIP_BATCH_MAX_INDICES * sizeof( TUINT32 ) ) );
	m_uiStripBatchCount  = 0;

	// NOTE: small buffers on purpose, ANGLE on D3D11 is very slow with ranges of a big one
	glBindBufferBase( GL_UNIFORM_BUFFER, 0, m_auiConstantBuffers[ 0 ] );

	m_uiViewportBuffer = CreateBuffer( GL_UNIFORM_BUFFER, sizeof( TVector4 ), TNULL, GL_DYNAMIC_DRAW );
	glBindBufferBase( GL_UNIFORM_BUFFER, 1, m_uiViewportBuffer );

	m_uiFixedFunctionProgram = CompileProgram( s_szFixedFunctionShader );

	InvalidateStateCache();
}

void TRenderGLInterface::DestroyDeviceObjects()
{
	DestroyProgram( m_uiFixedFunctionProgram );
	DestroyBuffer( m_uiViewportBuffer );
	DestroyBuffer( m_uiImmediateVertexBuffer );
	DestroyBuffer( m_uiImmediateIndexBuffer );

	for ( TUINT32 i = 0; i < NUM_CONSTANT_BUFFERS; i++ )
	{
		DestroyBuffer( m_auiConstantBuffers[ i ] );
		m_auiConstantBuffers[ i ] = 0;
	}

	for ( TUINT32 i = 0; i < NUM_STRIP_BATCH_BUFFERS; i++ )
	{
		DestroyBuffer( m_auiStripBatchBuffers[ i ] );
		m_auiStripBatchBuffers[ i ] = 0;
	}

	if ( m_pStripBatchIndices )
	{
		TFree( m_pStripBatchIndices );
		m_pStripBatchIndices = TNULL;
	}

	for ( TUINT32 i = 0; i < NUM_SMALL_DRAW_BUFFERS; i++ )
	{
		DestroyBuffer( m_auiSmallDrawBuffers[ i ] );
		m_auiSmallDrawBuffers[ i ] = 0;
	}

	m_uiFixedFunctionProgram  = 0;
	m_uiViewportBuffer        = 0;
	m_uiImmediateVertexBuffer = 0;
	m_uiImmediateIndexBuffer  = 0;

	if ( m_pConstantBufferData )
	{
		TFree( m_pConstantBufferData );
		m_pConstantBufferData = TNULL;
	}

	for ( TINT a = 0; a < TEXTUREFILTER_NUMOF; a++ )
		for ( TINT b = 0; b < TEXTUREFILTER_NUMOF; b++ )
			for ( TINT c = 0; c < TEXTUREFILTER_NUMOF; c++ )
				for ( TINT d = 0; d < ADDRESSINGMODE_NUMOF; d++ )
					for ( TINT e = 0; e < ADDRESSINGMODE_NUMOF; e++ )
						if ( m_auiSamplerStates[ a ][ b ][ c ][ d ][ e ] )
						{
							glDeleteSamplers( 1, &m_auiSamplerStates[ a ][ b ][ c ][ d ][ e ] );
							m_auiSamplerStates[ a ][ b ][ c ][ d ][ e ] = 0;
						}

	if ( m_pVertexArrays )
	{
		ClearVertexArrays();
		TFree( m_pVertexArrays );
		m_pVertexArrays = TNULL;
	}

	if ( m_uiVertexArray )
	{
		glDeleteVertexArrays( 1, &m_uiVertexArray );
		m_uiVertexArray = 0;
	}
}

GLuint TRenderGLInterface::CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, const void* a_pSrcData, TUINT8 a_uiFlags )
{
	GLuint uiTexture = 0;
	glGenTextures( 1, &uiTexture );

	// The stage's binding is restored by the next draw
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, uiTexture );
	m_auiAppliedTextures[ 0 ] = uiTexture;

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, a_uiWidth, a_uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_pSrcData );

	if ( HASANYFLAG( a_uiFlags, TEXTUREFLAGS_GENERATEMIPMAPS ) )
		glGenerateMipmap( GL_TEXTURE_2D );
	else
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );

	return uiTexture;
}

void TRenderGLInterface::UpdateTexture( GLuint a_uiTexture, TUINT32 a_uiWidth, TUINT32 a_uiHeight, const void* a_pSrcData )
{
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, a_uiTexture );
	m_auiAppliedTextures[ 0 ] = a_uiTexture;

	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, a_uiWidth, a_uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, a_pSrcData );
}

void TRenderGLInterface::DestroyTexture( GLuint a_uiTexture )
{
	if ( a_uiTexture )
	{
		InvalidateShaderResource( a_uiTexture );
		SetTextureOpaque( a_uiTexture, TFALSE );
		glDeleteTextures( 1, &a_uiTexture );
	}
}

#pragma pack( push, 1 )
struct DDSPIXELFORMAT
{
	TUINT32 uiSize;
	TUINT32 uiFlags;
	TUINT32 uiFourCC;
	TUINT32 uiBitCount;
	TUINT32 uiRMask;
	TUINT32 uiGMask;
	TUINT32 uiBMask;
	TUINT32 uiAMask;
};

struct DDSHEADER
{
	TUINT32        uiMagic;
	TUINT32        uiSize;
	TUINT32        uiFlags;
	TUINT32        uiHeight;
	TUINT32        uiWidth;
	TUINT32        uiPitchOrLinearSize;
	TUINT32        uiDepth;
	TUINT32        uiMipMapCount;
	TUINT32        auiReserved1[ 11 ];
	DDSPIXELFORMAT oPixelFormat;
	TUINT32        auiCaps[ 4 ];
	TUINT32        uiReserved2;
};

struct TGAHEADER
{
	TUINT8  uiIDLength;
	TUINT8  uiColourMapType;
	TUINT8  uiImageType;
	TUINT8  auiColourMap[ 5 ];
	TUINT16 uiOriginX;
	TUINT16 uiOriginY;
	TUINT16 uiWidth;
	TUINT16 uiHeight;
	TUINT8  uiBitsPerPixel;
	TUINT8  uiDescriptor;
};
#pragma pack( pop )

static constexpr TUINT32 MakeFourCC( TCHAR a, TCHAR b, TCHAR c, TCHAR d )
{
	return TUINT32( TUINT8( a ) ) | ( TUINT32( TUINT8( b ) ) << 8 ) | ( TUINT32( TUINT8( c ) ) << 16 ) | ( TUINT32( TUINT8( d ) ) << 24 );
}

static TUINT8 ExpandMaskedChannel( TUINT32 a_uiPixel, TUINT32 a_uiMask )
{
	if ( a_uiMask == 0 )
		return 0xFF;

	TUINT32 uiShift = 0;
	while ( !( a_uiMask & ( 1u << uiShift ) ) ) uiShift++;

	const TUINT32 uiMax = a_uiMask >> uiShift;
	return TUINT8( ( ( ( a_uiPixel & a_uiMask ) >> uiShift ) * 255 + uiMax / 2 ) / uiMax );
}

static void DecodeColour565( TUINT16 a_uiColour, TUINT8* a_pOut )
{
	a_pOut[ 0 ] = TUINT8( ( ( a_uiColour >> 11 ) & 0x1F ) * 255 / 31 );
	a_pOut[ 1 ] = TUINT8( ( ( a_uiColour >> 5 ) & 0x3F ) * 255 / 63 );
	a_pOut[ 2 ] = TUINT8( ( a_uiColour & 0x1F ) * 255 / 31 );
	a_pOut[ 3 ] = 255;
}

// Decodes one DXT colour block to 16 RGBA pixels. Only DXT1 has the three colour mode
static void DecodeDXTColourBlock( const TUINT8* a_pBlock, TUINT8* a_pPixels, TBOOL a_bDXT1 )
{
	const TUINT16 uiColour0 = TUINT16( a_pBlock[ 0 ] | ( a_pBlock[ 1 ] << 8 ) );
	const TUINT16 uiColour1 = TUINT16( a_pBlock[ 2 ] | ( a_pBlock[ 3 ] << 8 ) );

	TUINT8 aColours[ 4 ][ 4 ];
	DecodeColour565( uiColour0, aColours[ 0 ] );
	DecodeColour565( uiColour1, aColours[ 1 ] );

	if ( !a_bDXT1 || uiColour0 > uiColour1 )
	{
		for ( TINT i = 0; i < 3; i++ )
		{
			aColours[ 2 ][ i ] = TUINT8( ( 2 * aColours[ 0 ][ i ] + aColours[ 1 ][ i ] ) / 3 );
			aColours[ 3 ][ i ] = TUINT8( ( aColours[ 0 ][ i ] + 2 * aColours[ 1 ][ i ] ) / 3 );
		}

		aColours[ 2 ][ 3 ] = 255;
		aColours[ 3 ][ 3 ] = 255;
	}
	else
	{
		for ( TINT i = 0; i < 3; i++ )
		{
			aColours[ 2 ][ i ] = TUINT8( ( aColours[ 0 ][ i ] + aColours[ 1 ][ i ] ) / 2 );
			aColours[ 3 ][ i ] = 0;
		}

		aColours[ 2 ][ 3 ] = 255;
		aColours[ 3 ][ 3 ] = 0;
	}

	for ( TINT i = 0; i < 16; i++ )
	{
		const TINT iIndex = ( a_pBlock[ 4 + i / 4 ] >> ( ( i % 4 ) * 2 ) ) & 3;
		TUtil::MemCopy( a_pPixels + i * 4, aColours[ iIndex ], 4 );
	}
}

// One mip level of DXT data to RGBA bytes
static void DecodeDXT( GLenum a_eFormat, const TUINT8* a_pData, TUINT32 a_uiWidth, TUINT32 a_uiHeight, TUINT8* a_pOut )
{
	const TBOOL bDXT1 = a_eFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

	for ( TUINT32 uiY = 0; uiY < a_uiHeight; uiY += 4 )
	{
		for ( TUINT32 uiX = 0; uiX < a_uiWidth; uiX += 4 )
		{
			TUINT8 aPixels[ 16 * 4 ];
			DecodeDXTColourBlock( bDXT1 ? a_pData : a_pData + 8, aPixels, bDXT1 );

			if ( a_eFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT )
			{
				for ( TINT i = 0; i < 16; i++ )
				{
					const TUINT8 uiAlpha = ( a_pData[ i / 2 ] >> ( ( i & 1 ) * 4 ) ) & 0xF;
					aPixels[ i * 4 + 3 ] = TUINT8( uiAlpha * 17 );
				}
			}
			else if ( a_eFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT )
			{
				TUINT8 aAlphas[ 8 ];
				aAlphas[ 0 ] = a_pData[ 0 ];
				aAlphas[ 1 ] = a_pData[ 1 ];

				if ( aAlphas[ 0 ] > aAlphas[ 1 ] )
				{
					for ( TINT i = 1; i < 7; i++ )
						aAlphas[ i + 1 ] = TUINT8( ( ( 7 - i ) * aAlphas[ 0 ] + i * aAlphas[ 1 ] ) / 7 );
				}
				else
				{
					for ( TINT i = 1; i < 5; i++ )
						aAlphas[ i + 1 ] = TUINT8( ( ( 5 - i ) * aAlphas[ 0 ] + i * aAlphas[ 1 ] ) / 5 );

					aAlphas[ 6 ] = 0;
					aAlphas[ 7 ] = 255;
				}

				for ( TINT i = 0; i < 16; i++ )
				{
					const TINT    iBit   = i * 3;
					const TUINT32 uiBits = a_pData[ 2 + iBit / 8 ] | ( TUINT32( iBit / 8 < 5 ? a_pData[ 3 + iBit / 8 ] : 0 ) << 8 );
					aPixels[ i * 4 + 3 ] = aAlphas[ ( uiBits >> ( iBit % 8 ) ) & 7 ];
				}
			}

			const TUINT32 uiBlockWidth  = TMath::Min( a_uiWidth - uiX, 4u );
			const TUINT32 uiBlockHeight = TMath::Min( a_uiHeight - uiY, 4u );

			for ( TUINT32 uiRow = 0; uiRow < uiBlockHeight; uiRow++ )
				TUtil::MemCopy( a_pOut + ( ( uiY + uiRow ) * a_uiWidth + uiX ) * 4, aPixels + uiRow * 4 * 4, uiBlockWidth * 4 );

			a_pData += bDXT1 ? 8 : 16;
		}
	}
}

// The whole mip chain, like D3DX, unless it's compressed
static GLuint CreateDDSTexture( TBOOL a_bS3TCSupported, const void* a_pData, TUINT32 a_uiDataSize, TGLTextureInfo* a_pInfo )
{
	if ( a_uiDataSize < sizeof( DDSHEADER ) )
		return 0;

	const DDSHEADER& rHeader = *TSTATICCAST( const DDSHEADER, a_pData );
	const TUINT8*    pData   = TSTATICCAST( const TUINT8, a_pData ) + sizeof( DDSHEADER );
	const TUINT8*    pEnd    = TSTATICCAST( const TUINT8, a_pData ) + a_uiDataSize;

	const DDSPIXELFORMAT& rFormat   = rHeader.oPixelFormat;
	const TUINT32         uiMipMaps = TMath::Max( rHeader.uiMipMapCount, 1u );

	static constexpr TUINT32 DDPF_ALPHAPIXELS = 0x1;
	static constexpr TUINT32 DDPF_ALPHA       = 0x2;
	static constexpr TUINT32 DDPF_FOURCC      = 0x4;
	static constexpr TUINT32 DDPF_LUMINANCE   = 0x20000;

	GLenum eCompressedFormat = 0;

	if ( rFormat.uiFlags & DDPF_FOURCC )
	{
		switch ( rFormat.uiFourCC )
		{
			case MakeFourCC( 'D', 'X', 'T', '1' ): eCompressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
			case MakeFourCC( 'D', 'X', 'T', '2' ):
			case MakeFourCC( 'D', 'X', 'T', '3' ): eCompressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
			case MakeFourCC( 'D', 'X', 'T', '4' ):
			case MakeFourCC( 'D', 'X', 'T', '5' ): eCompressedFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
			default:
				TERROR( "Unsupported DDS format 0x%08X\n", rFormat.uiFourCC );
				return 0;
		}
	}

	GLuint uiTexture = 0;
	glGenTextures( 1, &uiTexture );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, uiTexture );

	TUINT32 uiWidth  = rHeader.uiWidth;
	TUINT32 uiHeight = rHeader.uiHeight;
	TUINT32 uiLevels = 0;
	TUINT8* pPixels  = TNULL;
	TBOOL   bOpaque  = TFALSE;

	const auto fnCheckAlpha = [ & ]() {
		for ( TUINT32 uiPixel = 0; uiPixel < uiWidth * uiHeight && bOpaque; uiPixel++ )
			bOpaque = pPixels[ uiPixel * 4 + 3 ] > ALPHA_TEST_MAX_REF;
	};

	if ( !a_bS3TCSupported )
		bOpaque = TTRUE;

	if ( !( a_bS3TCSupported && eCompressedFormat ) )
		pPixels = new TUINT8[ uiWidth * uiHeight * 4 ];

	for ( TUINT32 i = 0; i < uiMipMaps; i++ )
	{
		if ( eCompressedFormat )
		{
			const TUINT32 uiSize = ( ( uiWidth + 3 ) / 4 ) * ( ( uiHeight + 3 ) / 4 ) * ( eCompressedFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16 );

			if ( pData + uiSize > pEnd )
				break;

			if ( a_bS3TCSupported )
			{
				glCompressedTexImage2D( GL_TEXTURE_2D, i, eCompressedFormat, uiWidth, uiHeight, 0, uiSize, pData );
			}
			else
			{
				DecodeDXT( eCompressedFormat, pData, uiWidth, uiHeight, pPixels );
				fnCheckAlpha();
				glTexImage2D( GL_TEXTURE_2D, i, GL_RGBA8, uiWidth, uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels );
			}

			pData += uiSize;
		}
		else
		{
			const TUINT32 uiPixelSize = rFormat.uiBitCount / 8;
			const TUINT32 uiSize      = uiWidth * uiHeight * uiPixelSize;

			if ( uiPixelSize == 0 || uiPixelSize > 4 || pData + uiSize > pEnd )
				break;

			const TUINT32 uiAMask = ( rFormat.uiFlags & ( DDPF_ALPHAPIXELS | DDPF_ALPHA ) ) ? rFormat.uiAMask : 0;

			for ( TUINT32 uiPixel = 0; uiPixel < uiWidth * uiHeight; uiPixel++ )
			{
				TUINT32 uiValue = 0;

				for ( TUINT32 uiByte = 0; uiByte < uiPixelSize; uiByte++ )
					uiValue |= TUINT32( pData[ uiPixel * uiPixelSize + uiByte ] ) << ( uiByte * 8 );

				TUINT8* pOut = pPixels + uiPixel * 4;

				if ( rFormat.uiFlags & DDPF_LUMINANCE )
				{
					pOut[ 0 ] = pOut[ 1 ] = pOut[ 2 ] = ExpandMaskedChannel( uiValue, rFormat.uiRMask );
				}
				else
				{
					pOut[ 0 ] = ExpandMaskedChannel( uiValue, rFormat.uiRMask );
					pOut[ 1 ] = ExpandMaskedChannel( uiValue, rFormat.uiGMask );
					pOut[ 2 ] = ExpandMaskedChannel( uiValue, rFormat.uiBMask );
				}

				pOut[ 3 ] = ExpandMaskedChannel( uiValue, uiAMask );
			}

			fnCheckAlpha();
			glTexImage2D( GL_TEXTURE_2D, i, GL_RGBA8, uiWidth, uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels );
			pData += uiSize;
		}

		uiLevels++;
		uiWidth  = TMath::Max( uiWidth >> 1, 1u );
		uiHeight = TMath::Max( uiHeight >> 1, 1u );
	}

	delete[] pPixels;

	SetTextureOpaque( uiTexture, bOpaque && uiLevels != 0 );

	if ( uiLevels == 1 && !eCompressedFormat && ( rHeader.uiWidth > 1 || rHeader.uiHeight > 1 ) )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
		uiLevels = 0;

		for ( TUINT32 uiSize = TMath::Max( rHeader.uiWidth, rHeader.uiHeight ); uiSize != 0; uiSize >>= 1 )
			uiLevels++;
	}

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, TMath::Max( uiLevels, 1u ) - 1 );

	if ( a_pInfo )
	{
		a_pInfo->Width     = rHeader.uiWidth;
		a_pInfo->Height    = rHeader.uiHeight;
		a_pInfo->MipLevels = uiLevels;
		a_pInfo->Format    = eCompressedFormat ? eCompressedFormat : ( ( rFormat.uiFlags & DDPF_ALPHAPIXELS ) ? GL_RGBA8 : GL_RGB8 );
	}

	return uiTexture;
}

static TUINT8* DecodeTGA( const void* a_pData, TUINT32 a_uiDataSize, TUINT32& a_rWidth, TUINT32& a_rHeight, TBOOL* a_pHasAlpha )
{
	if ( a_uiDataSize < sizeof( TGAHEADER ) )
		return TNULL;

	const TGAHEADER& rHeader = *TSTATICCAST( const TGAHEADER, a_pData );
	const TUINT8*    pData   = TSTATICCAST( const TUINT8, a_pData ) + sizeof( TGAHEADER ) + rHeader.uiIDLength;
	const TUINT8*    pEnd    = TSTATICCAST( const TUINT8, a_pData ) + a_uiDataSize;

	const TUINT32 uiWidth     = rHeader.uiWidth;
	const TUINT32 uiHeight    = rHeader.uiHeight;
	const TUINT32 uiPixelSize = rHeader.uiBitsPerPixel / 8;
	const TBOOL   bRLE        = rHeader.uiImageType == 10 || rHeader.uiImageType == 11;
	const TBOOL   bGrey       = rHeader.uiImageType == 3 || rHeader.uiImageType == 11;
	const TBOOL   bTrueColour = rHeader.uiImageType == 2 || rHeader.uiImageType == 10;

	if ( ( !bGrey && !bTrueColour ) || uiPixelSize == 0 || uiPixelSize > 4 || uiWidth == 0 || uiHeight == 0 )
	{
		TERROR( "Unsupported TGA image type %d (%d bpp)\n", rHeader.uiImageType, rHeader.uiBitsPerPixel );
		return TNULL;
	}

	const TBOOL bBottomUp = ( rHeader.uiDescriptor & 0x20 ) == 0;
	TUINT8*     pPixels   = TSTATICCAST( TUINT8, TMalloc( uiWidth * uiHeight * 4 ) );
	TBOOL       bHasAlpha = TFALSE;

	TUINT32 uiRunLeft = 0;
	TBOOL   bRawRun   = TFALSE;
	TUINT8  aPixel[ 4 ] = { 0, 0, 0, 255 };

	for ( TUINT32 uiIndex = 0; uiIndex < uiWidth * uiHeight; uiIndex++ )
	{
		if ( bRLE && uiRunLeft == 0 )
		{
			if ( pData >= pEnd ) break;

			const TUINT8 uiPacket = *pData++;
			uiRunLeft             = ( uiPacket & 0x7F ) + 1;
			bRawRun               = !( uiPacket & 0x80 );

			if ( !bRawRun )
			{
				if ( pData + uiPixelSize > pEnd ) break;
				TUtil::MemCopy( aPixel, pData, uiPixelSize );
				pData += uiPixelSize;
			}
		}

		if ( !bRLE || bRawRun )
		{
			if ( pData + uiPixelSize > pEnd ) break;
			TUtil::MemCopy( aPixel, pData, uiPixelSize );
			pData += uiPixelSize;
		}

		if ( bRLE ) uiRunLeft--;

		const TUINT32 uiRow = uiIndex / uiWidth;
		const TUINT32 uiCol = uiIndex % uiWidth;
		TUINT8*       pOut  = pPixels + ( ( bBottomUp ? uiHeight - 1 - uiRow : uiRow ) * uiWidth + uiCol ) * 4;

		if ( bGrey )
		{
			pOut[ 0 ] = pOut[ 1 ] = pOut[ 2 ] = aPixel[ 0 ];
			pOut[ 3 ]                         = 255;
		}
		else if ( uiPixelSize == 2 )
		{
			const TUINT16 uiValue = TUINT16( aPixel[ 0 ] | ( aPixel[ 1 ] << 8 ) );
			pOut[ 0 ]             = TUINT8( ( ( uiValue >> 10 ) & 0x1F ) * 255 / 31 );
			pOut[ 1 ]             = TUINT8( ( ( uiValue >> 5 ) & 0x1F ) * 255 / 31 );
			pOut[ 2 ]             = TUINT8( ( uiValue & 0x1F ) * 255 / 31 );
			pOut[ 3 ]             = 255;
		}
		else
		{
			pOut[ 0 ] = aPixel[ 2 ];
			pOut[ 1 ] = aPixel[ 1 ];
			pOut[ 2 ] = aPixel[ 0 ];
			pOut[ 3 ] = uiPixelSize == 4 ? aPixel[ 3 ] : 255;
			bHasAlpha |= uiPixelSize == 4;
		}
	}

	a_rWidth  = uiWidth;
	a_rHeight = uiHeight;

	if ( a_pHasAlpha )
		*a_pHasAlpha = bHasAlpha;

	return pPixels;
}

static GLuint CreateTGATexture( const void* a_pData, TUINT32 a_uiDataSize, TGLTextureInfo* a_pInfo )
{
	TUINT32 uiWidth   = 0;
	TUINT32 uiHeight  = 0;
	TBOOL   bHasAlpha = TFALSE;
	TUINT8* pPixels   = DecodeTGA( a_pData, a_uiDataSize, uiWidth, uiHeight, &bHasAlpha );

	if ( !pPixels )
		return 0;

	GLuint uiTexture = 0;
	glGenTextures( 1, &uiTexture );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, uiTexture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, uiWidth, uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels );
	TFree( pPixels );

	TUINT32 uiLevels = 0;

	for ( TUINT32 uiSize = TMath::Max( uiWidth, uiHeight ); uiSize != 0; uiSize >>= 1 )
		uiLevels++;

	glGenerateMipmap( GL_TEXTURE_2D );

	if ( a_pInfo )
	{
		a_pInfo->Width     = uiWidth;
		a_pInfo->Height    = uiHeight;
		a_pInfo->MipLevels = uiLevels;
		a_pInfo->Format    = bHasAlpha ? GL_RGBA8 : GL_RGB8;
	}

	return uiTexture;
}

TUINT8* TRenderGLInterface::DecodeImage( const void* a_pData, TUINT32 a_uiDataSize, TUINT32& a_rWidth, TUINT32& a_rHeight )
{
	if ( a_uiDataSize >= sizeof( DDSHEADER ) && TUtil::MemCompare( a_pData, "DDS ", 4 ) == 0 )
	{
		const DDSHEADER& rHeader = *TSTATICCAST( const DDSHEADER, a_pData );
		const TUINT8*    pData   = TSTATICCAST( const TUINT8, a_pData ) + sizeof( DDSHEADER );

		a_rWidth  = rHeader.uiWidth;
		a_rHeight = rHeader.uiHeight;

		TUINT8* pPixels = TSTATICCAST( TUINT8, TMalloc( a_rWidth * a_rHeight * 4 ) );

		switch ( rHeader.oPixelFormat.uiFourCC )
		{
			case MakeFourCC( 'D', 'X', 'T', '1' ): DecodeDXT( GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pData, a_rWidth, a_rHeight, pPixels ); return pPixels;
			case MakeFourCC( 'D', 'X', 'T', '3' ): DecodeDXT( GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, pData, a_rWidth, a_rHeight, pPixels ); return pPixels;
			case MakeFourCC( 'D', 'X', 'T', '5' ): DecodeDXT( GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, pData, a_rWidth, a_rHeight, pPixels ); return pPixels;
		}

		const TUINT32 uiPixelSize = rHeader.oPixelFormat.uiBitCount / 8;

		for ( TUINT32 i = 0; i < a_rWidth * a_rHeight; i++ )
		{
			TUINT32 uiValue = 0;

			for ( TUINT32 uiByte = 0; uiByte < uiPixelSize; uiByte++ )
				uiValue |= TUINT32( pData[ i * uiPixelSize + uiByte ] ) << ( uiByte * 8 );

			pPixels[ i * 4 + 0 ] = ExpandMaskedChannel( uiValue, rHeader.oPixelFormat.uiRMask );
			pPixels[ i * 4 + 1 ] = ExpandMaskedChannel( uiValue, rHeader.oPixelFormat.uiGMask );
			pPixels[ i * 4 + 2 ] = ExpandMaskedChannel( uiValue, rHeader.oPixelFormat.uiBMask );
			pPixels[ i * 4 + 3 ] = ExpandMaskedChannel( uiValue, rHeader.oPixelFormat.uiAMask );
		}

		return pPixels;
	}

	return DecodeTGA( a_pData, a_uiDataSize, a_rWidth, a_rHeight, TNULL );
}

GLuint TRenderGLInterface::CreateTextureFromFileInMemory( const void* a_pData, TUINT32 a_uiDataSize, TGLTextureInfo* a_pInfo )
{
	GLuint uiTexture;

	if ( a_uiDataSize >= 4 && TUtil::MemCompare( a_pData, "DDS ", 4 ) == 0 )
		uiTexture = CreateDDSTexture( m_bS3TCSupported, a_pData, a_uiDataSize, a_pInfo );
	else
		uiTexture = CreateTGATexture( a_pData, a_uiDataSize, a_pInfo );

	m_auiAppliedTextures[ 0 ] = uiTexture;

	if ( !uiTexture )
		TERROR( "Unable to load a texture!\n" );

	return uiTexture;
}

GLuint TRenderGLInterface::CreateBuffer( GLenum a_eTarget, TUINT32 a_uiDataSize, const void* a_pData, GLenum a_eUsage )
{
	GLuint uiBuffer = 0;
	glGenBuffers( 1, &uiBuffer );

	// The index buffer binding is part of the vertex array
	if ( a_eTarget == GL_ELEMENT_ARRAY_BUFFER )
		BindDefaultVertexArray();

	glBindBuffer( a_eTarget, uiBuffer );
	glBufferData( a_eTarget, a_uiDataSize, a_pData, a_eUsage );

	if ( a_eTarget == GL_ELEMENT_ARRAY_BUFFER )
		m_uiAppliedIndexBuffer = uiBuffer;

	return uiBuffer;
}

void TRenderGLInterface::UpdateBuffer( GLuint a_uiBuffer, GLenum a_eTarget, TUINT32 a_uiOffset, TUINT32 a_uiDataSize, const void* a_pData, TUINT32 a_uiDiscardSize )
{
	if ( a_eTarget == GL_ELEMENT_ARRAY_BUFFER )
		BindDefaultVertexArray();

	glBindBuffer( a_eTarget, a_uiBuffer );

	if ( a_uiDiscardSize != 0 )
		glBufferData( a_eTarget, a_uiDiscardSize, TNULL, GL_DYNAMIC_DRAW );

	glBufferSubData( a_eTarget, a_uiOffset, a_uiDataSize, a_pData );

	if ( a_eTarget == GL_ELEMENT_ARRAY_BUFFER )
		m_uiAppliedIndexBuffer = a_uiBuffer;
}

void TRenderGLInterface::DestroyBuffer( GLuint a_uiBuffer )
{
	if ( a_uiBuffer )
	{
		// Vertex arrays keep a deleted buffer alive and its id can come back as another one
		ClearVertexArrays();
		glDeleteBuffers( 1, &a_uiBuffer );
		InvalidateStateCache();
	}
}

void TRenderGLInterface::PrepareDraw()
{
	if ( m_bFixedFunction )
		UpdateFixedFunctionConstants();

	UpdateRenderStates();
	ApplyDesiredState();
	FlushConstantBuffers();
}

void TRenderGLInterface::DrawNonIndexed( GLenum a_eTopology, GLuint a_uiVertexBuffer, TUINT a_uiVertexCount, TUINT a_uiStride, TUINT a_uiStartVertex, TUINT a_uiOffset )
{
	SetVertexBuffer( 0, a_uiVertexBuffer, a_uiStride, a_uiOffset );
	PrepareDraw();
	glDrawArrays( a_eTopology, a_uiStartVertex, a_uiVertexCount );
}

void TRenderGLInterface::DrawIndexed( GLenum a_eTopology, TUINT a_uiIndexCount, GLuint a_uiIndexBuffer, TUINT a_uiIndexBufferOffset, GLenum a_eIndexFormat, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiOffset )
{
	SetVertexBuffer( 0, a_uiVertexBuffer, a_uiStride, a_uiOffset );
	SetIndexBuffer( a_uiIndexBuffer, a_eIndexFormat, a_uiIndexBufferOffset );
	PrepareDraw();
	glDrawElements( a_eTopology, a_uiIndexCount, a_eIndexFormat, TREINTERPRETCAST( const void*, TUINTPTR( a_uiIndexBufferOffset ) ) );
}

TBOOL TRenderGLInterface::AddStripToBatch( const TUINT16* a_pIndices, TUINT a_uiIndexCount, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiVertexOffset )
{
	// Up to 3 indices join it to the previous strip
	if ( !a_pIndices || a_uiIndexCount == 0 || a_uiIndexCount + 3 > STRIP_BATCH_MAX_INDICES )
		return TFALSE;

	if ( m_uiStripBatchCount != 0 && ( a_uiVertexBuffer != m_uiStripBatchVertexBuffer || a_uiStride != m_uiStripBatchStride || m_uiStripBatchCount + a_uiIndexCount + 3 > STRIP_BATCH_MAX_INDICES ) )
		FlushStripBatch();

	m_uiStripBatchVertexBuffer = a_uiVertexBuffer;
	m_uiStripBatchStride       = a_uiStride;

	const TUINT32 uiBaseVertex = a_uiVertexOffset / a_uiStride;
	TUINT32*      pOut         = m_pStripBatchIndices + m_uiStripBatchCount;

	if ( m_uiStripBatchCount != 0 )
	{
		// Degenerate triangles, the new strip has to start on an even index to keep its winding
		const TUINT32 uiFirst = a_pIndices[ 0 ] + uiBaseVertex;

		*pOut = pOut[ -1 ];
		pOut++;
		*pOut++ = uiFirst;

		if ( ( m_uiStripBatchCount + 2 ) & 1 )
			*pOut++ = uiFirst;
	}

	for ( TUINT i = 0; i < a_uiIndexCount; i++ )
		*pOut++ = a_pIndices[ i ] + uiBaseVertex;

	m_uiStripBatchCount = TUINT32( pOut - m_pStripBatchIndices );
	return TTRUE;
}

void TRenderGLInterface::FlushStripBatch()
{
	if ( m_uiStripBatchCount == 0 )
		return;

	const GLuint uiBuffer    = m_auiStripBatchBuffers[ m_uiNextStripBatchBuffer ];
	m_uiNextStripBatchBuffer = ( m_uiNextStripBatchBuffer + 1 ) % NUM_STRIP_BATCH_BUFFERS;

	// Not the index buffer binding, that one belongs to the vertex array
	glBindBuffer( GL_COPY_WRITE_BUFFER, uiBuffer );
	glBufferSubData( GL_COPY_WRITE_BUFFER, 0, m_uiStripBatchCount * sizeof( TUINT32 ), m_pStripBatchIndices );

	const TUINT32 uiCount = m_uiStripBatchCount;
	m_uiStripBatchCount   = 0;

	DrawIndexed( GL_TRIANGLE_STRIP, uiCount, uiBuffer, 0, GL_UNSIGNED_INT, m_uiStripBatchVertexBuffer, m_uiStripBatchStride, 0 );
}

TUINT32 TRenderGLInterface::CopyToImmediateBuffer( GLuint a_uiBuffer, GLenum a_eTarget, TUINT32 a_uiBufferSize, TUINT32& a_rCurrentOffset, const void* a_pData, TUINT32 a_uiDataSize )
{
	TASSERT( a_uiDataSize <= a_uiBufferSize );

	TUINT32 uiDiscardSize = 0;

	if ( a_rCurrentOffset + a_uiDataSize > a_uiBufferSize || a_rCurrentOffset == 0 )
	{
		a_rCurrentOffset = 0;
		uiDiscardSize    = a_uiBufferSize;
	}

	UpdateBuffer( a_uiBuffer, a_eTarget, a_rCurrentOffset, a_uiDataSize, a_pData, uiDiscardSize );

	const TUINT32 uiOffset = a_rCurrentOffset;
	a_rCurrentOffset += a_uiDataSize;

	return uiOffset;
}

void TRenderGLInterface::DrawNonIndexedImmediate( GLenum a_eTopology, const void* a_pVertices, TUINT a_uiVertexCount, TUINT a_uiStride )
{
	// Text is one small draw per character. Writing to a buffer that earlier draws of the frame still use
	// makes tiled GPUs copy all of it, so these get a buffer of their own, small enough not to matter if reused
	const TUINT32 uiSize = a_uiStride * a_uiVertexCount;

	if ( uiSize <= SMALL_DRAW_BUFFER_SIZE )
	{
		const GLuint uiBuffer = m_auiSmallDrawBuffers[ m_uiNextSmallDrawBuffer ];
		m_uiNextSmallDrawBuffer = ( m_uiNextSmallDrawBuffer + 1 ) % NUM_SMALL_DRAW_BUFFERS;

		UpdateBuffer( uiBuffer, GL_ARRAY_BUFFER, 0, uiSize, a_pVertices );

		m_bImmediateDraw = TTRUE;
		DrawNonIndexed( a_eTopology, uiBuffer, a_uiVertexCount, a_uiStride, 0, 0 );
		m_bImmediateDraw = TFALSE;
		return;
	}

	TUINT uiOffset = CopyToImmediateBuffer( m_uiImmediateVertexBuffer, GL_ARRAY_BUFFER, IMMEDIATE_VERTEX_BUFFER_SIZE, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride * a_uiVertexCount );

	m_bImmediateDraw = TTRUE;
	DrawNonIndexed( a_eTopology, m_uiImmediateVertexBuffer, a_uiVertexCount, a_uiStride, 0, uiOffset );
	m_bImmediateDraw = TFALSE;
}

void TRenderGLInterface::DrawImmediately( GLenum a_eTopology, TUINT a_uiIndexCount, const void* a_pIndices, GLenum a_eIndexFormat, const void* a_pVertices, TUINT a_uiStride, TUINT a_uiVertexCount )
{
	const TUINT32 uiIndexSize    = ( a_eIndexFormat == GL_UNSIGNED_INT ) ? 4 : 2;
	TUINT         uiIndexOffset  = CopyToImmediateBuffer( m_uiImmediateIndexBuffer, GL_ELEMENT_ARRAY_BUFFER, IMMEDIATE_INDEX_BUFFER_SIZE, m_uiImmediateIndexCurrentOffset, a_pIndices, uiIndexSize * a_uiIndexCount );
	TUINT         uiVertexOffset = CopyToImmediateBuffer( m_uiImmediateVertexBuffer, GL_ARRAY_BUFFER, IMMEDIATE_VERTEX_BUFFER_SIZE, m_uiImmediateVertexCurrentOffset, a_pVertices, a_uiStride * a_uiVertexCount );

	m_bImmediateDraw = TTRUE;
	DrawIndexed( a_eTopology, a_uiIndexCount, m_uiImmediateIndexBuffer, uiIndexOffset, a_eIndexFormat, m_uiImmediateVertexBuffer, a_uiStride, uiVertexOffset );
	m_bImmediateDraw = TFALSE;
}

void TRenderGLInterface::SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount )
{
	const TUINT32 uiOffset = a_iIndex * sizeof( TVector4 );
	const TUINT32 uiSize   = a_iCount * sizeof( TVector4 );
	TASSERT( uiOffset + uiSize <= CONSTANT_BUFFER_SIZE );

	TUINT8* pDst = m_pConstantBufferData + uiOffset;

	if ( m_bConstantBufferChanged || TUtil::MemCompare( pDst, a_pSrc, uiSize ) != 0 )
	{
		TUtil::MemCopy( pDst, a_pSrc, uiSize );
		m_bConstantBufferChanged = TTRUE;
	}
}

void TRenderGLInterface::FlushConstantBuffers()
{
	const TUINT32 uiLimit   = m_uiConstantUploadLimit;
	m_uiConstantUploadLimit = 0;

	if ( !m_bConstantBufferChanged && m_uiUploadedConstantProgram == m_uiCurrentProgram && uiLimit != 0 && uiLimit <= m_uiUploadedConstantSize )
		return;

	auto          it           = s_oProgramBlockSizes.Find( m_uiCurrentProgram );
	const TUINT32 uiSize       = it != s_oProgramBlockSizes.End() ? it->GetSecond() : 0;
	const TUINT32 uiUploadSize = ( uiLimit != 0 && uiLimit < uiSize ) ? uiLimit : uiSize;

	// A new program can reuse what's uploaded if it fits
	if ( !m_bConstantBufferChanged && uiUploadSize <= m_uiUploadedConstantSize )
	{
		m_uiUploadedConstantProgram = m_uiCurrentProgram;
		return;
	}

	if ( uiSize != 0 )
	{
		// Rewriting a buffer that earlier draws still use stalls some GPUs for hundreds of ms a frame
		m_uiNextConstantBuffer = ( m_uiNextConstantBuffer + 1 ) % NUM_CONSTANT_BUFFERS;

		const GLuint uiBuffer = m_auiConstantBuffers[ m_uiNextConstantBuffer ];

		// This also makes it the buffer glBufferSubData writes to
		glBindBufferBase( GL_UNIFORM_BUFFER, 0, uiBuffer );
		glBufferSubData( GL_UNIFORM_BUFFER, 0, uiUploadSize, m_pConstantBufferData );

		m_bConstantBufferChanged    = TFALSE;
		m_uiUploadedConstantSize    = uiUploadSize;
		m_uiUploadedConstantProgram = m_uiCurrentProgram;
	}
}

void TRenderGLInterface::GetViewport( TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight ) const
{
	a_rX      = m_aViewport[ 0 ];
	a_rY      = m_aViewport[ 1 ];
	a_rWidth  = m_aViewport[ 2 ];
	a_rHeight = m_aViewport[ 3 ];
}

void TRenderGLInterface::SetViewport( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TFLOAT a_fMinDepth, TFLOAT a_fMaxDepth )
{
	if ( a_iWidth != m_aViewport[ 2 ] || a_iHeight != m_aViewport[ 3 ] )
	{
		const TVector4 vHalfPixel( 1.0f / a_iWidth, -1.0f / a_iHeight, 0.0f, 0.0f );
		UpdateBuffer( m_uiViewportBuffer, GL_UNIFORM_BUFFER, 0, sizeof( vHalfPixel ), &vHalfPixel );
	}

	m_aViewport[ 0 ] = a_iX;
	m_aViewport[ 1 ] = a_iY;
	m_aViewport[ 2 ] = a_iWidth;
	m_aViewport[ 3 ] = a_iHeight;

	glViewport( a_iX, TINT( m_uiBackBufferHeight ) - a_iY - a_iHeight, a_iWidth, a_iHeight );
	glDepthRangef( a_fMinDepth, a_fMaxDepth );
}

void TRenderGLInterface::SetScissorRect( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	glScissor( a_iX, TINT( m_uiBackBufferHeight ) - a_iY - a_iHeight, a_iWidth, a_iHeight );
}

void TRenderGLInterface::SetVertexBuffer( TUINT a_uiStream, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiOffset )
{
	TASSERT( a_uiStream < MAX_VERTEX_STREAMS );

	m_auiCurrentVertexBuffers[ a_uiStream ] = a_uiVertexBuffer;
	m_auiCurrentStrides[ a_uiStream ]       = a_uiStride;
	m_auiCurrentOffsets[ a_uiStream ]       = a_uiOffset;
}

void TRenderGLInterface::SetIndexBuffer( GLuint a_uiIndexBuffer, GLenum a_eFormat, TUINT a_uiOffset )
{
	m_uiCurrentIndexBuffer = a_uiIndexBuffer;
	m_eCurrentIndexFormat  = a_eFormat;
	m_uiCurrentIndexOffset = a_uiOffset;
}

void TRenderGLInterface::InvalidateStateCache()
{
	m_bStateCacheValid = TFALSE;
}

void TRenderGLInterface::InvalidateShaderResource( GLuint a_uiTexture )
{
	for ( TUINT i = 0; i < NUM_TEXTURE_STAGES; i++ )
	{
		if ( m_auiCurrentTextures[ i ] == a_uiTexture )
			m_auiCurrentTextures[ i ] = 0;

		if ( m_auiAppliedTextures[ i ] == a_uiTexture )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, 0 );
			m_auiAppliedTextures[ i ] = 0;
		}
	}
}

void TRenderGLInterface::BindDefaultVertexArray()
{
	if ( m_pAppliedVertexArray )
	{
		glBindVertexArray( m_uiVertexArray );
		m_pAppliedVertexArray  = TNULL;
		m_pAppliedLayout       = TNULL;
		m_uiAppliedIndexBuffer = ~GLuint( 0 );
	}
}

void TRenderGLInterface::ClearVertexArrays()
{
	BindDefaultVertexArray();

	if ( m_uiNumVertexArrays == 0 )
		return;

	for ( TUINT32 i = 0; i < VERTEX_ARRAY_TABLE_SIZE; i++ )
	{
		if ( m_pVertexArrays[ i ].uiVertexArray )
			glDeleteVertexArrays( 1, &m_pVertexArrays[ i ].uiVertexArray );
	}

	TUtil::MemClear( m_pVertexArrays, sizeof( VERTEXARRAY ) * VERTEX_ARRAY_TABLE_SIZE );
	m_uiNumVertexArrays = 0;
}

void TRenderGLInterface::SpecifyVertexAttributes( TUINT32& a_ruiAttributeMask )
{
	GLuint uiBoundBuffer = ~GLuint( 0 );

	for ( TUINT i = 0; i < m_pCurrentLayout->uiNumAttributes; i++ )
	{
		const TGLVertexAttribute& rAttribute = m_pCurrentLayout->pAttributes[ i ];
		const GLuint              uiBuffer   = m_auiCurrentVertexBuffers[ rAttribute.uiStream ];

		if ( uiBuffer != uiBoundBuffer )
		{
			glBindBuffer( GL_ARRAY_BUFFER, uiBuffer );
			uiBoundBuffer = uiBuffer;
		}

		const TUINTPTR uiOffset = m_auiCurrentOffsets[ rAttribute.uiStream ] + rAttribute.uiOffset;
		glVertexAttribPointer( rAttribute.uiLocation, rAttribute.uiNumComponents, rAttribute.eType, rAttribute.bNormalized, m_auiCurrentStrides[ rAttribute.uiStream ], TREINTERPRETCAST( const void*, uiOffset ) );

		a_ruiAttributeMask |= 1u << rAttribute.uiLocation;
	}
}

void TRenderGLInterface::BindCurrentVertexArray()
{
	TUINTPTR uiHash = TREINTERPRETCAST( TUINTPTR, m_pCurrentLayout );

	for ( TUINT i = 0; i < MAX_VERTEX_STREAMS; i++ )
		uiHash = uiHash * 31 + m_auiCurrentVertexBuffers[ i ] * 2654435761u + m_auiCurrentOffsets[ i ] + m_auiCurrentStrides[ i ] * 17;

	TUINT32 uiIndex = TUINT32( uiHash ^ ( uiHash >> 15 ) ) & ( VERTEX_ARRAY_TABLE_SIZE - 1 );

	while ( m_pVertexArrays[ uiIndex ].uiVertexArray )
	{
		VERTEXARRAY& rEntry = m_pVertexArrays[ uiIndex ];

		if ( rEntry.pLayout == m_pCurrentLayout &&
		     TUtil::MemCompare( rEntry.auiBuffers, m_auiCurrentVertexBuffers, sizeof( rEntry.auiBuffers ) ) == 0 &&
		     TUtil::MemCompare( rEntry.auiStrides, m_auiCurrentStrides, sizeof( rEntry.auiStrides ) ) == 0 &&
		     TUtil::MemCompare( rEntry.auiOffsets, m_auiCurrentOffsets, sizeof( rEntry.auiOffsets ) ) == 0 )
		{
			if ( &rEntry != m_pAppliedVertexArray )
			{
				glBindVertexArray( rEntry.uiVertexArray );
				m_pAppliedVertexArray  = &rEntry;
				m_uiAppliedIndexBuffer = rEntry.uiIndexBuffer;
			}

			return;
		}

		uiIndex = ( uiIndex + 1 ) & ( VERTEX_ARRAY_TABLE_SIZE - 1 );
	}

	if ( m_uiNumVertexArrays >= MAX_VERTEX_ARRAYS )
	{
		ClearVertexArrays();
		BindCurrentVertexArray();
		return;
	}

	VERTEXARRAY& rEntry = m_pVertexArrays[ uiIndex ];
	rEntry.pLayout      = m_pCurrentLayout;
	TUtil::MemCopy( rEntry.auiBuffers, m_auiCurrentVertexBuffers, sizeof( rEntry.auiBuffers ) );
	TUtil::MemCopy( rEntry.auiStrides, m_auiCurrentStrides, sizeof( rEntry.auiStrides ) );
	TUtil::MemCopy( rEntry.auiOffsets, m_auiCurrentOffsets, sizeof( rEntry.auiOffsets ) );
	rEntry.uiIndexBuffer = 0;

	glGenVertexArrays( 1, &rEntry.uiVertexArray );
	glBindVertexArray( rEntry.uiVertexArray );
	m_uiNumVertexArrays++;

	TUINT32 uiAttributeMask = 0;
	SpecifyVertexAttributes( uiAttributeMask );

	for ( TUINT i = 0; i < MAX_VERTEX_ATTRIBUTES; i++ )
	{
		if ( uiAttributeMask & ( 1u << i ) )
			glEnableVertexAttribArray( i );
	}

	m_pAppliedVertexArray  = &rEntry;
	m_uiAppliedIndexBuffer = 0;
}

TBOOL TRenderGLInterface::CanSkipAlphaTest() const
{
	if ( !m_bAlphaTestEnable )
		return TTRUE;

	// With blending the alpha matters, and it can also come from the vertices
	if ( m_oCurrentBlend.bBlendEnable || m_eAlphaFunc != GL_GREATER || m_uiAlphaRef > ALPHA_TEST_MAX_REF )
		return TFALSE;

	return IsTextureOpaque( m_auiCurrentTextures[ 0 ] ) && ( m_auiCurrentTextures[ 1 ] == 0 || IsTextureOpaque( m_auiCurrentTextures[ 1 ] ) );
}

void TRenderGLInterface::ApplyDesiredState()
{
	const TBOOL bAll = !m_bStateCacheValid;

	if ( bAll )
	{
		BindDefaultVertexArray();

		m_uiAppliedProgram          = ~GLuint( 0 );
		m_pAppliedLayout            = TNULL;
		m_uiAppliedAttributeMask    = 0;
		m_uiUploadedConstantProgram = ~GLuint( 0 );
		m_bConstantBufferChanged    = TTRUE;

		for ( TUINT i = 0; i < MAX_VERTEX_ATTRIBUTES; i++ )
			glDisableVertexAttribArray( i );

		TUtil::MemSet( m_auiAppliedVertexBuffers, 0xFF, sizeof( m_auiAppliedVertexBuffers ) );
		TUtil::MemSet( m_auiAppliedTextures, 0xFF, sizeof( m_auiAppliedTextures ) );
		TUtil::MemSet( m_auiAppliedSamplers, 0xFF, sizeof( m_auiAppliedSamplers ) );
		m_uiAppliedIndexBuffer = ~GLuint( 0 );
		m_bStateCacheValid     = TTRUE;
	}

	const GLuint uiProgram = ( m_uiCurrentNoAlphaTestProgram && CanSkipAlphaTest() ) ? m_uiCurrentNoAlphaTestProgram : m_uiCurrentProgram;

	if ( uiProgram != m_uiAppliedProgram )
	{
		glUseProgram( uiProgram );
		m_uiAppliedProgram = uiProgram;
	}

	// ES 3.0 has no base vertex draws, the offset of the vertices is in the attribute pointers
	TBOOL bLayoutChanged = m_pCurrentLayout != m_pAppliedLayout;

	for ( TUINT i = 0; i < MAX_VERTEX_STREAMS && !bLayoutChanged; i++ )
	{
		bLayoutChanged = m_auiCurrentVertexBuffers[ i ] != m_auiAppliedVertexBuffers[ i ] ||
		    m_auiCurrentStrides[ i ] != m_auiAppliedStrides[ i ] ||
		    m_auiCurrentOffsets[ i ] != m_auiAppliedOffsets[ i ];
	}

	if ( bLayoutChanged && m_pCurrentLayout )
	{
		if ( m_bImmediateDraw )
		{
			// Their offsets change every draw, these stay on the default vertex array
			BindDefaultVertexArray();

			TUINT32 uiAttributeMask = 0;
			SpecifyVertexAttributes( uiAttributeMask );

			const TUINT32 uiChangedMask = uiAttributeMask ^ m_uiAppliedAttributeMask;

			for ( TUINT i = 0; i < MAX_VERTEX_ATTRIBUTES; i++ )
			{
				if ( uiChangedMask & ( 1u << i ) )
				{
					if ( uiAttributeMask & ( 1u << i ) )
						glEnableVertexAttribArray( i );
					else
						glDisableVertexAttribArray( i );
				}
			}

			m_uiAppliedAttributeMask = uiAttributeMask;
		}
		else
		{
			BindCurrentVertexArray();
		}

		m_pAppliedLayout = m_pCurrentLayout;

		for ( TUINT i = 0; i < MAX_VERTEX_STREAMS; i++ )
		{
			m_auiAppliedVertexBuffers[ i ] = m_auiCurrentVertexBuffers[ i ];
			m_auiAppliedStrides[ i ]       = m_auiCurrentStrides[ i ];
			m_auiAppliedOffsets[ i ]       = m_auiCurrentOffsets[ i ];
		}
	}

	if ( m_uiCurrentIndexBuffer != m_uiAppliedIndexBuffer )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_uiCurrentIndexBuffer );
		m_uiAppliedIndexBuffer = m_uiCurrentIndexBuffer;

		if ( m_pAppliedVertexArray )
			m_pAppliedVertexArray->uiIndexBuffer = m_uiCurrentIndexBuffer;
	}

	for ( TUINT i = 0; i < NUM_TEXTURE_STAGES; i++ )
	{
		if ( m_auiCurrentTextures[ i ] != m_auiAppliedTextures[ i ] )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, m_auiCurrentTextures[ i ] );
			m_auiAppliedTextures[ i ] = m_auiCurrentTextures[ i ];
		}

		if ( m_auiCurrentSamplers[ i ] != m_auiAppliedSamplers[ i ] )
		{
			glBindSampler( i, m_auiCurrentSamplers[ i ] );
			m_auiAppliedSamplers[ i ] = m_auiCurrentSamplers[ i ];
		}
	}
}

static void SetGLEnabled( GLenum a_eCapability, TBOOL a_bEnabled )
{
	if ( a_bEnabled )
		glEnable( a_eCapability );
	else
		glDisable( a_eCapability );
}

void TRenderGLInterface::UpdateRenderStates()
{
	UpdateSamplerStates();

	const TBOOL bAll = !m_bStateCacheValid;

	const BLENDSTATE& rBlend  = m_oCurrentBlend;
	BLENDSTATE&       rABlend = m_oAppliedBlend;

	if ( bAll || rBlend.bBlendEnable != rABlend.bBlendEnable )
		SetGLEnabled( GL_BLEND, rBlend.bBlendEnable );

	if ( bAll || rBlend.SrcBlend != rABlend.SrcBlend || rBlend.DestBlend != rABlend.DestBlend || rBlend.SrcBlendAlpha != rABlend.SrcBlendAlpha || rBlend.DestBlendAlpha != rABlend.DestBlendAlpha )
		glBlendFuncSeparate( rBlend.SrcBlend, rBlend.DestBlend, rBlend.SrcBlendAlpha, rBlend.DestBlendAlpha );

	if ( bAll || rBlend.BlendOp != rABlend.BlendOp || rBlend.BlendOpAlpha != rABlend.BlendOpAlpha )
		glBlendEquationSeparate( rBlend.BlendOp, rBlend.BlendOpAlpha );

	if ( bAll || rBlend.bColorWrite != rABlend.bColorWrite || rBlend.bAlphaWrite != rABlend.bAlphaWrite )
		glColorMask( rBlend.bColorWrite, rBlend.bColorWrite, rBlend.bColorWrite, rBlend.bAlphaWrite );

	if ( bAll || rBlend.fBlendFactorAlpha != rABlend.fBlendFactorAlpha )
		glBlendColor( 1.0f, 1.0f, 1.0f, rBlend.fBlendFactorAlpha );

	rABlend = rBlend;

	const DEPTHSTATE& rDepth  = m_oCurrentDepth;
	DEPTHSTATE&       rADepth = m_oAppliedDepth;

	if ( bAll || rDepth.bDepthEnable != rADepth.bDepthEnable )
		SetGLEnabled( GL_DEPTH_TEST, rDepth.bDepthEnable );

	if ( bAll || rDepth.bDepthWrite != rADepth.bDepthWrite )
		glDepthMask( rDepth.bDepthWrite );

	if ( bAll || rDepth.DepthFunc != rADepth.DepthFunc )
		glDepthFunc( rDepth.DepthFunc );

	if ( bAll || rDepth.bStencilEnable != rADepth.bStencilEnable )
		SetGLEnabled( GL_STENCIL_TEST, rDepth.bStencilEnable );

	if ( bAll || rDepth.StencilMask != rADepth.StencilMask )
		glStencilMask( rDepth.StencilMask );

	if ( bAll || rDepth.StencilFunc != rADepth.StencilFunc || rDepth.uiStencilRef != rADepth.uiStencilRef || rDepth.StencilMask != rADepth.StencilMask )
		glStencilFunc( rDepth.StencilFunc, rDepth.uiStencilRef, rDepth.StencilMask );

	if ( bAll || rDepth.StencilFailOp != rADepth.StencilFailOp || rDepth.StencilDepthFailOp != rADepth.StencilDepthFailOp || rDepth.StencilPassOp != rADepth.StencilPassOp )
		glStencilOp( rDepth.StencilFailOp, rDepth.StencilDepthFailOp, rDepth.StencilPassOp );

	rADepth = rDepth;

	const RASTERIZERSTATE& rRasterizer  = m_oCurrentRasterizer;
	RASTERIZERSTATE&       rARasterizer = m_oAppliedRasterizer;

	if ( bAll || rRasterizer.eCullMode != rARasterizer.eCullMode )
	{
		SetGLEnabled( GL_CULL_FACE, rRasterizer.eCullMode != CULL_NONE );

		if ( rRasterizer.eCullMode != CULL_NONE )
			glCullFace( rRasterizer.eCullMode == CULL_FRONT ? GL_FRONT : GL_BACK );
	}

	if ( bAll || rRasterizer.bScissorEnable != rARasterizer.bScissorEnable )
		SetGLEnabled( GL_SCISSOR_TEST, rRasterizer.bScissorEnable );

	if ( bAll || rRasterizer.iDepthBias != rARasterizer.iDepthBias || rRasterizer.fSlopeScaledDepthBias != rARasterizer.fSlopeScaledDepthBias )
	{
		SetGLEnabled( GL_POLYGON_OFFSET_FILL, rRasterizer.iDepthBias != 0 || rRasterizer.fSlopeScaledDepthBias != 0.0f );
		glPolygonOffset( rRasterizer.fSlopeScaledDepthBias, TFLOAT( rRasterizer.iDepthBias ) );
	}

	rARasterizer = rRasterizer;
}

void TRenderGLInterface::ResetStateCache()
{
	m_uiDirtySamplers = ( 1u << NUM_TEXTURE_STAGES ) - 1;

	InvalidateStateCache();

	m_uiImmediateVertexCurrentOffset = 0;
	m_uiImmediateIndexCurrentOffset  = 0;
}

// The alpha channel blends the same way unless SetDstAlpha replaces it
static TBOOL IsDstAlpha( const TRenderGLInterface::BLENDSTATE& a_rState )
{
	return a_rState.SrcBlendAlpha == GL_CONSTANT_ALPHA;
}

// Alpha blend factors can't use the colour ones
static GLenum GetAlphaBlend( GLenum a_eBlend )
{
	switch ( a_eBlend )
	{
		case GL_SRC_COLOR: return GL_SRC_ALPHA;
		case GL_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_ALPHA;
		case GL_DST_COLOR: return GL_DST_ALPHA;
		case GL_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_ALPHA;
		default: return a_eBlend;
	}
}

void TRenderGLInterface::SetSrcBlend( GLenum a_eBlend )
{
	m_oCurrentBlend.SrcBlend = a_eBlend;

	if ( !IsDstAlpha( m_oCurrentBlend ) )
		m_oCurrentBlend.SrcBlendAlpha = GetAlphaBlend( a_eBlend );
}

void TRenderGLInterface::SetDestBlend( GLenum a_eBlend )
{
	m_oCurrentBlend.DestBlend = a_eBlend;

	if ( !IsDstAlpha( m_oCurrentBlend ) )
		m_oCurrentBlend.DestBlendAlpha = GetAlphaBlend( a_eBlend );
}

void TRenderGLInterface::SetBlendOp( GLenum a_eBlendOp )
{
	m_oCurrentBlend.BlendOp = a_eBlendOp;

	if ( !IsDstAlpha( m_oCurrentBlend ) )
		m_oCurrentBlend.BlendOpAlpha = a_eBlendOp;
}

TVector4 TRenderGLInterface::GetAlphaTestParams() const
{
	// 1 is never, the numbers follow the order of the GL functions
	return TVector4( m_bAlphaTestEnable ? 1.0f : 0.0f, m_uiAlphaRef / 255.0f, TFLOAT( m_eAlphaFunc - GL_NEVER + 1 ), 0.0f );
}

void TRenderGLInterface::GetFogParams( TVector4& a_rParams, TVector4& a_rColour ) const
{
	a_rParams = TVector4( m_bFogEnable ? 1.0f : 0.0f, m_fFogStart, m_fFogEnd, 0.0f );
	a_rColour = TVector4( ( ( m_uiFogColour >> 16 ) & 0xFF ) / 255.0f, ( ( m_uiFogColour >> 8 ) & 0xFF ) / 255.0f, ( m_uiFogColour & 0xFF ) / 255.0f, 1.0f );
}

void TRenderGLInterface::SetFixedFunctionShader()
{
	m_bFixedFunction   = TTRUE;
	m_uiCurrentProgram = m_uiFixedFunctionProgram;
	m_uiCurrentNoAlphaTestProgram = FindNoAlphaTestProgram( m_uiCurrentProgram );
	m_pCurrentLayout   = &ms_oFixedFunctionLayout;
}

void TRenderGLInterface::SetShaders( GLuint a_uiProgram, const TGLInputLayout* a_pInputLayout )
{
	m_bFixedFunction   = TFALSE;
	m_uiCurrentProgram = a_uiProgram;
	m_uiCurrentNoAlphaTestProgram = FindNoAlphaTestProgram( a_uiProgram );
	m_pCurrentLayout   = a_pInputLayout;
}

void TRenderGLInterface::SetTransform( TRANSFORM a_eTransform, const TMatrix44& a_rMatrix )
{
	m_aTransforms[ a_eTransform ] = a_rMatrix;
}

void TRenderGLInterface::SetTexture( TUINT a_uiStage, GLuint a_uiTexture )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_auiCurrentTextures[ a_uiStage ] = a_uiTexture;
}

void TRenderGLInterface::SetTextureMinFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMinFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderGLInterface::SetTextureMagFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMagFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderGLInterface::SetTextureMipFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter )
{
	TASSERT( a_uiStage < NUM_TEXTURE_STAGES );

	m_aMipFilters[ a_uiStage ] = a_eFilter;
	m_uiDirtySamplers |= 1 << a_uiStage;
}

void TRenderGLInterface::CaptureStateBlock( STATEBLOCK& a_rStateBlock ) const
{
	a_rStateBlock.oDepth           = m_oCurrentDepth;
	a_rStateBlock.oRasterizer      = m_oCurrentRasterizer;
	a_rStateBlock.oBlend           = m_oCurrentBlend;
	a_rStateBlock.bAlphaTestEnable = m_bAlphaTestEnable;
	a_rStateBlock.eAlphaFunc       = m_eAlphaFunc;
	a_rStateBlock.uiAlphaRef       = m_uiAlphaRef;
	a_rStateBlock.bFogEnable       = m_bFogEnable;
	a_rStateBlock.eTextureOp       = m_eTextureOp;
	a_rStateBlock.uiTexture        = m_auiCurrentTextures[ 0 ];
	a_rStateBlock.eMinFilter       = m_aMinFilters[ 0 ];
	a_rStateBlock.eMagFilter       = m_aMagFilters[ 0 ];
	a_rStateBlock.eMipFilter       = m_aMipFilters[ 0 ];
}

void TRenderGLInterface::ApplyStateBlock( const STATEBLOCK& a_rStateBlock )
{
	m_oCurrentDepth      = a_rStateBlock.oDepth;
	m_oCurrentRasterizer = a_rStateBlock.oRasterizer;
	m_oCurrentBlend      = a_rStateBlock.oBlend;
	m_bAlphaTestEnable   = a_rStateBlock.bAlphaTestEnable;
	m_eAlphaFunc         = a_rStateBlock.eAlphaFunc;
	m_uiAlphaRef         = a_rStateBlock.uiAlphaRef;
	m_bFogEnable         = a_rStateBlock.bFogEnable;
	m_eTextureOp         = a_rStateBlock.eTextureOp;

	SetTexture( 0, a_rStateBlock.uiTexture );
	SetTextureMinFilter( 0, a_rStateBlock.eMinFilter );
	SetTextureMagFilter( 0, a_rStateBlock.eMagFilter );
	SetTextureMipFilter( 0, a_rStateBlock.eMipFilter );
}

void TRenderGLInterface::UpdateSamplerStates()
{
	for ( TUINT32 i = 0; m_uiDirtySamplers != 0; i++ )
	{
		if ( !( m_uiDirtySamplers & ( 1 << i ) ) )
			continue;

		m_uiDirtySamplers &= ~( 1 << i );

		m_auiCurrentSamplers[ i ] = GetSamplerState( m_aMinFilters[ i ], m_aMagFilters[ i ], m_aMipFilters[ i ], m_aAddressU[ i ], m_aAddressV[ i ] );
	}
}

void TRenderGLInterface::UpdateFixedFunctionConstants()
{
	TMatrix44 oWorldView;
	oWorldView.Multiply( m_aTransforms[ TRANSFORM_VIEW ], m_aTransforms[ TRANSFORM_WORLD ] );

	TMatrix44 oWorldViewProjection;
	oWorldViewProjection.Multiply( m_aTransforms[ TRANSFORM_PROJECTION ], oWorldView );

	TVector4 vParams = GetAlphaTestParams();
	vParams.w        = ( m_eTextureOp == TEXTUREOP_MODULATE && m_auiCurrentTextures[ 0 ] ) ? 1.0f : 0.0f;

	SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );
	SetVec4InVSBuffer( 4, &vParams );
}

void TRenderGLInterface::SetStencil( TBOOL a_bEnable, GLenum a_eFunc, TUINT a_uiStencilRef, TUINT8 a_uiMask, GLenum a_eFailOp, GLenum a_eDepthFailOp, GLenum a_ePassOp )
{
	m_oCurrentDepth.bStencilEnable     = a_bEnable;
	m_oCurrentDepth.StencilMask        = a_uiMask;
	m_oCurrentDepth.StencilFunc        = a_eFunc;
	m_oCurrentDepth.StencilFailOp      = a_eFailOp;
	m_oCurrentDepth.StencilDepthFailOp = a_eDepthFailOp;
	m_oCurrentDepth.StencilPassOp      = a_ePassOp;
	m_oCurrentDepth.uiStencilRef       = a_uiStencilRef;
}

void TRenderGLInterface::SetDepthBias( TINT a_iDepthBias, TFLOAT a_fSlopeScaledDepthBias )
{
	m_oCurrentRasterizer.iDepthBias            = a_iDepthBias;
	m_oCurrentRasterizer.fSlopeScaledDepthBias = a_fSlopeScaledDepthBias;
}

void TRenderGLInterface::SetDstAlpha( TFLOAT a_fAlpha )
{
	if ( a_fAlpha >= 0.0f )
	{
		m_oCurrentBlend.fBlendFactorAlpha = a_fAlpha;
		m_oCurrentBlend.BlendOpAlpha      = GL_FUNC_ADD;
		m_oCurrentBlend.SrcBlendAlpha     = GL_CONSTANT_ALPHA;
		m_oCurrentBlend.DestBlendAlpha    = GL_ZERO;
	}
	else
	{
		m_oCurrentBlend.BlendOpAlpha   = m_oCurrentBlend.BlendOp;
		m_oCurrentBlend.SrcBlendAlpha  = GetAlphaBlend( m_oCurrentBlend.SrcBlend );
		m_oCurrentBlend.DestBlendAlpha = GetAlphaBlend( m_oCurrentBlend.DestBlend );
	}
}

TOSHI_NAMESPACE_END
