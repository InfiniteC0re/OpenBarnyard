#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{
	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), false>,
		public TNodeList<TInputDevice>::TNode
	{
	public:
		TInputDevice()
		{

		}
	};
}