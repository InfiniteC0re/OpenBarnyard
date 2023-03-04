#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{
	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), false>,
		public TNodeList<TInputDevice>::TNode
	{
	protected:
		struct RepeatInfo
		{
			int m_iDoodad;
		};

	public:
		virtual bool Flush() { return true; }
		virtual bool StartRepeat(int param_1, float param_2, float param_3);
		virtual bool StopRepeat(int param_1);
		virtual bool StopAllRepeats();
		virtual void ThrowRepeatEvent(TGenericEmitter& emitter, RepeatInfo* repeatInfo, float flt);

	public:
		struct DoodadProperties
		{
			int m_iUnk;
			int m_iUnk2;
		};

		TInputDevice()
		{

		}
	};
}