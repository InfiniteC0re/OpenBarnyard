#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{

	class TInputInterface;

	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), false>,
		public TNodeList<TInputDevice>::TNode
	{
	protected:

		struct DoodadProperties
		{
			int m_iUnk;
			int m_iUnk2;
		};

		struct RepeatInfo
		{
			int m_iDoodad;
		};

		int ProcessRepeats(TGenericEmitter& emitter, float flt);

	public:
		virtual bool Flush() { return true; }
		virtual bool StartRepeat(int param_1, float param_2, float param_3);
		virtual bool StopRepeat(int param_1);
		virtual bool StopAllRepeats();
		virtual void ThrowRepeatEvent(TGenericEmitter& emitter, RepeatInfo* repeatInfo, float flt);
		virtual bool IsForceFeedbackDevice() { return false; }

		bool IsAquired() const
		{
			return m_bIsAquired;
		}

		TInputInterface* GetInputInterface()
		{
			return m_pInterface;
		}

		void SetInputInterface(TInputInterface* a_pInterface)
		{
			m_pInterface = a_pInterface;
		}

	public:
		

		TInputDevice() : TNodeList<TInputDevice>::TNode()
		{
			m_pInterface = TNULL;
			m_bIsAquired = false;
		}

		TInputInterface* m_pInterface;
		bool m_bIsAquired;              // 0x39 de blob 0x35 JPOG
	};
}