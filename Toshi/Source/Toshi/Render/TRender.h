#pragma once
#include "Toshi/Core/TRefCounted.h"

namespace Toshi
{
	class TRender;

	class TRenderContext
	{
		typedef uint32_t FLAG;
		enum FLAG_ : FLAG
		{
			FLAG_DIRTY = BITFIELD(0),
			FLAG_FOG   = BITFIELD(1)
		};

		struct PROJECTIONPARAMS
		{
			TVector2 m_Centre;  // 0x0
			TVector2 m_Proj;    // 0x8
		};

	protected:
		inline void SetDirty(bool enable) { enable ? m_eFlags |= FLAG_DIRTY : m_eFlags &= ~FLAG_DIRTY; }
		inline void SetFlag(FLAG flag, bool enable) { enable ? m_eFlags |= flag : m_eFlags &= ~flag; }

		inline void EnableFog(bool enable) { enable ? m_eFlags |= FLAG_FOG : m_eFlags &= ~FLAG_FOG; }
		inline bool IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
		inline bool IsDirty() const { return m_eFlags & FLAG_DIRTY; }
	
	public:
		TRenderContext() { m_eFlags = 0; }
		TRenderContext(TRender&) { m_eFlags = 0; }

		virtual void SetModelViewMatrix(const TMatrix44& a_rMatrix)
		{
			m_eFlags |= 0x300;
			m_mModelViewMatrix = TMatrix44(a_rMatrix);
			m_eFlags &= ~0x300;
		}

		virtual void SetWorldViewMatrix(const TMatrix44& a_rMatrix)
		{
			m_eFlags |= 0x100;
			m_mModelViewMatrix = TMatrix44(a_rMatrix);
			m_eFlags &= ~0x100;
		}

	private:
		FLAG m_eFlags;                          // 0x8
		static PROJECTIONPARAMS m_sProjParams;  // 0x30 (it cannot have an offset if it's static)
		TMatrix44 m_mModelViewMatrix;           // 0x40
		TMatrix44 m_mWorldViewMatrix;           // 0x80
	};

	class TRender :
		public TGenericClassDerived<TRender, TObject, "TRender", TMAKEVERSION(1, 0), false>,
		public TRefCounted,
		public TSingleton<TRender>
	{

		static constexpr int SYSRESOURCE_NUMOF = 12;

	protected:
		bool m_bInScene;                  // 0x9
		bool m_bCreated = false;          // 0xA
		bool m_bDisplayCreated;           // 0xB
		bool m_bIsEnabled;                // 0xD
		TRenderContext* m_pRenderContext; // 0x18
		void* m_aSysResources;            // 0x20
		//TNodeList

	public:
		TRender() = default;

		// Creates display and returns true if success
		bool CreateDisplay();

		// Sets new render context and returns the previous one
		TRenderContext* SetCurrentRenderContext(TRenderContext* a_pRenderContext)
		{
			auto previousContext = m_pRenderContext;
			m_pRenderContext = a_pRenderContext;
			return previousContext;
		}

		virtual bool Create();    // 0x28
		virtual void DumpStats(); // 0x30
		
		bool IsInScene() { return m_bInScene; }
		bool IsCreated() { return m_bCreated; }
		bool IsDisplayCreated() { return m_bDisplayCreated; }
		TRenderContext* GetCurrentRenderContext() { return m_pRenderContext; }
	
	public:
		/*
		* Platform specific methods
		* Define them in TRender_{Renderer}.cpp
		*/
		TRenderContext* CreateRenderContext();
	};
}


