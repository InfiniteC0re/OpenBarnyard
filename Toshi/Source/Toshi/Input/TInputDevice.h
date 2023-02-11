#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{
	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), false>,
		public TNodeList<TInputDevice>::TNode
	{

		virtual bool Flush() { return true; }
		virtual bool StartRepeat(int param_1, float param_2, float param_3);

	public:
		TInputDevice()
		{

		}
	};
}