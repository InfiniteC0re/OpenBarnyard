#pragma once
#include "Toshi/Render/TRenderInterface.h"
#include "Toshi/Render/TOrderTable.h"
#include "TRenderAdapter_DX8.h"
#include "TDebugText_DX8.h"

#include "Platform/Windows/TMSWindow.h"

#include <d3d8.h>

namespace Toshi {

	class TRenderD3DInterface :
		public TGenericClassDerived<TRenderD3DInterface, TRenderInterface, "TRenderD3DInterface", TMAKEVERSION(1, 0), TTRUE>
	{
	public:
		TRenderD3DInterface();
		~TRenderD3DInterface();

		virtual TBOOL CreateDisplay(const DISPLAYPARAMS& a_rParams) override;
		virtual TBOOL DestroyDisplay() override;
		virtual TBOOL Update(float a_fDeltaTime) override;
		virtual TBOOL BeginScene() override;
		virtual TBOOL EndScene() override;
		virtual TRenderAdapter::Mode::Device* GetCurrentDevice() override;
		virtual DISPLAYPARAMS* GetCurrentDisplayParams() override;
		virtual TBOOL Create() override;
		virtual TBOOL Destroy() override;
		virtual void RenderIndexPrimitive(int param_2, int param_3, int param_4, int param_5, int param_6, int param_7) override;
		virtual float GetPixelAspectRatio() override;
		virtual TBOOL SetPixelAspectRatio(float a_fPixelAspectRatio) override;
		virtual void FlushOrderTables() override;
		virtual TBOOL IsTextureFormatSupported(int a_eTextureFormat) override;
		virtual TBOOL Supports32BitTextures() override;
		virtual TRenderContext* CreateRenderContext() override;
		virtual TRenderCapture* CreateCapture() override;
		virtual void DestroyCapture(TRenderCapture* a_pRenderCapture) override;
		virtual void* CreateUnknown(const char* a_szName, int a_iUnk1, int a_iUnk2, int a_iUnk3) override;
		virtual TModel* CreateModel1(void* a_Unk1, void* a_Unk2) override;
		virtual TModel* CreateModel2(void* a_Unk1, void* a_Unk2) override;
		virtual TModel* CreateModel3(void* a_Unk1, void* a_Unk2, void* a_Unk3, void* a_Unk4) override;
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

		IDirect3D8* GetDirect3D() const { return m_pDirect3D; }
		IDirect3DDevice8* GetDirect3DDevice() const { return m_pDirectDevice; }

		TBOOL IsTextureFormatSupportedImpl(D3DFORMAT a_eFormat);

		TBOOL Create(const char* a_szWindowName);
		void BuildAdapterDatabase();
		void DestroyAccelTable();
		void CreateAccelTable();
		void OnD3DDeviceLost();
		void OnD3DDeviceFound();
		void GetCurrentColourRamp();
		TDebugD3DText* InitDebugText(TINT a_iBufferSize);
		void SetDeviceDefaultStates();

		void BeginEndScene();

		void Exit()
		{
			m_bExited = TTRUE;
		}

		static void PrintError(TINT32 a_eError, const char* a_szInfo);
		static const char* GetErrorString(TINT32 a_eError);
		static const char* GetErrorDescription(TINT32 a_eError);

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