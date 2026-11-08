#pragma once
#include "Render/TRenderInterface.h"
#include "Render/TOrderTable.h"
#include "Toshi/T2Map.h"
#include "TRenderAdapter_GL.h"

#ifdef TOSHI_SKU_WASM
#  include "Platform/WASM/TWindow_WASM.h"
#else
#  include "Platform/GL/TMSWindow.h"
#endif
#include "Platform/GL/TGLHeaders.h"

TOSHI_NAMESPACE_START

#ifdef TOSHI_SKU_WASM
using TGLWindow = TWindow_WASM;
#else
using TGLWindow = TMSWindow;
#endif

class TDebugGLText;
class TRenderCaptureGL;

// One vertex attribute. A location that no attribute covers takes the generic value (0, 0, 0, 1).
struct TGLVertexAttribute
{
	TUINT8  uiLocation;
	TUINT8  uiStream;
	TUINT8  uiNumComponents;
	TUINT8  bNormalized;
	GLenum  eType;
	TUINT32 uiOffset;
};

struct TGLInputLayout
{
	const TGLVertexAttribute* pAttributes;
	TUINT                     uiNumAttributes;
};

struct TGLTextureInfo
{
	TUINT  Width;
	TUINT  Height;
	TUINT  MipLevels;
	GLenum Format;
};

class MODAPI TRenderGLInterface : public TRenderInterface
{
public:
	TDECLARE_CLASS( TRenderGLInterface, TRenderInterface );

	friend TGLWindow;

	using CLEARFLAGS = TUINT8;
	enum CLEARFLAGS_ : CLEARFLAGS
	{
		CLEARFLAGS_NONE    = 0,
		CLEARFLAGS_TARGET  = BITFLAG( 0 ),
		CLEARFLAGS_DEPTH   = BITFLAG( 1 ),
		CLEARFLAGS_STENCIL = BITFLAG( 2 ),
	};

	using t_DeviceLostCallback  = TBOOL ( * )();
	using t_DeviceFoundCallback = TBOOL ( * )();

	static constexpr TUINT32 CONSTANT_BUFFER_SIZE         = 0x1000;
	static constexpr TUINT32 IMMEDIATE_VERTEX_BUFFER_SIZE = 0x100000;
	static constexpr TUINT32 IMMEDIATE_INDEX_BUFFER_SIZE  = 0x10000;
	static constexpr TUINT32 SMALL_DRAW_BUFFER_SIZE       = 0x400;
	static constexpr TUINT32 NUM_CONSTANT_BUFFERS         = 1024;
	static constexpr TUINT32 VERTEX_ARRAY_TABLE_SIZE      = 8192; // power of two
	static constexpr TUINT32 NUM_STRIP_BATCH_BUFFERS      = 256;
	static constexpr TUINT32 STRIP_BATCH_MAX_INDICES      = 4096;
	static constexpr TUINT32 MAX_VERTEX_ARRAYS            = 4096;
	static constexpr TUINT32 NUM_SMALL_DRAW_BUFFERS       = 512;
	static constexpr TUINT32 NUM_TEXTURE_STAGES           = 8;
	static constexpr TUINT   MAX_VERTEX_STREAMS           = 3;
	static constexpr TUINT   MAX_VERTEX_ATTRIBUTES        = 8;

	enum TRANSFORM : TINT
	{
		TRANSFORM_WORLD,
		TRANSFORM_VIEW,
		TRANSFORM_PROJECTION,
		TRANSFORM_NUMOF
	};

	enum TEXTUREFILTER : TINT
	{
		TEXTUREFILTER_NONE,
		TEXTUREFILTER_POINT,
		TEXTUREFILTER_LINEAR,
		TEXTUREFILTER_NUMOF
	};

	enum TEXTUREOP : TINT
	{
		TEXTUREOP_SELECTDIFFUSE,
		TEXTUREOP_MODULATE,
	};

	// Same meaning as in D3D: front faces are the clockwise ones
	enum CULLMODE : TINT
	{
		CULL_NONE,
		CULL_FRONT,
		CULL_BACK,
	};

	struct BLENDSTATE
	{
		GLenum BlendOp;
		GLenum BlendOpAlpha;
		GLenum SrcBlend;
		GLenum DestBlend;
		GLenum SrcBlendAlpha;
		GLenum DestBlendAlpha;
		TBOOL  bBlendEnable;
		TBOOL  bColorWrite;
		TBOOL  bAlphaWrite;
		TFLOAT fBlendFactorAlpha;
	};

	struct DEPTHSTATE
	{
		TBOOL  bDepthEnable;
		TBOOL  bDepthWrite;
		GLenum DepthFunc;
		TBOOL  bStencilEnable;
		TUINT8 StencilMask;
		GLenum StencilFunc;
		GLenum StencilFailOp;
		GLenum StencilDepthFailOp;
		GLenum StencilPassOp;
		TUINT  uiStencilRef;
	};

	struct RASTERIZERSTATE
	{
		CULLMODE eCullMode;
		TBOOL    bScissorEnable;
		TINT     iDepthBias;
		TFLOAT   fSlopeScaledDepthBias;
	};

	enum TEXTUREFLAGS_ : TUINT8
	{
		TEXTUREFLAGS_NONE            = 0,
		TEXTUREFLAGS_GENERATEMIPMAPS = BITFLAG( 1 ),
	};

public:
	TRenderGLInterface();
	~TRenderGLInterface();

	//-----------------------------------------------------------------------------
	// TRenderInterface
	//-----------------------------------------------------------------------------
	virtual TBOOL                         CreateDisplay( const DISPLAYPARAMS& a_rParams ) OVERRIDE;
	virtual TBOOL                         DestroyDisplay() OVERRIDE;
	virtual TBOOL                         Update( float a_fDeltaTime ) OVERRIDE;
	virtual TBOOL                         BeginScene() OVERRIDE;
	virtual TBOOL                         EndScene() OVERRIDE;
	virtual TRenderAdapter::Mode::Device* GetCurrentDevice() OVERRIDE;
	virtual DISPLAYPARAMS*                GetCurrentDisplayParams() OVERRIDE;
	virtual TBOOL                         Create() OVERRIDE;
	virtual TBOOL                         Destroy() OVERRIDE;
	virtual void                          RenderIndexPrimitive( TINT a_ePrimitiveType, TINT a_iNumVertices, TINT a_iNumPrimitives, TINT a_pIndices, TINT a_pVertices, TINT a_iVertexStride ) OVERRIDE;
	virtual float                         GetPixelAspectRatio() OVERRIDE;
	virtual TBOOL                         SetPixelAspectRatio( float a_fPixelAspectRatio ) OVERRIDE;
	virtual void                          FlushOrderTables() OVERRIDE;
	virtual TBOOL                         IsTextureFormatSupported( TTEXTURERESOURCEFORMAT a_eTextureFormat ) OVERRIDE;
	virtual TBOOL                         Supports32BitTextures() OVERRIDE;
	virtual TRenderContext*               CreateRenderContext() OVERRIDE;
	virtual TRenderCapture*               CreateCapture() OVERRIDE;
	virtual void                          DestroyCapture( TRenderCapture* a_pRenderCapture ) OVERRIDE;
	virtual void*                         CreateUnknown( const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ) OVERRIDE;
	virtual TModel*                       CreateModelTMD( TTMD* a_pTMD, TBOOL a_bLoad ) OVERRIDE;
	virtual TModel*                       CreateModelTMDFile( const TCHAR* a_szFilePath, TBOOL a_bLoad ) OVERRIDE;
	virtual TModel*                       CreateModelTRB( const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen ) OVERRIDE;
	virtual void                          OnInitializationFailureDevice() OVERRIDE;
	virtual void                          OnInitializationFailureDisplay() OVERRIDE;
	virtual TDebugText*                   CreateDebugText() OVERRIDE;
	virtual void                          DestroyDebugText() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL RecreateDisplay( const DISPLAYPARAMS& a_rDisplayParams );

	TBOOL          ChangeDisplay( TUINT32 a_uiWidth, TUINT32 a_uiHeight, TBOOL a_bWindowed, TBOOL a_bBorderless );
	virtual void   SetContrast( TFLOAT a_fConstrast );
	virtual void   SetBrightness( TFLOAT a_fBrightness );
	virtual void   SetGamma( TFLOAT a_fGamma );
	virtual void   SetSaturate( TFLOAT a_fSaturate );
	virtual TFLOAT GetContrast() const;
	virtual TFLOAT GetBrightness() const;
	virtual TFLOAT GetGamma() const;
	virtual TFLOAT GetSaturate() const;
	virtual void   UpdateColourSettings();
	virtual TBOOL  IsCapableColourCorrection();
	virtual void   EnableColourCorrection( TBOOL a_bEnable );
	virtual void   ForceEnableColourCorrection( TBOOL a_bEnable );
	virtual TBOOL  IsColourCorrection();

	TBOOL Create( const TCHAR* a_szWindowName );

	static TDebugGLText* InitDebugText( TRenderGLInterface* a_pRenderer, TINT a_iBufferSize );

	void SetDeviceDefaultStates();
	void SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate = TEXCOORD_UV );
	void ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, CLEARFLAGS a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil );

	void Exit() { m_bExited = TTRUE; }

	void RegisterOrderTable( TOrderTable* a_pOrderTable );

	TFORCEINLINE TGLWindow*             GetWindow() { return &m_Window; }
	TFORCEINLINE TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }

	TFORCEINLINE TBOOL IsDeviceLost() const { return m_bFailed; }

	void SetDeviceFoundCallback( t_DeviceFoundCallback a_fnOnDeviceFoundCallback ) { m_fnOnDeviceFoundCallback = a_fnOnDeviceFoundCallback; }
	void SetDeviceLostCallback( t_DeviceLostCallback a_fnOnDeviceLostCallback ) { m_fnOnDeviceLostCallback = a_fnOnDeviceLostCallback; }

	TFORCEINLINE TUINT32 GetBackBufferWidth() const { return m_uiBackBufferWidth; }
	TFORCEINLINE TUINT32 GetBackBufferHeight() const { return m_uiBackBufferHeight; }

	//-----------------------------------------------------------------------------
	// Resources
	//-----------------------------------------------------------------------------

	// a_pSrcData holds a_uiWidth * a_uiHeight RGBA bytes
	GLuint CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, const void* a_pSrcData, TUINT8 a_uiFlags );
	void   UpdateTexture( GLuint a_uiTexture, TUINT32 a_uiWidth, TUINT32 a_uiHeight, const void* a_pSrcData );
	void   DestroyTexture( GLuint a_uiTexture );

	// A new RGBA image from the first mip of a DDS or TGA file, TFree it
	static TUINT8* DecodeImage( const void* a_pData, TUINT32 a_uiDataSize, TUINT32& a_rWidth, TUINT32& a_rHeight );

	// Reads a DDS or a TGA file
	GLuint CreateTextureFromFileInMemory( const void* a_pData, TUINT32 a_uiDataSize, TGLTextureInfo* a_pInfo );

	GLuint CreateBuffer( GLenum a_eTarget, TUINT32 a_uiDataSize, const void* a_pData, GLenum a_eUsage );
	void   UpdateBuffer( GLuint a_uiBuffer, GLenum a_eTarget, TUINT32 a_uiOffset, TUINT32 a_uiDataSize, const void* a_pData, TUINT32 a_uiDiscardSize = 0 );
	void   DestroyBuffer( GLuint a_uiBuffer );

	// a_szSource has the vertex shader under VERTEX_SHADER and the fragment shader under FRAGMENT_SHADER.
	// The textures are named g_Texture0, g_Texture1... after their stages, the constants are a block named Constants
	static GLuint CompileProgram( const TCHAR* a_szSource, const TCHAR* a_szDefines = TNULL );
	static void   DestroyProgram( GLuint a_uiProgram );

	//-----------------------------------------------------------------------------
	// Drawing
	//-----------------------------------------------------------------------------
	void DrawNonIndexed( GLenum a_eTopology, GLuint a_uiVertexBuffer, TUINT a_uiVertexCount, TUINT a_uiStride, TUINT a_uiStartVertex, TUINT a_uiOffset );
	void DrawIndexed( GLenum a_eTopology, TUINT a_uiIndexCount, GLuint a_uiIndexBuffer, TUINT a_uiIndexBufferOffset, GLenum a_eIndexFormat, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiOffset );
	void DrawNonIndexedImmediate( GLenum a_eTopology, const void* a_pVertices, TUINT a_uiVertexCount, TUINT a_uiStride );
	void DrawImmediately( GLenum a_eTopology, TUINT a_uiIndexCount, const void* a_pIndices, GLenum a_eIndexFormat, const void* a_pVertices, TUINT a_uiStride, TUINT a_uiVertexCount );

	// Strips drawn one after another with the same state become one draw. A draw is expensive on phones, way more
	// than the indices. The state has to stay the same until FlushStripBatch, returns TFALSE if it can't take this one
	TBOOL AddStripToBatch( const TUINT16* a_pIndices, TUINT a_uiIndexCount, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiVertexOffset );
	void  FlushStripBatch();
	TBOOL IsStripBatchOpen() const { return m_uiStripBatchCount != 0; }

	void SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount = 1 );

	// The next upload of the constants only has to include this many, the rest isn't read by the next draw
	void SetConstantUploadLimit( TINT a_iNumConstants ) { m_uiConstantUploadLimit = a_iNumConstants * sizeof( TVector4 ); }

	// Top left origin, like D3D
	void SetViewport( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TFLOAT a_fMinDepth = 0.0f, TFLOAT a_fMaxDepth = 1.0f );
	void GetViewport( TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight ) const;
	void SetScissorRect( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );

	void SetVertexBuffer( TUINT a_uiStream, GLuint a_uiVertexBuffer, TUINT a_uiStride, TUINT a_uiOffset );
	void SetIndexBuffer( GLuint a_uiIndexBuffer, GLenum a_eFormat, TUINT a_uiOffset );

	// The next draw sends every state again, for code that changed the GL state behind the renderer
	void InvalidateStateCache();

	// Drops a_uiTexture from every stage, call it before deleting a texture that may be set
	void InvalidateShaderResource( GLuint a_uiTexture );

	void UpdateRenderStates();
	void ResetStateCache();

	// The capture copies the next frame before it's presented
	void SetPendingCapture( TRenderCaptureGL* a_pCapture ) { m_pPendingCapture = a_pCapture; }

	// The states a D3D8 state block of TGLFont records
	struct STATEBLOCK
	{
		DEPTHSTATE      oDepth;
		RASTERIZERSTATE oRasterizer;
		BLENDSTATE      oBlend;
		TBOOL           bAlphaTestEnable;
		GLenum          eAlphaFunc;
		TUINT8          uiAlphaRef;
		TBOOL           bFogEnable;
		TEXTUREOP       eTextureOp;
		GLuint          uiTexture;
		TEXTUREFILTER   eMinFilter;
		TEXTUREFILTER   eMagFilter;
		TEXTUREFILTER   eMipFilter;
	};

	void CaptureStateBlock( STATEBLOCK& a_rStateBlock ) const;
	void ApplyStateBlock( const STATEBLOCK& a_rStateBlock );

	// The fixed function pipeline of D3D8 for FVF XYZ | DIFFUSE | TEX1 vertices
	void             SetFixedFunctionShader();
	void             SetPositionColourLayout() { m_pCurrentLayout = &ms_oPositionColourLayout; }
	void             SetShaders( GLuint a_uiProgram, const TGLInputLayout* a_pInputLayout );
	void             SetTransform( TRANSFORM a_eTransform, const TMatrix44& a_rMatrix );
	const TMatrix44& GetTransform( TRANSFORM a_eTransform ) const { return m_aTransforms[ a_eTransform ]; }
	void             SetTexture( TUINT a_uiStage, GLuint a_uiTexture );
	GLuint           GetTexture( TUINT a_uiStage ) const { return m_auiCurrentTextures[ a_uiStage ]; }
	void             SetTextureOp( TEXTUREOP a_eTextureOp ) { m_eTextureOp = a_eTextureOp; }
	void             SetTextureMinFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter );
	void             SetTextureMagFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter );
	void             SetTextureMipFilter( TUINT a_uiStage, TEXTUREFILTER a_eFilter );

	void SetAlphaBlendEnable( TBOOL a_bEnable ) { m_oCurrentBlend.bBlendEnable = a_bEnable; }
	void SetSrcBlend( GLenum a_eBlend );
	void SetDestBlend( GLenum a_eBlend );
	void SetBlendOp( GLenum a_eBlendOp );
	void SetZEnable( TBOOL a_bEnable ) { m_oCurrentDepth.bDepthEnable = a_bEnable; }
	void SetZWriteEnable( TBOOL a_bEnable ) { m_oCurrentDepth.bDepthWrite = a_bEnable; }
	void SetZFunc( GLenum a_eFunc ) { m_oCurrentDepth.DepthFunc = a_eFunc; }
	void SetAlphaTestEnable( TBOOL a_bEnable ) { m_bAlphaTestEnable = a_bEnable; }
	void SetAlphaFunc( GLenum a_eFunc ) { m_eAlphaFunc = a_eFunc; }
	void SetAlphaRef( TUINT8 a_uiRef ) { m_uiAlphaRef = a_uiRef; }

	// Enable, reference and function of the alpha test for the AlphaTest function of the shaders
	TVector4 GetAlphaTestParams() const;

	void SetFogEnable( TBOOL a_bEnable ) { m_bFogEnable = a_bEnable; }
	void SetFogColour( TUINT32 a_uiColour ) { m_uiFogColour = a_uiColour; }
	void SetFogStart( TFLOAT a_fStart ) { m_fFogStart = a_fStart; }
	void SetFogEnd( TFLOAT a_fEnd ) { m_fFogEnd = a_fEnd; }

	// Enable, start and end of the linear fog and its colour
	void GetFogParams( TVector4& a_rParams, TVector4& a_rColour ) const;

	void     SetStencil( TBOOL a_bEnable, GLenum a_eFunc, TUINT a_uiStencilRef, TUINT8 a_uiMask, GLenum a_eFailOp, GLenum a_eDepthFailOp, GLenum a_ePassOp );
	void     SetCullMode( CULLMODE a_eCullMode ) { m_oCurrentRasterizer.eCullMode = a_eCullMode; }
	CULLMODE GetCullMode() const { return m_oCurrentRasterizer.eCullMode; }
	void     SetScissorEnable( TBOOL a_bEnable ) { m_oCurrentRasterizer.bScissorEnable = a_bEnable; }
	void     SetDepthBias( TINT a_iDepthBias, TFLOAT a_fSlopeScaledDepthBias );
	void     SetColorUpdate( TBOOL a_bUpdate ) { m_oCurrentBlend.bColorWrite = a_bUpdate; }
	void     SetAlphaUpdate( TBOOL a_bUpdate ) { m_oCurrentBlend.bAlphaWrite = a_bUpdate; }

	void SetDstAlpha( TFLOAT a_fAlpha );

public:
	static void FlushShaders();

	static TFORCEINLINE TRenderGLInterface* Interface()
	{
		return TSTATICCAST( TRenderGLInterface, TRenderInterface::GetSingleton() );
	}

private:
	void OnDeviceLost();
	void OnDeviceFound();

	void CreateDeviceObjects();
	void DestroyDeviceObjects();

	void  BuildAdapterDatabase();
	TBOOL ResizeBackBuffer();

	void   FlushConstantBuffers();
	void   UpdateFixedFunctionConstants();
	void   UpdateSamplerStates();
	TBOOL CanSkipAlphaTest() const;
	void BindDefaultVertexArray();
	void ClearVertexArrays();
	void SpecifyVertexAttributes( TUINT32& a_ruiAttributeMask );
	void BindCurrentVertexArray();
	void   ApplyDesiredState();
	void   PrepareDraw();
	GLuint GetSamplerState( TEXTUREFILTER a_eMinFilter, TEXTUREFILTER a_eMagFilter, TEXTUREFILTER a_eMipFilter, ADDRESSINGMODE a_eAddressU, ADDRESSINGMODE a_eAddressV );

	TUINT32 CopyToImmediateBuffer( GLuint a_uiBuffer, GLenum a_eTarget, TUINT32 a_uiBufferSize, TUINT32& a_rCurrentOffset, const void* a_pData, TUINT32 a_uiDataSize );

private:
	static const TGLInputLayout ms_oFixedFunctionLayout;
	static const TGLInputLayout ms_oPositionColourLayout;

	TFLOAT                     m_fPixelAspectRatio;
	TGLAdapter::Mode::Device* m_pDevice;
	DISPLAYPARAMS              m_oDisplayParams;
	TGLWindow                  m_Window;
	TBOOL                      m_bExited;
	TFLOAT                     m_fContrast;
	TFLOAT                     m_fBrightness;
	TFLOAT                     m_fGamma;
	TFLOAT                     m_fSaturate;
	TBOOL                      m_bEnableColourCorrection;
	TBOOL                      m_bFailed;
	t_DeviceLostCallback       m_fnOnDeviceLostCallback;
	t_DeviceFoundCallback      m_fnOnDeviceFoundCallback;
	TPriList<TOrderTable>      m_OrderTables;

	TRenderCaptureGL* m_pPendingCapture;
	TUINT32            m_uiBackBufferWidth;
	TUINT32            m_uiBackBufferHeight;
	GLuint             m_uiVertexArray;
	TINT               m_aViewport[ 4 ];

	// A vertex array for each mesh, so a draw of it is one bind instead of all of its attributes
	struct VERTEXARRAY
	{
		const TGLInputLayout* pLayout;
		GLuint                auiBuffers[ MAX_VERTEX_STREAMS ];
		TUINT                 auiStrides[ MAX_VERTEX_STREAMS ];
		TUINT                 auiOffsets[ MAX_VERTEX_STREAMS ];
		GLuint                uiVertexArray;
		GLuint                uiIndexBuffer;
	};

	VERTEXARRAY* m_pVertexArrays;
	TUINT32      m_uiNumVertexArrays;
	VERTEXARRAY* m_pAppliedVertexArray;
	TBOOL        m_bImmediateDraw;

	// What the callers want for the next draw
	GLuint                m_uiCurrentProgram;
	GLuint                m_uiCurrentNoAlphaTestProgram;
	const TGLInputLayout* m_pCurrentLayout;
	GLuint                m_auiCurrentVertexBuffers[ MAX_VERTEX_STREAMS ];
	TUINT                 m_auiCurrentStrides[ MAX_VERTEX_STREAMS ];
	TUINT                 m_auiCurrentOffsets[ MAX_VERTEX_STREAMS ];
	GLuint                m_uiCurrentIndexBuffer;
	GLenum                m_eCurrentIndexFormat;
	TUINT                 m_uiCurrentIndexOffset;
	GLuint                m_auiCurrentTextures[ NUM_TEXTURE_STAGES ];
	GLuint                m_auiCurrentSamplers[ NUM_TEXTURE_STAGES ];
	BLENDSTATE            m_oCurrentBlend;
	DEPTHSTATE            m_oCurrentDepth;
	RASTERIZERSTATE       m_oCurrentRasterizer;

	// What the context holds
	TBOOL                 m_bStateCacheValid;
	GLuint                m_uiAppliedProgram;
	const TGLInputLayout* m_pAppliedLayout;
	GLuint                m_auiAppliedVertexBuffers[ MAX_VERTEX_STREAMS ];
	TUINT                 m_auiAppliedStrides[ MAX_VERTEX_STREAMS ];
	TUINT                 m_auiAppliedOffsets[ MAX_VERTEX_STREAMS ];
	TUINT32               m_uiAppliedAttributeMask;
	GLuint                m_uiAppliedIndexBuffer;
	GLuint                m_auiAppliedTextures[ NUM_TEXTURE_STAGES ];
	GLuint                m_auiAppliedSamplers[ NUM_TEXTURE_STAGES ];
	BLENDSTATE            m_oAppliedBlend;
	DEPTHSTATE            m_oAppliedDepth;
	RASTERIZERSTATE       m_oAppliedRasterizer;

	TUINT8* m_pConstantBufferData;
	TBOOL   m_bConstantBufferChanged;
	TUINT32 m_uiConstantUploadLimit;
	GLuint  m_auiConstantBuffers[ NUM_CONSTANT_BUFFERS ];
	TUINT32 m_uiNextConstantBuffer;
	GLuint  m_uiViewportBuffer;
	TUINT32 m_uiUploadedConstantSize;
	GLuint  m_uiUploadedConstantProgram;

	GLuint   m_auiStripBatchBuffers[ NUM_STRIP_BATCH_BUFFERS ];
	TUINT32  m_uiNextStripBatchBuffer;
	TUINT32* m_pStripBatchIndices;
	TUINT32  m_uiStripBatchCount;
	GLuint   m_uiStripBatchVertexBuffer;
	TUINT    m_uiStripBatchStride;

	GLuint  m_auiSmallDrawBuffers[ NUM_SMALL_DRAW_BUFFERS ];
	TUINT32 m_uiNextSmallDrawBuffer;
	GLuint  m_uiImmediateVertexBuffer;
	TUINT32 m_uiImmediateVertexCurrentOffset;
	GLuint  m_uiImmediateIndexBuffer;
	TUINT32 m_uiImmediateIndexCurrentOffset;

	GLuint         m_auiSamplerStates[ TEXTUREFILTER_NUMOF ][ TEXTUREFILTER_NUMOF ][ TEXTUREFILTER_NUMOF ][ ADDRESSINGMODE_NUMOF ][ ADDRESSINGMODE_NUMOF ];
	ADDRESSINGMODE m_aAddressU[ NUM_TEXTURE_STAGES ];
	ADDRESSINGMODE m_aAddressV[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER  m_aMinFilters[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER  m_aMagFilters[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER  m_aMipFilters[ NUM_TEXTURE_STAGES ];
	TUINT32        m_uiDirtySamplers;

	GLuint    m_uiFixedFunctionProgram;
	TBOOL     m_bFixedFunction;
	TMatrix44 m_aTransforms[ TRANSFORM_NUMOF ];
	TEXTUREOP m_eTextureOp;
	TBOOL     m_bAlphaTestEnable;
	GLenum    m_eAlphaFunc;
	TUINT8    m_uiAlphaRef;
	TBOOL     m_bFogEnable;
	TUINT32   m_uiFogColour;
	TFLOAT    m_fFogStart;
	TFLOAT    m_fFogEnd;

	TBOOL m_bS3TCSupported;
};

TSINGLETON_DECLARE_INHERITED_ALIAS( TRenderInterface, TRenderGLInterface, RenderGL );

TOSHI_NAMESPACE_END
