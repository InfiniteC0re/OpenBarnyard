#pragma once
#include "Toshi/Math/Math.h"
#include "Toshi/Core/TStack.h"
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Core/TRefCounted.h"
#include "Toshi/Render/TResource.h"
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Render/TRenderAdapter.h"
#include "Toshi/Render/TRenderParamTable.h"

namespace Toshi
{
	class TRender;

	class TRenderContext
	{
	public:
		typedef uint32_t FLAG;
		enum FLAG_ : FLAG
		{
			FLAG_DIRTY = BITFIELD(0),
			FLAG_FOG = BITFIELD(1),
			FLAG_HAS_MODELWORLDMATRIX = BITFIELD(2),
			FLAG_HAS_VIEWWORLDMATRIX = BITFIELD(3),
			FLAG_UNK3 = BITFIELD(4),
			FLAG_UNK4 = BITFIELD(5),
			FLAG_UNK5 = BITFIELD(6),
			FLAG_UNK6 = BITFIELD(7),
			FLAG_HAS_MODELVIEWMATRIX = BITFIELD(8),
			FLAG_DIRTY_VIEWMODELMATRIX = BITFIELD(9),
		};

		typedef uint32_t CameraMode;
		enum CameraMode_ : CameraMode
		{
			CameraMode_Perspective,
			CameraMode_Orthographic,
		};

		struct Params
		{
			float fX;
			float fY;
			float fWidth;
			float fHeight;
			float fMinZ;
			float fMaxZ;
		};

		struct PROJECTIONPARAMS
		{
			TVector2 m_Centre;  // 0x0
			TVector2 m_Proj;    // 0x8
			float m_fNearClip;
			float m_fFarClip;
			float m_fUnk;
		};

	protected:
		void SetDirty(TBOOL enable) { enable ? m_eFlags |= FLAG_DIRTY : m_eFlags &= ~FLAG_DIRTY; }
		void SetFlag(FLAG flag, TBOOL enable) { enable ? m_eFlags |= flag : m_eFlags &= ~flag; }

		void EnableFog(TBOOL enable) { enable ? m_eFlags |= FLAG_FOG : m_eFlags &= ~FLAG_FOG; }
		TBOOL IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
		TBOOL IsDirty() const { return m_eFlags & FLAG_DIRTY; }
	
	public:
		TRenderContext(TRender* pRender);

		virtual void SetModelViewMatrix(const TMatrix44& a_rMatrix);
		virtual void SetWorldViewMatrix(const TMatrix44& a_rMatrix);
		virtual void Update() = 0;

		void SetProjectionParams(const PROJECTIONPARAMS& params);

		void SetParams(const Params& params)
		{
			m_oParams = params;
			m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_UNK5 | FLAG_UNK6))) | FLAG_DIRTY;
		}

		void SetCameraMode(CameraMode cameraMode)
		{
			m_eCameraMode = cameraMode;
			m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_UNK5 | FLAG_UNK6))) | FLAG_DIRTY;
		}

		Params& GetParams()
		{
			return m_oParams;
		}

		float GetX() const
		{
			return m_oParams.fX;
		}

		float GetY() const
		{
			return m_oParams.fY;
		}

		float GetWidth() const
		{
			return m_oParams.fWidth;
		}

		float GetHeight() const
		{
			return m_oParams.fHeight;
		}

		const TMatrix44& GetViewWorldMatrix();
		const TMatrix44& GetModelWorldMatrix();
		const TMatrix44& GetViewModelMatrix();
		TMatrix44& GetModelViewMatrix() { return m_oModelViewMatrix; }
		TMatrix44& GetWorldViewMatrix() { return m_oWorldViewMatrix; }

		const PROJECTIONPARAMS& GetProjectionParams() const
		{
			return m_ProjParams;
		}

	protected:
		TRender* m_pRender;                     // 0x0004
		FLAG m_eFlags;                          // 0x0008
		CameraMode m_eCameraMode;               // 0x0014
		Params m_oParams;                       // 0x0018
		PROJECTIONPARAMS m_ProjParams;          // 0x0030
		TMatrix44 m_oModelViewMatrix;           // 0x0050
		TMatrix44 m_oWorldViewMatrix;           // 0x0090
		TMatrix44 m_oModelWorldMatrix;          // 0x00D0
		TMatrix44 m_oViewWorldMatrix;           // 0x0110
		// ...
		TMatrix44 m_oViewModelMatrix;           // 0x036C
	};

	class TRender :
		public TGenericClassDerived<TRender, TObject, "TRender", TMAKEVERSION(1, 0), TFALSE>,
		public TRefCounted,
		public TSingleton<TRender>
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
			SYSRESOURCE_NUMOF
		};

		typedef uint32_t ASPECT_RATIO;
		enum ASPECT_RATIO_ : ASPECT_RATIO
		{
			ASPECT_RATIO_4_3,
			ASPECT_RATIO_16_9,
		};

		struct DisplayParams
		{
			uint32_t Width;
			uint32_t Height;
			uint32_t Unk3;
			uint32_t Unk4;
			TBOOL Unk5;
			TBOOL IsFullscreen;
			uint32_t MultisampleQualityLevel;
		};

	public:
		TRender();

		virtual ~TRender();                                               // 0x08 at vftable
		virtual TBOOL CreateDisplay(DisplayParams* params) = 0;           // 0x0C at vftable
		virtual TBOOL DestroyDisplay() = 0;                               // 0x10 at vftable
		virtual void Update(float deltaTime);                             // 0x14 at vftable
		virtual void BeginScene();                                        // 0x18 at vftable
		virtual void EndScene();                                          // 0x1C at vftable
		virtual void* GetCurrentDevice() = 0;                             // 0x20 at vftable
		virtual DisplayParams* GetCurrentDisplayParams() = 0;             // 0x24 at vftable
		virtual TBOOL Create();                                           // 0x28 at vftable
		virtual TBOOL Destroy();                                          // 0x2C at vftable
		virtual void DumpStats();                                         // 0x30 at vftable
		virtual void GetScreenOffset(TVector2& a_rVec);                   // 0x34 at vftable
		virtual void SetScreenOffset(const TVector2& a_rVec);             // 0x38 at vftable
		virtual void SetLightDirectionMatrix(const TMatrix44& a_rMatrix); // 0x3C at vftable
		virtual void SetLightColourMatrix(const TMatrix44& a_rMatrix);    // 0x40 at vftable
		virtual TBOOL CreateSystemResources();                            // 0x44 at vftable
		virtual void DestroySystemResources();                            // 0x44 at vftable

		float GetResolutionScalar()
		{
			auto displayParams = GetCurrentDisplayParams();
			
			if (m_eAspectRatio == ASPECT_RATIO_16_9)
				return (displayParams->Height * 16.0f) / (displayParams->Width * 9.0f);
			else
				return (displayParams->Height * 4.0f) / (displayParams->Width * 3.0f);
		}

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
			auto previousContext = m_pRenderContext;
			m_pRenderContext = a_pRenderContext;
			return previousContext;
		}

		TBOOL IsInScene() { return m_bInScene; }
		TBOOL IsCreated() { return m_bCreated; }
		TBOOL IsDisplayCreated() { return m_bDisplayCreated; }
		TRenderContext* GetCurrentRenderContext() const { return m_pRenderContext; }
		TNodeList<TRenderAdapter>* GetAdapterList() { return &m_AdapterList; }
		TRenderParamTable* GetParamTable() const { return m_ParamTable; }
		ASPECT_RATIO GetAspectRatio() const { return m_eAspectRatio; }
		TKeyframeLibraryManager& GetKeyframeLibraryManager() { return m_KeyframeManager; }

		void DestroyDyingResources(TResource* resources);
		void DestroyDyingResources();

		void DeleteResource(TResource* resources);
		void DeleteResourceRecurse(TResource* resources);
		void DeleteResourceAtomic(TResource* resources);

		void FlushDyingResources()
		{
			if (m_HasDyingResources)
			{
				DestroyDyingResources();
			}
		}
	
	public:
		/*
		* Platform specific methods
		* Define them in TRender_{Renderer}.cpp
		*/
		TRenderContext* CreateRenderContext();

	protected:
		uint32_t m_Unk1;                                 // 0x04
		TBOOL m_bIsEnabled;                              // 0x08
		TBOOL m_bInScene;                                // 0x09
		TBOOL m_bCreated = TFALSE;                       // 0x0A
		TBOOL m_bDisplayCreated;                         // 0x0B
		TVector2 m_ScreenOffset;                         // 0x0C
		ASPECT_RATIO m_eAspectRatio;                     // 0x14
		TRenderContext* m_pRenderContext;                // 0x18
		TRenderContext* m_pCreatedRenderContext;         // 0x1C
		TResource* m_SystemResources[SYSRESOURCE_NUMOF]; // 0x20
		TMatrix44 m_LightDirection;                      // 0x50
		TMatrix44 m_LightColour;                         // 0x90
		TRenderParamTable* m_ParamTable;                 // 0xD4
		TNodeList<TRenderAdapter> m_AdapterList;         // 0xD8
		TNodeTree<TResource> m_Resources;                // 0xE8
		size_t m_ResourceCount = 0;                      // 0x100
		uint32_t m_iFrameCount = 0;                      // 0x104
		TBOOL m_HasDyingResources;                       // 0x108
		TKeyframeLibraryManager m_KeyframeManager;       // 0x10C
		TStack<TMatrix44, 20> m_Transforms;              // 0x118
	};
}


