#include "ToshiPCH.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TString8.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	TApplication::TApplication()
	{
		m_Flags = 0;
	}

	TApplication::~TApplication()
	{
		Destroy();
	}

	bool TApplication::Create(const char* appName, int argc, char** argv)
	{
		TTODO("TGenericGlobalListener<Toshi::TApplicationExitEvent>::ConnectImpl(&this->m_ExitEventListener,this,OnApplicationExitEvent);");
		m_Name = appName;
		
		m_pDebugConsole = new TDebugConsole;

#ifdef TOSHI_DEBUG
		m_pDebugConsole->Show(true);
#else
		m_pDebugConsole->Show(false);
#endif

		return OnCreate(argc, argv);
	}

	void TApplication::Destroy()
	{
		m_Flags |= TApplicationFlag_Destroyed;
		OnDestroy();
	}

	bool TApplication::Execute()
	{
		TASSERT(TApplication::IsCreated() == TTRUE);
		TSystemManager* pSystemManager = TSystemManager::GetSingleton();
		
		bool updateResult = true;
		while (updateResult && !IsDestroyed())
		{
			pSystemManager->Update();
			updateResult = OnUpdate(pSystemManager->GetTimer()->GetDelta());
		}

		return OnDestroy();
	}

	bool TApplication::OnCreate(int argc, char** argv)
	{
		m_Flags |= TApplicationFlag_Created;
		return true;
	}

	bool TApplication::OnUpdate(float deltaTime)
	{
		return (m_Flags & TApplicationFlag_Destroyed) == 0;
	}

	bool TApplication::OnDestroy()
	{
		return true;
	}
}