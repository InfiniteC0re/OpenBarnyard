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
			FLAG_UNK1 = BITFIELD(2),
			FLAG_UNK2 = BITFIELD(3),
			FLAG_UNK3 = BITFIELD(4),
			FLAG_UNK4 = BITFIELD(5),
			FLAG_UNK5 = BITFIELD(6),
			FLAG_UNK6 = BITFIELD(7),
			FLAG_HASMODELVIEWMATRIX = BITFIELD(8),
			FLAG_HASWORLDVIEWMATRIX = BITFIELD(9),
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
		void SetDirty(bool enable) { enable ? m_eFlags |= FLAG_DIRTY : m_eFlags &= ~FLAG_DIRTY; }
		void SetFlag(FLAG flag, bool enable) { enable ? m_eFlags |= flag : m_eFlags &= ~flag; }

		void EnableFog(bool enable) { enable ? m_eFlags |= FLAG_FOG : m_eFlags &= ~FLAG_FOG; }
		bool IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
		bool IsDirty() const { return m_eFlags & FLAG_DIRTY; }
	
	public:
		TRenderContext(TRender* pRender);

		virtual void SetModelViewMatrix(const TMatrix44& a_rMatrix);
		virtual void SetWorldViewMatrix(const TMatrix44& a_rMatrix);
		
		Params& GetParams()
		{
			return m_oParams;
		}

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

		TMatrix44& GetModelViewMatrix()
		{
			return m_mModelViewMatrix;
		}

		void SetProjectionParams(const PROJECTIONPARAMS& params)
		{
			TTODO("asserts and something more...");
			m_sProjParams = params;
			m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_UNK5 | FLAG_UNK6))) | FLAG_DIRTY;
		}

	protected:
		TRender* m_pRender;                     // 0x04
		FLAG m_eFlags;                          // 0x08
		CameraMode m_eCameraMode;               // 0x14
		Params m_oParams;                       // 0x18
		PROJECTIONPARAMS m_sProjParams;         // 0x30
		TMatrix44 m_mModelViewMatrix;           // 0x40
		TMatrix44 m_mWorldViewMatrix;           // 0x80
	};

	class TRender :
		public TGenericClassDerived<TRender, TObject, "TRender", TMAKEVERSION(1, 0), false>,
		public TRefCounted,
		public TSingleton<TRender>
	{
	public:
		static constexpr size_t MAXNUMRESOURCES = 6000;
		
		typedef uint8_t SYSRESOURCE;
		enum SYSRESOURCE_ : SYSRESOURCE
		{
			SYSRESOURCE_1,
			SYSRESOURCE_2,
			SYSRESOURCE_3,
			SYSRESOURCE_4,
			SYSRESOURCE_5,
			SYSRESOURCE_6,
			SYSRESOURCE_7,
			SYSRESOURCE_8,
			SYSRESOURCE_9,
			SYSRESOURCE_10,
			SYSRESOURCE_11,
			SYSRESOURCE_12,
			SYSRESOURCE_NUMOF
		};

		struct DisplayParams
		{
			uint32_t Width;
			uint32_t Height;
			uint32_t Unk3;
			uint32_t Unk4;
			bool Unk5;
			bool IsFullscreen;
			uint32_t MultisampleQualityLevel;
		};

	public:
		TRender();

		virtual ~TRender();                                    // 0x08 at vftable
		virtual bool CreateDisplay(DisplayParams* params) = 0; // 0x0C at vftable
		virtual bool DestroyDisplay() = 0;                     // 0x10 at vftable
		virtual void Update(float deltaTime);                 // 0x14 at vftable
		virtual void BeginScene();                             // 0x18 at vftable
		virtual void EndScene();                               // 0x1C at vftable
		virtual void* GetCurrentDevice() = 0;                  // 0x20 at vftable
		virtual DisplayParams* GetCurrentDisplayParams() = 0;  // 0x24 at vftable
		virtual bool Create();                                 // 0x28 at vftable
		virtual bool Destroy();                                // 0x2C at vftable
		virtual void DumpStats();                              // 0x30 at vftable
		virtual void GetScreenOffset(TVector2* pOutVec);       // 0x34 at vftable
		virtual void SetScreenOffset(TVector2* pVec);          // 0x38 at vftable
		virtual void SetLightDirectionMatrix(TMatrix44* pMat); // 0x3C at vftable
		virtual void SetLightColourMatrix(TMatrix44* pMat);    // 0x40 at vftable
		virtual bool CreateSystemResources();                  // 0x44 at vftable
		virtual void DestroySystemResources();                 // 0x44 at vftable

		// Creates display and returns true if success
		bool CreateDisplay();

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

		bool IsInScene() { return m_bInScene; }
		bool IsCreated() { return m_bCreated; }
		bool IsDisplayCreated() { return m_bDisplayCreated; }
		TRenderContext* GetCurrentRenderContext() const { return m_pRenderContext; }
		TNodeList<TRenderAdapter>* GetAdapterList() { return &m_AdapterList; }
		TRenderParamTable* GetParamTable() const { return m_ParamTable; }

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
		bool m_bIsEnabled;                               // 0x08
		bool m_bInScene;                                 // 0x09
		bool m_bCreated = false;                         // 0x0A
		bool m_bDisplayCreated;                          // 0x0B
		TVector2 m_ScreenOffset;                         // 0x0C
		uint32_t m_Unk2;                                 // 0x14
		TRenderContext* m_pRenderContext;                // 0x18
		uint32_t m_Unk5;                                 // 0x1C
		TResource* m_SystemResources[SYSRESOURCE_NUMOF]; // 0x20
		TMatrix44 m_LightDirection;                      // 0x50
		TMatrix44 m_LightColour;                         // 0x90
		TRenderParamTable* m_ParamTable;                 // 0xD4
		TNodeList<TRenderAdapter> m_AdapterList;         // 0xD8
		TNodeTree<TResource> m_Resources;                // 0xE8
		size_t m_ResourceCount = 0;                      // 0x100
		uint32_t m_Unk4 = 0;                             // 0x104
		bool m_HasDyingResources;                        // 0x108
		TKeyframeLibraryManager m_KeyframeManager;       // 0x10C
		TStack<TMatrix44, 20> m_Transforms;              // 0x118
	};
}


