#pragma once
#include "Toshi/Core/TRefCounted.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Math/Math.h"

namespace Toshi
{
	class TRenderInterface;

	class TRenderContext
	{
		typedef uint8_t FLAG;
		enum FLAG_ : FLAG
		{
			FLAG_DIRTY = 1,
			FLAG_FOG
		};

		struct PROJECTIONPARAMS
		{
			float m_fCentreX;	// 0x0
			float m_fCentreY;	// 0x0
			float m_fProjX;		// 0x8
			float m_fProjY;		// 0xC
		};

		uint8_t m_eFlags;						// 0x8
		static PROJECTIONPARAMS m_sProjParams;	// 0x30
		TMatrix44 m_mModelViewMatrix;			// 0x40
		TMatrix44 m_mWorldViewMatrix;			// 0x80

	protected:
		inline void SetDirty(bool enable) { enable ? m_eFlags |= FLAG_DIRTY : m_eFlags &= ~FLAG_DIRTY; }
		inline void SetFlag(FLAG flag, bool enable) { enable ? m_eFlags |= flag : m_eFlags &= ~flag; }

		inline void EnableFog(bool enable) { enable ? m_eFlags |= FLAG_FOG : m_eFlags &= ~FLAG_FOG; }

		inline bool IsFogEnabled() const { return (m_eFlags & FLAG_FOG) != 0; }
		inline bool IsDirty() const { return (m_eFlags & FLAG_DIRTY) != 0; }
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
	};

	class TRenderInterface :
		public TGenericClassDerived<TRenderInterface, TObject, "TRenderInterface", TMAKEVERSION(1, 0), false>,
		public TRefCounted,
		public TSingleton<TRenderInterface>
	{

		const int SYSRESOURCE_NUMOF = 12;

	protected:
		bool m_bInScene;		// 0x9
		bool m_bCreated = false;		// 0xA
		bool m_bDisplayCreated;	// 0xB
		bool m_bIsEnabled;		// 0xD
		TRenderContext* m_pRenderContext; // 0x18
		void* m_aSysResources;		// 0x20
		//TNodeList

	public:

		//TRenderInterface() {}

		inline TRenderContext* SetCurrentRenderContext(TRenderContext* a_pRenderContext) { m_pRenderContext = a_pRenderContext; return m_pRenderContext; }
		inline TRenderContext* GetCurrentRenderContext() { return m_pRenderContext; }

		virtual bool Create(); //{ return true; }
		bool CreateDisplay();
		inline bool IsInScene() { return m_bInScene; }
		inline bool IsCreated() { return m_bCreated; }
		inline bool IsDisplayCreated() { return m_bDisplayCreated; }
		virtual void DumpStats();

		TRenderContext* CreateRenderContext();
	};
}


