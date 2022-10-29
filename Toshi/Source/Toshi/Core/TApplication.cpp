#include "ToshiPCH.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TString8.h"
#include "Toshi/Core/TDebugConsole.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	TApplication::TApplication()
	{
		m_Flags = 0;
		
		m_DebugConsole = TDebugConsole::Create();

#ifdef TOSHI_DEBUG
		m_DebugConsole->Show(true);
#else
		m_DebugConsole->Show(false);
#endif
	}

	TApplication::~TApplication()
	{
		Destroy();
	}

	bool TApplication::Create(const char* appName, int argc, char** argv)
	{
		TTODO("TGenericGlobalListener<Toshi::TApplicationExitEvent>::ConnectImpl(&this->m_ExitEventListener,this,OnApplicationExitEvent);");
		m_Name = appName;
		
		return OnCreate(argc, argv);
	}

	void TApplication::Destroy()
	{
		m_Flags |= TApplicationFlag_Destroyed;
		OnDestroy();
	}

	bool TApplication::OnCreate(int argc, char** argv)
	{
		m_Flags |= TApplicationFlag_Created;
		return true;
	}

	bool TApplication::OnUpdate()
	{
		return (m_Flags & TApplicationFlag_Destroyed) == 0;
	}

	bool TApplication::OnDestroy()
	{
		return true;
	}
}