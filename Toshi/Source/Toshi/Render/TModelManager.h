#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi2/T2ModelPtr.h"

namespace Toshi
{
	class TModelManager : public TSingleton<TModelManager>
	{
	public:
		TModelManager()
		{
			// 006aefe0
			m_Unk = TNULL;
			T2ResourceManager::GetSingleton()->IncRefCount(0);
		}

		void AddModel(T2ModelPtr model)
		{
			TIMPLEMENT();
		}

		void FindModel(const char* name)
		{
			TIMPLEMENT();
		}

		void GetFirstModel()
		{
			TIMPLEMENT();
		}

		void GetNextModel(T2ModelPtr model)
		{
			TIMPLEMENT();
		}

		void RemoveModel(T2ModelPtr model)
		{
			TIMPLEMENT();
		}

	private:
		void* m_Unk;
	};
}