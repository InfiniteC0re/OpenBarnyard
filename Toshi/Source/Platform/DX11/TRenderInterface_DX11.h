#pragma once
#include "Render/TRenderInterface.h"
#include "Render/TOrderTable.h"
#include "Toshi/T2Map.h"
#include "TRenderAdapter_DX11.h"

#include "Platform/DX11/TMSWindow.h"

#include <d3d11_1.h>
#include <dxgi1_2.h>

TOSHI_NAMESPACE_START

class TDebugD3DText;
class TRenderCaptureD3D;
class TConstantBufferGeneric;

class MODAPI TRenderD3DInterface : public TRenderInterface
{
public:
	TDECLARE_CLASS( TRenderD3DInterface, TRenderInterface );

	friend TMSWindow;

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

	static constexpr TUINT32 SWAPCHAIN_BUFFER_COUNT       = 2;
	static constexpr TUINT32 CONSTANT_BUFFER_SIZE         = 0x1000;
	static constexpr TUINT32 NUMCONSTANTBUFFERS           = 16;
	static constexpr TUINT32 IMMEDIATE_VERTEX_BUFFER_SIZE = 0x100000;
	static constexpr TUINT32 IMMEDIATE_INDEX_BUFFER_SIZE  = 0x10000;
	static constexpr TUINT32 MAXIMUM_NUMBER_OF_MIPMAPS    = 15;
	static constexpr TUINT32 NUM_TEXTURE_STAGES           = 8;

	enum TRANSFORM : TINT
	{
		TRANSFORM_WORLD,
		TRANSFORM_VIEW,
		TRANSFORM_PROJECTION,
		TRANSFORM_NUMOF
	};

	// D3DTEXTUREFILTERTYPE
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

	union BLENDSTATE
	{
		struct
		{
			TUINT32 BlendOp : 3;               // D3D11_BLEND_OP
			TUINT32 BlendOpAlpha : 3;          // D3D11_BLEND_OP
			TUINT32 SrcBlendAlpha : 5;         // D3D11_BLEND
			TUINT32 DestBlendAlpha : 5;        // D3D11_BLEND
			TUINT32 RenderTargetWriteMask : 4; // D3D11_COLOR_WRITE_ENABLE
			TUINT32 bBlendEnable : 1;
			TUINT32 SrcBlend : 5;  // D3D11_BLEND
			TUINT32 DestBlend : 5; // D3D11_BLEND
		} Parts;

		TUINT32 uiRaw;
	};

	union DEPTHSTATE
	{
		struct
		{
			TUINT64 bDepthEnable : 1;
			TUINT64 DepthWriteMask : 1; // D3D11_DEPTH_WRITE_MASK
			TUINT64 DepthFunc : 4;      // D3D11_COMPARISON_FUNC
			TUINT64 bStencilEnable : 1;
			TUINT64 PADDING : 1;
			TUINT64 StencilReadMask : 8;
			TUINT64 StencilWriteMask : 8;
			TUINT64 FrontStencilFailOp : 4;      // D3D11_STENCIL_OP
			TUINT64 FrontStencilDepthFailOp : 4; // D3D11_STENCIL_OP
			TUINT64 FrontStencilPassOp : 4;      // D3D11_STENCIL_OP
			TUINT64 FrontStencilFunc : 4;        // D3D11_COMPARISON_FUNC
			TUINT64 BackStencilFailOp : 4;       // D3D11_STENCIL_OP
			TUINT64 BackStencilDepthFailOp : 4;  // D3D11_STENCIL_OP
			TUINT64 BackStencilPassOp : 4;       // D3D11_STENCIL_OP
			TUINT64 BackStencilFunc : 4;         // D3D11_COMPARISON_FUNC
		} Parts;

		TUINT64 uiRaw;
	};

	struct RASTERIZERSTATE
	{
		union
		{
			struct
			{
				TUINT32 FillMode : 2; // D3D11_FILL_MODE
				TUINT32 CullMode : 2; // D3D11_CULL_MODE
				TUINT32 bFrontCounterClockwise : 1;
				TUINT32 bDepthClipEnable : 1;
				TUINT32 bScissorEnable : 1;
				TUINT32 bMultisampleEnable : 1;
			} Parts;

			TUINT32 uiRaw;
		} Flags;

		INT   DepthBias;
		FLOAT SlopeScaledDepthBias;

		TBOOL operator==( const RASTERIZERSTATE& a_rOther ) const { return Flags.uiRaw == a_rOther.Flags.uiRaw && DepthBias == a_rOther.DepthBias && SlopeScaledDepthBias == a_rOther.SlopeScaledDepthBias; }
		TBOOL operator!=( const RASTERIZERSTATE& a_rOther ) const { return !( *this == a_rOther ); }
	};

	struct RasterizerStateComparator
	{
		TINT operator()( const RASTERIZERSTATE& a, const RASTERIZERSTATE& b ) const
		{
			if ( a.Flags.uiRaw != b.Flags.uiRaw ) return a.Flags.uiRaw < b.Flags.uiRaw ? -1 : 1;
			if ( a.DepthBias != b.DepthBias ) return a.DepthBias < b.DepthBias ? -1 : 1;
			if ( a.SlopeScaledDepthBias != b.SlopeScaledDepthBias ) return a.SlopeScaledDepthBias < b.SlopeScaledDepthBias ? -1 : 1;
			return 0;
		}
	};

	using DepthStateMap      = T2Map<TUINT64, ID3D11DepthStencilState*>;
	using RasterizerStateMap = T2Map<RASTERIZERSTATE, ID3D11RasterizerState*, RasterizerStateComparator>;
	using BlendStateMap      = T2Map<TUINT32, ID3D11BlendState*>;

	enum TEXTUREFLAGS_ : TUINT8
	{
		TEXTUREFLAGS_NONE            = 0,
		TEXTUREFLAGS_MIPMAPSFROMDATA = BITFLAG( 0 ),
		TEXTUREFLAGS_GENERATEMIPMAPS = BITFLAG( 1 ),
		TEXTUREFLAGS_RENDERTARGET    = BITFLAG( 2 ),
	};

public:
	TRenderD3DInterface();
	~TRenderD3DInterface();

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
	virtual TBOOL  RecreateDisplay( const DISPLAYPARAMS& a_rDisplayParams );
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

	TBOOL IsTextureFormatSupportedImpl( DXGI_FORMAT a_eFormat );

	TBOOL Create( const TCHAR* a_szWindowName );
	void  BuildAdapterDatabase();
	void  DestroyAccelTable();
	void  CreateAccelTable();
	void  GetCurrentColourRamp();

	static TDebugD3DText* InitDebugText( TRenderD3DInterface* a_pRenderer, TINT a_iBufferSize );

	void SetDeviceDefaultStates();
	void SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate = TEXCOORD_UV );
	void ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, CLEARFLAGS a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil );

	void Exit() { m_bExited = TTRUE; }

	void RegisterOrderTable( TOrderTable* a_pOrderTable );

	TFORCEINLINE TMSWindow*             GetMSWindow() { return &m_Window; }
	TFORCEINLINE TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }

	TFORCEINLINE TBOOL IsDeviceLost() const { return m_bFailed; }

	void SetDeviceFoundCallback( t_DeviceFoundCallback a_fnOnDeviceFoundCallback ) { m_fnOnDeviceFoundCallback = a_fnOnDeviceFoundCallback; }
	void SetDeviceLostCallback( t_DeviceLostCallback a_fnOnDeviceLostCallback ) { m_fnOnDeviceLostCallback = a_fnOnDeviceLostCallback; }

	//-----------------------------------------------------------------------------
	// DX11
	//-----------------------------------------------------------------------------

	TFORCEINLINE ID3D11Device*           GetDevice() const { return m_pDevice11; }
	TFORCEINLINE ID3D11DeviceContext*    GetDeviceContext() const { return m_pDeviceContext; }
	TFORCEINLINE IDXGISwapChain1*        GetSwapChain() const { return m_pSwapChain; }
	TFORCEINLINE ID3D11Texture2D*        GetBackBuffer() const { return m_pBackBuffer; }
	TFORCEINLINE ID3D11RenderTargetView* GetBackBufferRTView() const { return m_pBackBufferRTView; }

	// A depth buffer shaders can read. The read-only view keeps the depth test on while it's read,
	// it's TNULL below feature level 11_0
	struct DEPTHTARGET
	{
		ID3D11DepthStencilView*   pView;
		ID3D11DepthStencilView*   pReadOnlyView;
		ID3D11ShaderResourceView* pSRV; // a Texture2DMS view with MSAA
	};

	// The depth buffer that is rendered with, the one of the scene target while it's active
	TFORCEINLINE const DEPTHTARGET&        GetDepthTarget() const { return m_bSceneTargetActive ? m_oSceneDepth : m_oDepth; }
	TFORCEINLINE ID3D11DepthStencilView*   GetDepthStencilView() const { return GetDepthTarget().pView; }
	TFORCEINLINE ID3D11ShaderResourceView* GetDepthStencilSRV() const { return GetDepthTarget().pSRV; }
	TFORCEINLINE ID3D11DepthStencilView*   GetReadOnlyDepthStencilView() const { return GetDepthTarget().pReadOnlyView; }

	//-----------------------------------------------------------------------------
	// Scene target: an off-screen colour target with its own depth buffer, in any format and with
	// MSAA. Between BeginSceneTarget and EndSceneTarget everything renders into it (the viewport
	// clears too), then into the back buffer again. Mods use it for HDR and post processing: the
	// scene goes into it and is resolved to the back buffer before the GUI is drawn.
	//-----------------------------------------------------------------------------

	// DXGI_FORMAT_UNKNOWN is the format of the back buffer, takes effect at the next BeginSceneTarget
	void        SetSceneTargetFormat( DXGI_FORMAT a_eFormat, UINT a_uiSampleCount );
	DXGI_FORMAT GetSceneTargetFormat() const { return m_eSceneFormat; }
	UINT        GetSceneTargetSampleCount() const { return m_uiSceneSampleCount; }

	TBOOL BeginSceneTarget();
	void  EndSceneTarget();
	TBOOL IsSceneTargetActive() const { return m_bSceneTargetActive; }

	// Valid once BeginSceneTarget has been called, the shader views are Texture2DMS ones with MSAA
	TFORCEINLINE ID3D11Texture2D*          GetSceneTexture() const { return m_pSceneTexture; }
	TFORCEINLINE ID3D11RenderTargetView*   GetSceneRTView() const { return m_pSceneRTView; }
	TFORCEINLINE ID3D11ShaderResourceView* GetSceneSRV() const { return m_pSceneSRV; }
	TFORCEINLINE const DEPTHTARGET&        GetSceneDepthTarget() const { return m_oSceneDepth; }

	// What is rendered into: the scene target while it's active, or the back buffer
	TFORCEINLINE ID3D11Texture2D*        GetRenderTargetTexture() const { return m_bSceneTargetActive ? m_pSceneTexture : m_pBackBuffer; }
	TFORCEINLINE ID3D11RenderTargetView* GetRenderTargetView() const { return m_bSceneTargetActive ? m_pSceneRTView : m_pBackBufferRTView; }
	TFORCEINLINE TUINT32                 GetBackBufferWidth() const { return m_uiBackBufferWidth; }
	TFORCEINLINE TUINT32                 GetBackBufferHeight() const { return m_uiBackBufferHeight; }

	ID3D11ShaderResourceView* CreateTexture( TUINT32 a_uiWidth, TUINT32 a_uiHeight, DXGI_FORMAT a_eFormat, const void* a_pSrcData, TUINT8 a_uiFlags, D3D11_USAGE a_eUsage, TUINT32 a_uiCPUAccessFlags, TUINT32 a_uiSampleCount );

	ID3D11ShaderResourceView* CreateTextureFromFileInMemory( const void* a_pData, TUINT32 a_uiDataSize, D3D11_TEXTURE2D_DESC* a_pDesc );

	static TUINT32 GetTextureRowPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth );
	static TUINT32 GetTextureDepthPitch( DXGI_FORMAT a_eFormat, TUINT32 a_uiWidth, TUINT32 a_uiHeight );

	ID3D11Buffer* CreateBuffer( UINT a_uiBindFlags, UINT a_uiDataSize, const void* a_pData, D3D11_USAGE a_eUsage, UINT a_uiCPUAccessFlags );

	static ID3DBlob* CompileShader( const TCHAR* a_szSrcData, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const D3D_SHADER_MACRO* a_pDefines );

	// The helpers CompileShader puts before every shader, TShaderCombo serves them as TShaderCommon.hlsli
	static const TCHAR* GetCommonShaderSource();

	static TBOOL CreateVertexShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11VertexShader** a_ppVertexShader );
	static TBOOL CreatePixelShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11PixelShader** a_ppPixelShader );
	static TBOOL CreateComputeShader( const void* a_pShaderBytecode, SIZE_T a_uiBytecodeLength, ID3D11ComputeShader** a_ppComputeShader );

	// Structured buffer and its shader view. a_bDynamic makes it writable with Map (WRITE_DISCARD),
	// otherwise it's written with UpdateSubresource
	TBOOL CreateStructuredBuffer( UINT a_uiStride, UINT a_uiCount, const void* a_pData, TBOOL a_bDynamic, ID3D11Buffer** a_ppBuffer, ID3D11ShaderResourceView** a_ppSRV );

	// Typed buffer written by compute shaders and read by the other stages, a_uiExtraBindFlags
	// adds e.g. D3D11_BIND_VERTEX_BUFFER for an instance stream; a_ppSRV is optional
	TBOOL CreateTypedUAVBuffer( DXGI_FORMAT a_eFormat, UINT a_uiElementSize, UINT a_uiCount, UINT a_uiExtraBindFlags, ID3D11Buffer** a_ppBuffer, ID3D11UnorderedAccessView** a_ppUAV, ID3D11ShaderResourceView** a_ppSRV );

	// Buffer of D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS, a compute shader can patch them through the raw view
	TBOOL CreateIndirectArgsBuffer( UINT a_uiNumDraws, const void* a_pData, ID3D11Buffer** a_ppBuffer, ID3D11UnorderedAccessView** a_ppUAV );

	// Compute shaders, UAV buffers and indirect draws need feature level 11_0
	TBOOL IsComputeSupported() const { return m_pDevice11->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0; }

	void DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pVertexBuffer, UINT a_uiVertexCount, UINT a_uiStride, UINT a_uiStartVertex, UINT a_uiOffset );
	void DrawIndexed( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset );

	// The per instance data comes from vertex stream 1 (SetVertexBuffer) or from shader resources
	void DrawIndexedInstanced( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, UINT a_uiInstanceCount, ID3D11Buffer* a_pIndexBuffer, UINT a_uiIndexBufferOffset, DXGI_FORMAT a_eIndexFormat, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset, UINT a_uiStartInstance = 0 );

	// Draws with the vertex and index buffers that are set and the arguments of a CreateIndirectArgsBuffer buffer
	void DrawIndexedInstancedIndirect( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, ID3D11Buffer* a_pArgsBuffer, UINT a_uiArgsOffset );

	void DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, const void* a_pVertices, UINT a_uiVertexCount, UINT a_uiStride );
	void DrawImmediately( D3D11_PRIMITIVE_TOPOLOGY a_eTopology, UINT a_uiIndexCount, const void* a_pIndices, DXGI_FORMAT a_eIndexFormat, const void* a_pVertices, UINT a_uiStride, UINT a_uiVertexCount );

	void SetVec4InVSBuffer( TINT a_iIndex, const void* a_pSrc, TINT a_iCount = 1 );

	void FlushConstantBuffers();

	static constexpr TUINT MAX_CONSTANT_BUFFER_SLOTS = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;

	// Binds a_pBuffer (TNULL unbinds) to a slot from the next draw on, slot 0 belongs to SetVec4InVSBuffer
	void SetVSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer );
	void SetPSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer );

	// Compute shaders have no shared buffer, every slot is free
	void SetCSConstantBuffer( UINT a_uiSlot, TConstantBufferGeneric* a_pBuffer );

	// Unbinds a_pBuffer from every slot, TConstantBufferGeneric::Destroy calls it
	void UnbindConstantBuffer( TConstantBufferGeneric* a_pBuffer );

	static constexpr TUINT MAX_VERTEX_STREAMS = 3; // the vertices, the tangents of the remaster, the instances
	static constexpr TUINT MAX_SRV_SLOTS      = 16;
	static constexpr TUINT MAX_SAMPLER_SLOTS  = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
	static constexpr TUINT MAX_UAV_SLOTS      = D3D11_PS_CS_UAV_REGISTER_COUNT;

	// Everything the renderer binds on the immediate context
	struct PIPELINESTATE
	{
		ID3D11VertexShader*       pVertexShader;
		ID3D11PixelShader*        pPixelShader;
		ID3D11InputLayout*        pInputLayout;
		ID3D11RenderTargetView*   pRenderTargetView;
		ID3D11RenderTargetView*   pSecondaryRenderTargetView; // SV_Target1
		ID3D11DepthStencilView*   pDepthStencilView;
		D3D11_PRIMITIVE_TOPOLOGY  eTopology;
		ID3D11Buffer*             apVertexBuffers[ MAX_VERTEX_STREAMS ];
		UINT                      auiVertexStrides[ MAX_VERTEX_STREAMS ];
		UINT                      auiVertexOffsets[ MAX_VERTEX_STREAMS ];
		ID3D11Buffer*             pIndexBuffer;
		DXGI_FORMAT               eIndexFormat;
		UINT                      uiIndexOffset;
		ID3D11Buffer*             apVSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
		ID3D11Buffer*             apPSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
		ID3D11ShaderResourceView* apVSShaderResources[ MAX_SRV_SLOTS ];
		ID3D11ShaderResourceView* apPSShaderResources[ MAX_SRV_SLOTS ];
		ID3D11SamplerState*       apVSSamplers[ MAX_SAMPLER_SLOTS ];
		ID3D11SamplerState*       apPSSamplers[ MAX_SAMPLER_SLOTS ];

		ID3D11ComputeShader*       pComputeShader;
		ID3D11Buffer*              apCSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
		ID3D11ShaderResourceView*  apCSShaderResources[ MAX_SRV_SLOTS ];
		ID3D11UnorderedAccessView* apCSUnorderedAccessViews[ MAX_UAV_SLOTS ];
		ID3D11SamplerState*        apCSSamplers[ MAX_SAMPLER_SLOTS ];
	};

	// The setters change the desired state, ApplyDesiredState sends what differs from the
	// state the device holds; draws apply it themselves
	void SetVertexShader( ID3D11VertexShader* a_pVertexShader ) { m_oCurrent.pVertexShader = a_pVertexShader; }
	void SetPixelShader( ID3D11PixelShader* a_pPixelShader ) { m_oCurrent.pPixelShader = a_pPixelShader; }
	void SetInputLayout( ID3D11InputLayout* a_pInputLayout ) { m_oCurrent.pInputLayout = a_pInputLayout; }
	void SetRenderTargets( ID3D11RenderTargetView* a_pRenderTargetView, ID3D11DepthStencilView* a_pDepthStencilView );

	// A second colour target (SV_Target1), bound only while a_pPrimary is the first one, so the passes
	// that switch to other targets don't need to know about it. Same size and sample count as a_pPrimary
	void SetSecondaryRenderTarget( ID3D11RenderTargetView* a_pPrimary, ID3D11RenderTargetView* a_pSecondary )
	{
		m_pSecondaryRenderTargetOwner          = a_pPrimary;
		m_oCurrent.pSecondaryRenderTargetView = a_pSecondary;
	}
	void SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY a_eTopology ) { m_oCurrent.eTopology = a_eTopology; }
	void SetVertexBuffer( UINT a_uiSlot, ID3D11Buffer* a_pVertexBuffer, UINT a_uiStride, UINT a_uiOffset );
	void SetIndexBuffer( ID3D11Buffer* a_pIndexBuffer, DXGI_FORMAT a_eFormat, UINT a_uiOffset );
	void VSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView );
	void PSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView );
	void VSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler );
	void PSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler );

	void SetComputeShader( ID3D11ComputeShader* a_pComputeShader ) { m_oCurrent.pComputeShader = a_pComputeShader; }
	void CSSetShaderResource( UINT a_uiSlot, ID3D11ShaderResourceView* a_pView );
	void CSSetUnorderedAccessView( UINT a_uiSlot, ID3D11UnorderedAccessView* a_pView );
	void CSSetSampler( UINT a_uiSlot, ID3D11SamplerState* a_pSampler );

	// Clears the compute shader resources and UAVs, so the next draw can read what the dispatch wrote
	void UnbindComputeResources();

	void Dispatch( UINT a_uiThreadGroupsX, UINT a_uiThreadGroupsY, UINT a_uiThreadGroupsZ );
	void DispatchIndirect( ID3D11Buffer* a_pArgsBuffer, UINT a_uiArgsOffset );

	void ClearUnorderedAccessViewUint( ID3D11UnorderedAccessView* a_pView, UINT a_uiValue );

	const PIPELINESTATE& GetDesiredState() const { return m_oCurrent; }

	// Drops a_pView from every slot, call it before releasing a view that may be set
	void InvalidateShaderResource( ID3D11ShaderResourceView* a_pView );
	void InvalidateUnorderedAccessView( ID3D11UnorderedAccessView* a_pView );

	void ApplyDesiredState();

	// The samplers of the texture stages are set again at the next draw, after code that set its own with PSSetSampler
	void InvalidateSamplerStates() { m_uiDirtySamplers = ( 1u << NUM_TEXTURE_STAGES ) - 1; }

	// The next ApplyDesiredState sends everything, for code that changed the device state behind the renderer
	void InvalidateStateCache() { m_bStateCacheValid = TFALSE; }

	void UpdateRenderStates();

	void ResetStateCache();

	// The capture copies the next frame before it's presented
	void SetPendingCapture( TRenderCaptureD3D* a_pCapture ) { m_pPendingCapture = a_pCapture; }

	// The states a D3D8 state block of CD3DFont records
	struct STATEBLOCK
	{
		DEPTHSTATE                oDepth;
		RASTERIZERSTATE           oRasterizer;
		BLENDSTATE                oBlend;
		TBOOL                     bAlphaTestEnable;
		D3D11_COMPARISON_FUNC     eAlphaFunc;
		TUINT8                    uiAlphaRef;
		TBOOL                     bFogEnable;
		TEXTUREOP                 eTextureOp;
		ID3D11ShaderResourceView* pTexture;
		TEXTUREFILTER             eMinFilter;
		TEXTUREFILTER             eMagFilter;
		TEXTUREFILTER             eMipFilter;
	};

	void CaptureStateBlock( STATEBLOCK& a_rStateBlock ) const;
	void ApplyStateBlock( const STATEBLOCK& a_rStateBlock );

	// The fixed function pipeline of D3D8 for FVF XYZ | DIFFUSE | TEX1 vertices
	void SetFixedFunctionShader();
	void SetShaders( ID3D11VertexShader* a_pVertexShader, ID3D11PixelShader* a_pPixelShader, ID3D11InputLayout* a_pInputLayout );
	void SetTransform( TRANSFORM a_eTransform, const TMatrix44& a_rMatrix );
	const TMatrix44& GetTransform( TRANSFORM a_eTransform ) const { return m_aTransforms[ a_eTransform ]; }
	void                      SetTexture( UINT a_uiStage, ID3D11ShaderResourceView* a_pTexture );
	ID3D11ShaderResourceView* GetTexture( UINT a_uiStage ) const { return m_oCurrent.apPSShaderResources[ a_uiStage ]; }
	void SetTextureOp( TEXTUREOP a_eTextureOp ) { m_eTextureOp = a_eTextureOp; }
	void SetTextureMinFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter );
	void SetTextureMagFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter );
	void SetTextureMipFilter( UINT a_uiStage, TEXTUREFILTER a_eFilter );

	void SetAlphaBlendEnable( TBOOL a_bEnable ) { m_CurrentBlendState.Parts.bBlendEnable = a_bEnable; }
	void SetSrcBlend( D3D11_BLEND a_eBlend );
	void SetDestBlend( D3D11_BLEND a_eBlend );
	void SetBlendOp( D3D11_BLEND_OP a_eBlendOp );
	void SetZEnable( TBOOL a_bEnable ) { m_CurrentDepth.Parts.bDepthEnable = a_bEnable; }
	void SetZWriteEnable( TBOOL a_bEnable ) { m_CurrentDepth.Parts.DepthWriteMask = a_bEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO; }
	void SetZFunc( D3D11_COMPARISON_FUNC a_eFunc ) { m_CurrentDepth.Parts.DepthFunc = a_eFunc; }
	void SetAlphaTestEnable( TBOOL a_bEnable ) { m_bAlphaTestEnable = a_bEnable; }
	void SetAlphaFunc( D3D11_COMPARISON_FUNC a_eFunc ) { m_eAlphaFunc = a_eFunc; }
	void SetAlphaRef( TUINT8 a_uiRef ) { m_uiAlphaRef = a_uiRef; }

	// Enable, reference and function of the alpha test for the AlphaTest function of the shaders
	TVector4 GetAlphaTestParams() const;

	void SetFogEnable( TBOOL a_bEnable ) { m_bFogEnable = a_bEnable; }
	void SetFogColour( TUINT32 a_uiColour ) { m_uiFogColour = a_uiColour; }
	void SetFogStart( TFLOAT a_fStart ) { m_fFogStart = a_fStart; }
	void SetFogEnd( TFLOAT a_fEnd ) { m_fFogEnd = a_fEnd; }

	// Enable, start and end of the linear fog and its colour
	void GetFogParams( TVector4& a_rParams, TVector4& a_rColour ) const;

	void SetStencil( TBOOL a_bEnable, D3D11_COMPARISON_FUNC a_eFunc, UINT a_uiStencilRef, TUINT8 a_uiMask, D3D11_STENCIL_OP a_eFailOp, D3D11_STENCIL_OP a_eDepthFailOp, D3D11_STENCIL_OP a_ePassOp );
	void            SetCullMode( D3D11_CULL_MODE a_eCullMode ) { m_CurrentRasterizer.Flags.Parts.CullMode = a_eCullMode; }
	D3D11_CULL_MODE GetCullMode() const { return D3D11_CULL_MODE( m_CurrentRasterizer.Flags.Parts.CullMode ); }
	void            SetScissorEnable( TBOOL a_bEnable ) { m_CurrentRasterizer.Flags.Parts.bScissorEnable = a_bEnable; }
	void SetDepthBias( INT a_iDepthBias, FLOAT a_fSlopeScaledDepthBias );
	void SetColorUpdate( TBOOL a_bUpdate );
	void SetAlphaUpdate( TBOOL a_bUpdate );

	void SetDstAlpha( TFLOAT a_fAlpha );

public:
	static void FlushShaders();

	static void         PrintError( TINT32 a_eError, const TCHAR* a_szInfo );
	static const TCHAR* GetErrorString( TINT32 a_eError );
	static const TCHAR* GetErrorDescription( TINT32 a_eError );

	static TFORCEINLINE TRenderD3DInterface* Interface()
	{
		return TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	}

private:
	void OnD3DDeviceLost();
	void OnD3DDeviceFound();

	void CreateDeviceObjects();
	void DestroyDeviceObjects();

	TBOOL CreateBackBufferViews();
	void  DestroyBackBufferViews();

	TBOOL CreateDepthTarget( UINT a_uiSampleCount, DEPTHTARGET& a_rTarget );
	void  DestroyDepthTarget( DEPTHTARGET& a_rTarget );

	TBOOL CreateSceneTarget();
	void  DestroySceneTarget();

	void UpdateFixedFunctionConstants();
	void UpdateSamplerStates();
	void UploadConstantBufferSlots( TConstantBufferGeneric** a_ppBuffers, ID3D11Buffer** a_ppCurrent, TUINT32 a_uiSlots );
	void PrepareDraw();

	ID3D11SamplerState* GetSamplerState( TEXTUREFILTER a_eMinFilter, TEXTUREFILTER a_eMagFilter, TEXTUREFILTER a_eMipFilter, ADDRESSINGMODE a_eAddressU, ADDRESSINGMODE a_eAddressV );

private:
	TFLOAT                     m_fPixelAspectRatio;               // Pixel aspect ratio
	HACCEL                     m_AcceleratorTable;                // Accelerator table
	TD3DAdapter::Mode::Device* m_pDevice;                         // Current device (of the adapter database)
	DISPLAYPARAMS              m_oDisplayParams;                  // Display parameters
	TMSWindow                  m_Window;                          // Window
	TBOOL                      m_bExited;                         // Exit flag
	TFLOAT                     m_fContrast;                       // Contrast value
	TFLOAT                     m_fBrightness;                     // Brightness value
	TFLOAT                     m_fGamma;                          // Gamma value
	TFLOAT                     m_fSaturate;                       // Saturation value
	TBOOL                      m_bChangedColourSettings;          // Color settings changed flag
	TBOOL                      m_bCheckedCapableColourCorrection; // Color correction capability checked flag
	TBOOL                      m_bCapableColourCorrection;        // Color correction capability flag
	TBOOL                      m_bEnableColourCorrection;         // Color correction enabled flag
	TUINT16                    m_aGammaRamp[ 3 ][ 256 ];          // Gamma ramp
	TBOOL                      m_bFailed;                         // Failure flag
	t_DeviceLostCallback       m_fnOnDeviceLostCallback;          // Called on device lost
	t_DeviceFoundCallback      m_fnOnDeviceFoundCallback;         // Called on device found
	TPriList<TOrderTable>      m_OrderTables;                     // Order tables

	ID3D11Device*           m_pDevice11;
	ID3D11DeviceContext*    m_pDeviceContext;
	ID3D11DeviceContext1*   m_pDeviceContext1;
	IDXGISwapChain1*        m_pSwapChain;
	ID3D11Texture2D*        m_pBackBuffer;
	TRenderCaptureD3D*      m_pPendingCapture;

	TConstantBufferGeneric* m_apVSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
	TConstantBufferGeneric* m_apPSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
	TUINT32                 m_uiVSConstantBufferSlots; // slots with a buffer
	TUINT32                 m_uiPSConstantBufferSlots;
	TConstantBufferGeneric* m_apCSConstantBuffers[ MAX_CONSTANT_BUFFER_SLOTS ];
	TUINT32                 m_uiCSConstantBufferSlots;

	PIPELINESTATE m_oCurrent; // desired by the callers
	PIPELINESTATE m_oApplied; // held by the device
	TBOOL         m_bStateCacheValid;
	ID3D11RenderTargetView* m_pBackBufferRTView;
	DEPTHTARGET               m_oDepth;
	ID3D11RenderTargetView*   m_pSecondaryRenderTargetOwner;
	ID3D11Texture2D*          m_pSceneTexture;
	ID3D11RenderTargetView*   m_pSceneRTView;
	ID3D11ShaderResourceView* m_pSceneSRV;
	DEPTHTARGET               m_oSceneDepth;
	DXGI_FORMAT               m_eSceneFormat;
	UINT                      m_uiSceneSampleCount;
	TBOOL                     m_bSceneTargetActive;
	TUINT32                 m_uiBackBufferWidth;
	TUINT32                 m_uiBackBufferHeight;
	TBOOL                   m_bOccluded;

	TUINT8*       m_pConstantBufferData;
	TUINT32       m_uiConstantBufferSize;
	TBOOL         m_bConstantBufferChanged;
	ID3D11Buffer* m_aConstantBuffers[ NUMCONSTANTBUFFERS ];
	TUINT32       m_uiConstantBufferIndex;

	ID3D11Buffer* m_pImmediateVertexBuffer;
	TUINT32       m_uiImmediateVertexCurrentOffset;
	ID3D11Buffer* m_pImmediateIndexBuffer;
	TUINT32       m_uiImmediateIndexCurrentOffset;

	ID3D11SamplerState* m_apSamplerStates[ TEXTUREFILTER_NUMOF ][ TEXTUREFILTER_NUMOF ][ TEXTUREFILTER_NUMOF ][ ADDRESSINGMODE_NUMOF ][ ADDRESSINGMODE_NUMOF ];
	ADDRESSINGMODE      m_aAddressU[ NUM_TEXTURE_STAGES ];
	ADDRESSINGMODE      m_aAddressV[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER       m_aMinFilters[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER       m_aMagFilters[ NUM_TEXTURE_STAGES ];
	TEXTUREFILTER       m_aMipFilters[ NUM_TEXTURE_STAGES ];
	TUINT32             m_uiDirtySamplers;

	ID3D11VertexShader*       m_pFixedFunctionVS;
	ID3D11PixelShader*        m_pFixedFunctionPS;
	ID3D11InputLayout*        m_pFixedFunctionLayout;
	ID3D11InputLayout*        m_pPositionColourLayout; // D3DVSDE_POSITION, D3DVSDE_DIFFUSE
	ID3D11Buffer*             m_pZeroTexCoordBuffer;   // the texture coordinates of vertices that have none
	TBOOL                     m_bFixedFunction;
	TMatrix44                 m_aTransforms[ TRANSFORM_NUMOF ];
	TEXTUREOP                 m_eTextureOp;
	TBOOL                     m_bAlphaTestEnable;
	D3D11_COMPARISON_FUNC     m_eAlphaFunc;
	TUINT8                    m_uiAlphaRef;
	TBOOL                     m_bFogEnable;
	TUINT32                   m_uiFogColour;
	TFLOAT                    m_fFogStart;
	TFLOAT                    m_fFogEnd;

	DEPTHSTATE         m_CurrentDepth;
	UINT               m_uiCurrentStencilRef;
	DEPTHSTATE         m_PreviousDepth;
	UINT               m_uiPreviousStencilRef;
	DepthStateMap      m_DepthStates;
	RASTERIZERSTATE    m_CurrentRasterizer;
	RASTERIZERSTATE    m_PreviousRasterizer;
	RasterizerStateMap m_RasterizerStates;
	BLENDSTATE         m_CurrentBlendState;
	FLOAT              m_aCurrentBlendFactor[ 4 ];
	BLENDSTATE         m_PreviousBlendState;
	FLOAT              m_aPreviousBlendFactor[ 4 ];
	BlendStateMap      m_BlendStates;
};

TSINGLETON_DECLARE_INHERITED_ALIAS( TRenderInterface, TRenderD3DInterface, RenderD3D );

TOSHI_NAMESPACE_END
