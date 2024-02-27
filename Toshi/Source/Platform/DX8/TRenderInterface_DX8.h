#pragma once
#include "Render/TRenderInterface.h"
#include "Render/TOrderTable.h"
#include "TRenderAdapter_DX8.h"
#include "TDebugText_DX8.h"

#include "Platform/Windows/TMSWindow.h"

#include <d3d8.h>

namespace Toshi {

	class TRenderD3DInterface : public TRenderInterface
	{
	public:
		TDECLARE_CLASS(TRenderInterface);

		friend TMSWindow;

	public:
		TRenderD3DInterface();
		~TRenderD3DInterface();

		//-----------------------------------------------------------------------------
		// TRenderInterface
		//-----------------------------------------------------------------------------
		virtual TBOOL CreateDisplay(const DISPLAYPARAMS& a_rParams) override;
		virtual TBOOL DestroyDisplay() override;
		virtual TBOOL Update(float a_fDeltaTime) override;
		virtual TBOOL BeginScene() override;
		virtual TBOOL EndScene() override;
		virtual TRenderAdapter::Mode::Device* GetCurrentDevice() override;
		virtual DISPLAYPARAMS* GetCurrentDisplayParams() override;
		virtual TBOOL Create() override;
		virtual TBOOL Destroy() override;
		virtual void RenderIndexPrimitive(TINT param_2, TINT param_3, TINT param_4, TINT param_5, TINT param_6, TINT param_7) override;
		virtual float GetPixelAspectRatio() override;
		virtual TBOOL SetPixelAspectRatio(float a_fPixelAspectRatio) override;
		virtual void FlushOrderTables() override;
		virtual TBOOL IsTextureFormatSupported(TINT a_eTextureFormat) override;
		virtual TBOOL Supports32BitTextures() override;
		virtual TRenderContext* CreateRenderContext() override;
		virtual TRenderCapture* CreateCapture() override;
		virtual void DestroyCapture(TRenderCapture* a_pRenderCapture) override;
		virtual void* CreateUnknown(const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3) override;
		virtual TModel* CreateModel(TTMD* a_pTMD, TBOOL a_bLoad) override;
		virtual TModel* CreateModel(const TCHAR* a_szFilePath, TBOOL a_bLoad) override;
		virtual TModel* CreateModel(const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen) override;
		virtual void OnInitializationFailureDevice() override;
		virtual void OnInitializationFailureDisplay() override;
		virtual TDebugText* CreateDebugText() override;
		virtual void DestroyDebugText() override;
		virtual TBOOL RecreateDisplay(const DISPLAYPARAMS& a_rDisplayParams);
		virtual void SetContrast(TFLOAT a_fConstrast);
		virtual void SetBrightness(TFLOAT a_fBrightness);
		virtual void SetGamma(TFLOAT a_fGamma);
		virtual void SetSaturate(TFLOAT a_fSaturate);
		virtual TFLOAT GetContrast() const;
		virtual TFLOAT GetBrightness() const;
		virtual TFLOAT GetGamma() const;
		virtual TFLOAT GetSaturate() const;
		virtual void UpdateColourSettings();
		virtual TBOOL IsCapableColourCorrection();
		virtual void EnableColourCorrection(TBOOL a_bEnable);
		virtual void ForceEnableColourCorrection(TBOOL a_bEnable);
		virtual TBOOL IsColourCorrection();

		TBOOL IsTextureFormatSupportedImpl(D3DFORMAT a_eFormat);

		TBOOL Create(const TCHAR* a_szWindowName);
		
		void BuildAdapterDatabase();
		
		void DestroyAccelTable();
		void CreateAccelTable();
		
		void GetCurrentColourRamp();
		
		TDebugD3DText* InitDebugText(TINT a_iBufferSize);
		
		void SetDeviceDefaultStates();
		void SetTextureAddress(DWORD a_iStage, TINT a_eType, TINT a_iUnk);

		void ClearRegion(TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TUINT8 a_eClearFlags, TUINT8 a_uiColorR, TUINT8 a_uiColorG, TUINT8 a_uiColorB, TFLOAT a_fZ, TUINT a_uiStencil);

		void Exit() { m_bExited = TTRUE; }

		TMSWindow* GetWindow() { return &m_Window; }
		TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }
		IDirect3D8* GetDirect3D() const { return m_pDirect3D; }
		IDirect3DDevice8* GetDirect3DDevice() const { return m_pDirectDevice; }

	public:
		/* Flushes all order tables and shaders. */
		static void FlushShaders();

		/**
		 * Compiles vertex shader in debug mode or takes compiled buffer to create shader.
		 * @param a_szFileName name of the file containing shader source
		 * @param a_pFunction compiled shader
		 * @param a_pOutVertexShader pointer which will be used to store created shader
		 * @return TTRUE if succeeded
		 */
		static TBOOL CreateVertexShader(const char* a_szFileName, const DWORD* a_pFunction, DWORD* a_pOutVertexShader);

		/* Prints error text caused by some directx call. */
		static void PrintError(TINT32 a_eError, const TCHAR* a_szInfo);
		
		static const TCHAR* GetErrorString(TINT32 a_eError);
		static const TCHAR* GetErrorDescription(TINT32 a_eError);

		static TRenderD3DInterface* Interface()
		{
			return TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
		}

	private:
		void OnD3DDeviceLost();
		void OnD3DDeviceFound();

	private:
		IDirect3D8* m_pDirect3D;
		IDirect3DDevice8* m_pDirectDevice;
		D3DPRESENT_PARAMETERS m_PresentParams;
		D3DSURFACE_DESC m_SurfaceDesk;
		float m_fPixelAspectRatio;
		HACCEL m_AcceleratorTable;
		TD3DAdapter::Mode::Device* m_pDevice;
		DISPLAYPARAMS m_oDisplayParams;
		TMSWindow m_Window;
		TBOOL m_bExited;
		TFLOAT m_fContrast;
		TFLOAT m_fBrightness;
		TFLOAT m_fGamma;
		TFLOAT m_fSaturate;
		TBOOL m_bChangedColourSettings;
		TBOOL m_bCheckedCapableColourCorrection;
		TBOOL m_bCapableColourCorrection;
		TBOOL m_bEnableColourCorrection;
		D3DGAMMARAMP m_GammaRamp;
		TBOOL m_bFailed;
		void* m_Unk1;
		void* m_Unk2;
		TPriList<TOrderTable> m_OrderTables;
	};

}