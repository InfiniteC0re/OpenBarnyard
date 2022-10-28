#pragma once
#include "Toshi/Core/TRefCounted.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TRenderInterface;

	class TRenderContext
	{
		typedef uint8_t FLAG;
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
		TRenderContext() {}
		TRenderContext(TRenderInterface&) {}

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

	class TRenderInterface :
		public TGenericClassDerived<TRenderInterface, TObject, "TRenderInterface", TMAKEVERSION(1, 0), false>,
		public TRefCounted,
		public TSingleton<TRenderInterface>
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
		//TRenderInterface() {}

		bool CreateDisplay();
		TRenderContext* CreateRenderContext();

		TRenderContext* SetCurrentRenderContext(TRenderContext* a_pRenderContext) { m_pRenderContext = a_pRenderContext; return m_pRenderContext; }
		TRenderContext* GetCurrentRenderContext() { return m_pRenderContext; }

		virtual bool Create(); //{ return true; }
		virtual void DumpStats();
		
		bool IsInScene() { return m_bInScene; }
		bool IsCreated() { return m_bCreated; }
		bool IsDisplayCreated() { return m_bDisplayCreated; }
	};
}


