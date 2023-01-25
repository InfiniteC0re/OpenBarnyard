#pragma once
#include "Toshi/Input/TInputInterface.h"

namespace Toshi
{
	class TInputDXInterface :
		public TGenericClassDerived<TInputDXInterface, TInputInterface, "TInputDXInterface", TMAKEVERSION(1, 0), false>
	{
	public:
		static TInputDXInterface* GetInterface()
		{
			return static_cast<TInputDXInterface*>(TInputInterface::GetSingletonWeak());
		}
	};
}