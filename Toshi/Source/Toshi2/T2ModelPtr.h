#pragma once
#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{
	class T2ModelPtr
	{
	public:
		T2ModelPtr(int ID) : m_ID(ID)
		{
			T2ResourceManager::GetSingleton()->IncRefCount(m_ID);
		}

		~T2ModelPtr()
		{
			T2ResourceManager::GetSingleton()->DecRefCount(m_ID);
		}

		void operator=(T2ModelPtr other)
		{
			T2ResourceManager::GetSingleton()->DecRefCount(m_ID);
			m_ID = other.m_ID;
			T2ResourceManager::GetSingleton()->IncRefCount(m_ID);
		}

	private:
		int m_ID;
	};
}