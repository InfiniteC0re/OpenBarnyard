#pragma once
#include "Toshi/Math/Math.h"
#include "Toshi/Math/TSphere.h"
#include "Toshi/Core/TStack.h"
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Core/TRefCounted.h"
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Render/TRenderAdapter.h"
#include "Toshi/Render/TRenderParamTable.h"
#include "Toshi/Render/TRenderCapture.h"
#include "Toshi/Render/TRenderContext.h"
#include "Toshi/Render/TModel.h"
#include "Toshi/Render/TDebugText.h"

namespace Toshi {

	class TRenderInterface :
		public TGenericClassDerived<TRenderInterface, TObject, "TRender", TMAKEVERSION(1, 0), TFALSE>,
		public TSingleton<TRenderInterface>
	{
	public:
		static constexpr size_t MAXNUMRESOURCES = 6000;
		
		typedef uint8_t SYSRESOURCE;
		enum SYSRESOURCE_ : SYSRESOURCE
		{
			SYSRESOURCE_VFactories,
			SYSRESOURCE_VFSYSVNDUV1,
			SYSRESOURCE_VFSKIN,
			SYSRESOURCE_4,
			SYSRESOURCE_5,
			SYSRESOURCE_6,
			SYSRESOURCE_VFWORLD,
			SYSRESOURCE_8,
			SYSRESOURCE_9,
			SYSRESOURCE_IFactories,
			SYSRESOURCE_IFSYS,
			SYSRESOURCE_12,
			SYSRESOURCE_NUMOF = 28
		};

		typedef uint32_t ASPECT_RATIO;
		enum ASPECT_RATIO_ : ASPECT_RATIO
		{
			ASPECT_RATIO_4_3,
			ASPECT_RATIO_16_9,
		};

		struct DISPLAYPARAMS
		{
			TUINT32 uiWidth;
			TUINT32 uiHeight;
			TUINT32 uiColourDepth;
			TUINT32 eDepthStencilFormat;
			TBOOL bWindowed;
		};

	public:
		TRenderInterface();
		virtual ~TRenderInterface();

		virtual TBOOL CreateDisplay(const DISPLAYPARAMS& a_rParams);
		virtual TBOOL DestroyDisplay() = 0;
		virtual TBOOL Update(float a_fDeltaTime) = 0;
		virtual TBOOL BeginScene();
		virtual TBOOL EndScene() = 0;
		virtual TRenderAdapter::Mode::Device* GetCurrentDevice() = 0;
		virtual DISPLAYPARAMS* GetCurrentDisplayParams() = 0;
		virtual TBOOL Create();
		virtual TBOOL Destroy();
		virtual void RenderIndexPrimitive(int param_2, int param_3, int param_4, int param_5, int param_6, int param_7);
		virtual void DumpStats();
		virtual void GetScreenOffset(TVector2& a_rVec);
		virtual void SetScreenOffset(const TVector2& a_rVec);
		virtual float GetScreenAspectRatio();
		virtual float GetPixelAspectRatio();
		virtual TBOOL SetPixelAspectRatio(float a_fPixelAspectRatio);
		virtual void FlushOrderTables();
		virtual TBOOL IsTextureFormatSupported(int a_eTextureFormat);
		virtual TBOOL Supports32BitTextures();
		virtual TRenderContext* CreateRenderContext() = 0;
		virtual TRenderCapture* CreateCapture() = 0;
		virtual void DestroyCapture(TRenderCapture* a_pRenderCapture) = 0;
		virtual void SetLightDirectionMatrix(const TMatrix44& a_rMatrix);
		virtual void SetLightColourMatrix(const TMatrix44& a_rMatrix);
		virtual void Unknown1(int a_iUnknown);
		virtual void* CreateUnknown(const char* a_szName, int a_iUnk1, int a_iUnk2, int a_iUnk3) = 0;
		virtual TModel* CreateModel1(void* a_Unk1, void* a_Unk2) = 0;
		virtual TModel* CreateModel2(void* a_Unk1, void* a_Unk2) = 0;
		virtual TModel* CreateModel3(void* a_Unk1, void* a_Unk2, void* a_Unk3, void* a_Unk4) = 0;
		virtual void OnInitializationFailureDevice();
		virtual void OnInitializationFailureDisplay();
		virtual TDebugText* CreateDebugText() = 0;
		virtual void DestroyDebugText() = 0;
		virtual TBOOL CreateSystemResources();
		virtual void DestroySystemResources();

		// Creates display and returns TTRUE if success
		TBOOL CreateDisplay();

		// Destroys resource
		void DestroyResource(TResource* resource);
		
		// Destroys resource recursively
		void DestroyResourceRecurse(TResource* resource);

		// Returns system resource
		TResource* GetSystemResource(SYSRESOURCE systemResource)
		{
			TASSERT(systemResource >= 0 && systemResource < SYSRESOURCE_NUMOF, "Unknown resource");
			return m_SystemResources[systemResource];
		}

		// Sets resource explicit
		void SetResourceExplicit(TResource* resource, SYSRESOURCE systemResource)
		{
			TASSERT(systemResource >= 0 && systemResource < SYSRESOURCE_NUMOF, "Unknown resource");
			TASSERT(m_SystemResources[systemResource] == TNULL, "This resource has already been assigned!");
			m_SystemResources[systemResource] = resource;
		}

		// Creates resource and returns it
		TResource* CreateResource(TClass* pClass, char* name, TNodeTree<TResource>::TNode* parent);

		// Sets new render context and returns the old one
		TRenderContext* SetCurrentRenderContext(TRenderContext* a_pRenderContext)
		{
			return std::exchange(m_pCurrentContext, a_pRenderContext);
		}

		TBOOL IsInScene() { return m_bInScene; }
		TBOOL IsCreated() { return m_bCreated; }
		TBOOL IsDisplayCreated() { return m_bDisplayCreated; }
		TRenderContext* GetCurrentRenderContext() const { return m_pCurrentContext; }
		TNodeList<TRenderAdapter>* GetAdapterList() { return &m_AdapterList; }
		TKeyframeLibraryManager& GetKeyframeLibraryManager() { return m_KeyframeManager; }

		void DestroyAllShaderResources();

		void DestroyDyingResources(TResource* resources);
		void DestroyDyingResources();

		void DeleteResource(TResource* resources);
		void DeleteResourceRecurse(TResource* resources);
		void DeleteResourceAtomic(TResource* resources);

		TRenderAdapter::Mode::Device* FindDevice(const DISPLAYPARAMS& a_rDisplayParams);

		void FlushDyingResources()
		{
			if (m_bHasDyingResources)
			{
				DestroyDyingResources();
			}
		}

	protected:
		TUINT32 m_Unk1;                                  // 0x04
		TBOOL m_bCreateSystemResources;                  // 0x08
		TBOOL m_bInScene;                                // 0x09
		TBOOL m_bCreated = TFALSE;                       // 0x0A
		TBOOL m_bDisplayCreated;                         // 0x0B
		TVector2 m_ScreenOffset;                         // 0x0C
		TRenderContext* m_pCurrentContext;               // 0x14
		TRenderContext* m_pCreatedContext;               // 0x18
		TResource* m_SystemResources[SYSRESOURCE_NUMOF]; // 0x1C
		TMatrix44 m_LightDirection;                      // 0x8C
		TMatrix44 m_LightColour;                         // 0xCC
		TDebugText* m_pDebugText;                        // 0x10C
		void* m_Unk2;                                    // 0x110
		void* m_Unk3;                                    // 0x114
		TNodeList<TRenderAdapter> m_AdapterList;         // 0x118
		TNodeTree<TResource> m_Resources;                // 0x128
		TUINT32 m_ResourceCount = 0;                     // 0x140
		TUINT32 m_iFrameCount = 0;                       // 0x144
		TBOOL m_bHasDyingResources;                      // 0x148
		TKeyframeLibraryManager m_KeyframeManager;       // 0x14C
		TGenericEmitter m_BeginSceneEmitter;             // 0x158
		TGenericEmitter m_EndSceneEmitter;               // 0x164
		TStack<TMatrix44, 20> m_Transforms;              // 0x170
	};

}