#pragma once
#include "Render/TRenderInterface.h"
#include "Render/TOrderTable.h"
#include "TRenderAdapter_DX8.h"
#include "TDebugText_DX8.h"

#include "Platform/DX8/TMSWindow.h"

#include <d3d8.h>

TOSHI_NAMESPACE_START

/**
 * DirectX 8 implementation of the render interface
 * Handles DirectX 8 specific rendering functionality and device management
 */
class TRenderD3DInterface : public TRenderInterface
{
public:
	TDECLARE_CLASS( TRenderD3DInterface, TRenderInterface );

	friend TMSWindow;

public:
	TRenderD3DInterface();
	~TRenderD3DInterface();

	//-----------------------------------------------------------------------------
	// TRenderInterface
	//-----------------------------------------------------------------------------

	/**
	 * Creates a display with the specified parameters
	 * @param a_rParams Display parameters including resolution, color depth, etc.
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL CreateDisplay( const DISPLAYPARAMS& a_rParams ) OVERRIDE;

	/**
	 * Destroys the current display and releases associated resources
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL DestroyDisplay() OVERRIDE;

	/**
	 * Updates the render interface state
	 * @param a_fDeltaTime Time elapsed since last update
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL Update( float a_fDeltaTime ) OVERRIDE;

	/**
	 * Begins a new rendering scene
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL BeginScene() OVERRIDE;

	/**
	 * Ends the current rendering scene and presents the results
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL EndScene() OVERRIDE;

	/**
	 * Gets the current rendering device
	 * @return Pointer to the current device
	 */
	virtual TRenderAdapter::Mode::Device* GetCurrentDevice() OVERRIDE;

	/**
	 * Gets the current display parameters
	 * @return Pointer to the current display parameters
	 */
	virtual DISPLAYPARAMS* GetCurrentDisplayParams() OVERRIDE;

	/**
	 * Creates the render interface
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL Create() OVERRIDE;

	/**
	 * Destroys the render interface and releases all resources
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL Destroy() OVERRIDE;

	/**
	 * Renders an indexed primitive
	 * @param param_2 First parameter
	 * @param param_3 Second parameter
	 * @param param_4 Third parameter
	 * @param param_5 Fourth parameter
	 * @param param_6 Fifth parameter
	 * @param param_7 Sixth parameter
	 */
	virtual void RenderIndexPrimitive( TINT param_2, TINT param_3, TINT param_4, TINT param_5, TINT param_6, TINT param_7 ) OVERRIDE;

	/**
	 * Gets the pixel aspect ratio
	 * @return The current pixel aspect ratio
	 */
	virtual float GetPixelAspectRatio() OVERRIDE;

	/**
	 * Sets the pixel aspect ratio
	 * @param a_fPixelAspectRatio The new pixel aspect ratio
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL SetPixelAspectRatio( float a_fPixelAspectRatio ) OVERRIDE;

	/**
	 * Flushes all order tables
	 */
	virtual void FlushOrderTables() OVERRIDE;

	/**
	 * Checks if a texture format is supported
	 * @param a_eTextureFormat The texture format to check
	 * @return True if supported, false otherwise
	 */
	virtual TBOOL IsTextureFormatSupported( TINT a_eTextureFormat ) OVERRIDE;

	/**
	 * Checks if 32-bit textures are supported
	 * @return True if supported, false otherwise
	 */
	virtual TBOOL Supports32BitTextures() OVERRIDE;

	/**
	 * Creates a new render context
	 * @return Pointer to the new render context
	 */
	virtual TRenderContext* CreateRenderContext() OVERRIDE;

	/**
	 * Creates a new render capture
	 * @return Pointer to the new render capture
	 */
	virtual TRenderCapture* CreateCapture() OVERRIDE;

	/**
	 * Destroys a render capture
	 * @param a_pRenderCapture The render capture to destroy
	 */
	virtual void DestroyCapture( TRenderCapture* a_pRenderCapture ) OVERRIDE;

	/**
	 * Creates an unknown object
	 * @param a_szName Name of the object
	 * @param a_iUnk1 Unknown parameter 1
	 * @param a_iUnk2 Unknown parameter 2
	 * @param a_iUnk3 Unknown parameter 3
	 * @return Pointer to the created object
	 */
	virtual void* CreateUnknown( const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ) OVERRIDE;

	/**
	 * Creates a model from a TMD file
	 * @param a_pTMD Pointer to the TMD data
	 * @param a_bLoad Whether to load the model immediately
	 * @return Pointer to the created model
	 */
	virtual TModel* CreateModelTMD( TTMD* a_pTMD, TBOOL a_bLoad ) OVERRIDE;

	/**
	 * Creates a model from a file
	 * @param a_szFilePath Path to the model file
	 * @param a_bLoad Whether to load the model immediately
	 * @return Pointer to the created model
	 */
	virtual TModel* CreateModelTMDFile( const TCHAR* a_szFilePath, TBOOL a_bLoad ) OVERRIDE;

	/**
	 * Creates a model from a file with TRB asset
	 * @param a_szFilePath Path to the model file
	 * @param a_bLoad Whether to load the model immediately
	 * @param a_pAssetTRB Pointer to the TRB asset
	 * @param a_ui8FileNameLen Length of the filename
	 * @return Pointer to the created model
	 */
	virtual TModel* CreateModelTRB( const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen ) OVERRIDE;

	/**
	 * Called when device initialization fails
	 */
	virtual void OnInitializationFailureDevice() OVERRIDE;

	/**
	 * Called when display initialization fails
	 */
	virtual void OnInitializationFailureDisplay() OVERRIDE;

	/**
	 * Creates debug text
	 * @return Pointer to the created debug text
	 */
	virtual TDebugText* CreateDebugText() OVERRIDE;

	/**
	 * Destroys debug text
	 */
	virtual void DestroyDebugText() OVERRIDE;

	/**
	 * Recreates the display with new parameters
	 * @param a_rDisplayParams New display parameters
	 * @return True if successful, false otherwise
	 */
	virtual TBOOL RecreateDisplay( const DISPLAYPARAMS& a_rDisplayParams );

	/**
	 * Sets the contrast value
	 * @param a_fConstrast New contrast value (0.0-1.0)
	 */
	virtual void SetContrast( TFLOAT a_fConstrast );

	/**
	 * Sets the brightness value
	 * @param a_fBrightness New brightness value (0.0-1.0)
	 */
	virtual void SetBrightness( TFLOAT a_fBrightness );

	/**
	 * Sets the gamma value
	 * @param a_fGamma New gamma value (0.0-1.0)
	 */
	virtual void SetGamma( TFLOAT a_fGamma );

	/**
	 * Sets the saturation value
	 * @param a_fSaturate New saturation value (0.0-1.0)
	 */
	virtual void SetSaturate( TFLOAT a_fSaturate );

	/**
	 * Gets the current contrast value
	 * @return Current contrast value
	 */
	virtual TFLOAT GetContrast() const;

	/**
	 * Gets the current brightness value
	 * @return Current brightness value
	 */
	virtual TFLOAT GetBrightness() const;

	/**
	 * Gets the current gamma value
	 * @return Current gamma value
	 */
	virtual TFLOAT GetGamma() const;

	/**
	 * Gets the current saturation value
	 * @return Current saturation value
	 */
	virtual TFLOAT GetSaturate() const;

	/**
	 * Updates color correction settings
	 */
	virtual void UpdateColourSettings();

	/**
	 * Checks if color correction is supported
	 * @return True if supported, false otherwise
	 */
	virtual TBOOL IsCapableColourCorrection();

	/**
	 * Enables or disables color correction
	 * @param a_bEnable Whether to enable color correction
	 */
	virtual void EnableColourCorrection( TBOOL a_bEnable );

	/**
	 * Forces color correction on or off
	 * @param a_bEnable Whether to force color correction on
	 */
	virtual void ForceEnableColourCorrection( TBOOL a_bEnable );

	/**
	 * Checks if color correction is enabled
	 * @return True if enabled, false otherwise
	 */
	virtual TBOOL IsColourCorrection();

	/**
	 * Checks if a specific D3D format is supported
	 * @param a_eFormat The D3D format to check
	 * @return True if supported, false otherwise
	 */
	TBOOL IsTextureFormatSupportedImpl( D3DFORMAT a_eFormat );

	/**
	 * Creates the render interface with a specific window name
	 * @param a_szWindowName Name of the window
	 * @return True if successful, false otherwise
	 */
	TBOOL Create( const TCHAR* a_szWindowName );

	/**
	 * Builds the adapter database
	 */
	void BuildAdapterDatabase();

	/**
	 * Destroys the accelerator table
	 */
	void DestroyAccelTable();

	/**
	 * Creates the accelerator table
	 */
	void CreateAccelTable();

	/**
	 * Gets the current color ramp
	 */
	void GetCurrentColourRamp();

	/**
	 * Initializes debug text
	 * @param a_iBufferSize Size of the buffer
	 * @return Pointer to the initialized debug text
	 */
	TDebugD3DText* InitDebugText( TINT a_iBufferSize );

	/**
	 * Sets default device states
	 */
	void SetDeviceDefaultStates();

	/**
	 * Sets texture addressing mode
	 * @param a_iStage Texture stage
	 * @param a_eAddressing Addressing type
	 * @param a_eTextureCoordinate Texture coordinate (U or V)
	 */
	void SetTextureAddress( TINT a_iStage, ADDRESSINGMODE a_eAddressing, TEXCOORD a_eTextureCoordinate = TEXCOORD_UV );

	/**
	 * Clears a region of the screen
	 * @param a_iX X position
	 * @param a_iY Y position
	 * @param a_iWidth Width
	 * @param a_iHeight Height
	 * @param a_eClearFlags Clear flags
	 * @param a_uiColorR Red component
	 * @param a_uiColorG Green component
	 * @param a_uiColorB Blue component
	 * @param a_fZ Z value
	 * @param a_uiStencil Stencil value
	 */
	void ClearRegion( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TUINT8 a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil );

	// $Barnyard: FUNCTION 006c59f0
	/**
	 * Signals the render interface to exit
	 */
	void Exit() { m_bExited = TTRUE; }

	/**
	 * Registers an order table
	 * @param a_pOrderTable The order table to register
	 */
	void RegisterOrderTable( TOrderTable* a_pOrderTable );

	/**
	 * Gets the MS window
	 * @return Pointer to the MS window
	 */
	TFORCEINLINE TMSWindow* GetMSWindow() { return &m_Window; }

	/**
	 * Gets the order tables
	 * @return Reference to the order tables list
	 */
	TFORCEINLINE TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }

	/**
	 * Gets the Direct3D interface
	 * @return Pointer to the Direct3D interface
	 */
	TFORCEINLINE IDirect3D8* GetDirect3D() const { return m_pDirect3D; }

	/**
	 * Gets the Direct3D device
	 * @return Pointer to the Direct3D device
	 */
	TFORCEINLINE IDirect3DDevice8* GetDirect3DDevice() const { return m_pDirectDevice; }

public:
	/**
	 * Flushes all order tables and shaders
	 */
	static void FlushShaders();

	/**
	 * Creates a vertex shader
	 * @param a_ShaderDeclaration Shader declaration
	 * @param a_pFunction Shader function
	 * @param a_pOutVertexShader Output vertex shader handle
	 * @return True if successful, false otherwise
	 */
	static TBOOL CreateVertexShader( const DWORD* a_ShaderDeclaration, const DWORD* a_pFunction, DWORD* a_pOutVertexShader );

	/**
	 * Destroys a vertex shader
	 * @param a_hVertexShader Vertex shader handle to destroy
	 */
	static void DestroyVertexShader( DWORD a_hVertexShader );

	/**
	 * Creates a pixel shader
	 * @param a_pFunction Shader function
	 * @param a_pOutPixelShader Output pixel shader handle
	 * @return True if successful, false otherwise
	 */
	static TBOOL CreatePixelShader( const DWORD* a_pFunction, DWORD* a_pOutPixelShader );

	/**
	 * Destroys a pixel shader
	 * @param a_hPixelShader Pixel shader handle to destroy
	 */
	static void DestroyPixelShader( DWORD a_hPixelShader );

	/**
	 * Prints error text caused by some directx call
	 * @param a_eError Error code
	 * @param a_szInfo Additional error information
	 */
	static void PrintError( TINT32 a_eError, const TCHAR* a_szInfo );

	/**
	 * Gets the error string for an error code
	 * @param a_eError Error code
	 * @return Error string
	 */
	static const TCHAR* GetErrorString( TINT32 a_eError );

	/**
	 * Gets the error description for an error code
	 * @param a_eError Error code
	 * @return Error description
	 */
	static const TCHAR* GetErrorDescription( TINT32 a_eError );

	/**
	 * Gets the render interface singleton
	 * @return Pointer to the render interface singleton
	 */
	static TFORCEINLINE TRenderD3DInterface* Interface()
	{
		return TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
	}

private:
	/**
	 * Called when the D3D device is lost
	 */
	void OnD3DDeviceLost();

	/**
	 * Called when the D3D device is found
	 */
	void OnD3DDeviceFound();

private:
	IDirect3D8*                m_pDirect3D;                       // Direct3D interface
	IDirect3DDevice8*          m_pDirectDevice;                   // Direct3D device
	D3DPRESENT_PARAMETERS      m_PresentParams;                   // Presentation parameters
	D3DSURFACE_DESC            m_SurfaceDesk;                     // Surface description
	float                      m_fPixelAspectRatio;               // Pixel aspect ratio
	HACCEL                     m_AcceleratorTable;                // Accelerator table
	TD3DAdapter::Mode::Device* m_pDevice;                         // Current device
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
	D3DGAMMARAMP               m_GammaRamp;                       // Gamma ramp
	TBOOL                      m_bFailed;                         // Failure flag
	void*                      m_Unk1;                            // Unknown 1
	void*                      m_Unk2;                            // Unknown 2
	TPriList<TOrderTable>      m_OrderTables;                     // Order tables
};

TSINGLETON_DECLARE_INHERITED_ALIAS( TRenderInterface, TRenderD3DInterface, RenderD3D );

TOSHI_NAMESPACE_END
