#include "ToshiPCH.h"
#include "TSystem.h"

namespace Toshi
{
	TSystemManager::TSystemManager() : m_Emitter(m_Emitter)
	{
		this->m_Unk1 = 0;
		this->m_PauseManagers = false;
		this->m_Unk4 = 0;
		this->m_Unk5 = 0;
		this->m_Unk6 = 0;
		this->m_Unk7 = false;
	}

	bool TSystemManager::Create()
	{
		new TSystemManager();
		return true;
	}
}