#pragma once

#include "Toshi/Core/TRefCounted.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Render/TRenderContext.h"


namespace Toshi
{

	class TRenderInterface :
		public TGenericClassDerived<TRenderInterface, TObject, "TRenderInterface", TMAKEVERSION(1, 0), false>,
		TRefCounted,
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
		TRenderContextRev* CreateRenderContext();
	};
}


