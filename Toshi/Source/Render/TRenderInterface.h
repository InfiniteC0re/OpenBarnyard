#pragma once
#include "Math/Math.h"
#include "Math/TSphere.h"
#include "Toshi/TStack.h"
#include "Toshi/TNodeList.h"
#include "Toshi/TRefCounted.h"
#include "Render/TResource.h"
#include "Render/TAnimation.h"
#include "Render/TRenderAdapter.h"
#include "Render/TRenderCapture.h"
#include "Render/TRenderContext.h"
#include "Render/TModel.h"
#include "Render/TDebugText.h"

namespace Toshi {
	
	class TViewport;

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
		SYSRESOURCE_TEXTUREFACTORY = 27,
		SYSRESOURCE_NUMOF
	};

	class TRenderInterface :
		public TObject,
		public TSingleton<TRenderInterface>
	{
	public:
		TDECLARE_CLASS(TObject);

	public:
		static constexpr size_t MAXNUMRESOURCES = 6000;

		typedef TUINT32 ASPECT_RATIO;
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
		virtual void RenderIndexPrimitive(TINT param_2, TINT param_3, TINT param_4, TINT param_5, TINT param_6, TINT param_7);
		virtual void DumpStats();
		virtual void GetScreenOffset(TVector2& a_rVec);
		virtual void SetScreenOffset(const TVector2& a_rVec);
		virtual float GetScreenAspectRatio();
		virtual float GetPixelAspectRatio();
		virtual TBOOL SetPixelAspectRatio(float a_fPixelAspectRatio);
		virtual void FlushOrderTables();
		virtual TBOOL IsTextureFormatSupported(TINT a_eTextureFormat);
		virtual TBOOL Supports32BitTextures();
		virtual TRenderContext* CreateRenderContext() = 0;
		virtual TRenderCapture* CreateCapture() = 0;
		virtual void DestroyCapture(TRenderCapture* a_pRenderCapture) = 0;
		virtual void SetLightDirectionMatrix(const TMatrix44& a_rMatrix);
		virtual void SetLightColourMatrix(const TMatrix44& a_rMatrix);
		virtual void Unknown1(TViewport* a_pViewport);
		virtual void* CreateUnknown(const TCHAR* a_szName, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3) = 0;
		virtual TModel* CreateModel(TTMD* a_pTMD, TBOOL a_bLoad) = 0;
		virtual TModel* CreateModel(const TCHAR* a_szFilePath, TBOOL a_bLoad) = 0;
		virtual TModel* CreateModel(const TCHAR* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen) = 0;
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
		template <class T>
		T* GetSystemResource(SYSRESOURCE systemResource)
		{
			TASSERT(systemResource >= 0 && systemResource < SYSRESOURCE_NUMOF, "Unknown resource");
			return TSTATICCAST(T, m_SystemResources[systemResource]);
		}

		// Sets resource explicit
		void SetResourceExplicit(TResource* resource, SYSRESOURCE systemResource);

		// Creates resource and returns it
		TResource* CreateResource(TClass* pClass, const TCHAR* name, TResource* parent);

		// Sets new render context and returns the old one
		TRenderContext* SetCurrentRenderContext(TRenderContext* a_pRenderContext)
		{
			return std::exchange(m_pCurrentContext, a_pRenderContext);
		}

		void SetAutoCreateSystemResources(TBOOL a_bAutoCreate) { m_bCreateSystemResources = a_bAutoCreate; }

		TBOOL IsInScene() { return m_bInScene; }
		TBOOL IsCreated() { return m_bCreated; }
		TBOOL IsDisplayCreated() { return m_bDisplayCreated; }

		const TMatrix44& GetLightDirection() const { return m_LightDirection; }
		const TMatrix44& GetLightColour() const { return m_LightColour; }

		TRenderContext* GetCurrentContext() const { return m_pCurrentContext; }
		TNodeList<TRenderAdapter>* GetAdapterList() { return &m_AdapterList; }
		TKeyframeLibraryManager& GetKeyframeLibraryManager() { return m_KeyframeManager; }
		TStack<TMatrix44, 20>& GetTransforms() { return m_Transforms; }
		TTexture* GetInvalidTexture() { return m_pInvalidTexture; }

		void DestroyAllShaderResources();

		void FlushDyingResources();
		void DestroyDyingResources(TResource* resources);
		//void DestroyDyingResources();

		void DeleteResource(TResource* resources);
		void DeleteResourceRecurse(TResource* resources);
		void DeleteResourceAtomic(TResource* resources);

		TRenderAdapter::Mode::Device* FindDevice(const DISPLAYPARAMS& a_rDisplayParams);

		void BeginEndSceneHAL();

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
		TTexture* m_pInvalidTexture;                     // 0x114
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
