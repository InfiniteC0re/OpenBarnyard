#pragma once

#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{

	class T2TexturePtr
	{

		T2TexturePtr& operator=(T2TexturePtr texturePtr)
		{
			auto resourceManager = T2ResourceManager::GetSingletonWeak();
			resourceManager->DecRefCount(m_iID);
			m_iID = texturePtr.m_iID;
			resourceManager->IncRefCount(m_iID);
		}

	public:
		T2TexturePtr()
		{
			m_iID = 0;
		}

		~T2TexturePtr()
		{
			auto resourceManager = T2ResourceManager::GetSingletonWeak();
			resourceManager->DecRefCount(m_iID);
		}

		int m_iID;
	};
}
