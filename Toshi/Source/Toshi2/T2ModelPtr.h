#pragma once
#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{
	class TModel;

	class T2ModelPtr : public T2ResourcePtr
	{
	public:
		T2ModelPtr(int ID = 0) : T2ResourcePtr(ID) { }

		void operator=(const T2ModelPtr& other)
		{
			T2ResourcePtr::operator=(other);
		}

		TModel* GetData() const
		{
			return TSTATICCAST(TModel*, T2ResourcePtr::GetData());
		}

		TModel* operator->() const
		{
			return GetData();
		}
	};
}