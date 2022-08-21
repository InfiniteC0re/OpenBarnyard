#include "pch.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TCString.h"
#include "Toshi/Core/TMemory.h"
#include "Toshi/Core/TDebugConsole.h"

namespace Toshi
{
	TApplication::TApplication()
	{
		m_Unk1 = 0;
		m_Flags = 0;

		m_DebugConsole = TDebugConsoleFactory::Create();

		// at the moment it's useless since the console is always opened
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

	bool TApplication::Create()
	{
		// todo..
		// TApplication.dll: 0x10001260

		// temporary solution
		OnCreate();
		OnUpdate();

		return true;
	}

	void TApplication::Destroy()
	{
		m_Flags |= TApplicationFlag_Destroyed;
		OnDestroy();
	}

	bool TApplication::OnCreate()
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