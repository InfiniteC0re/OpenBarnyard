#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{

	class TInputInterface;

	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), TFALSE>,
		public TNodeList<TInputDevice>::TNode
	{
	protected:
		const int INPUT_DEVICE_MOUSE_BUTTONS = 2;
		const int INPUT_DEVICE_MOUSE_WHEEL = 4;
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
		virtual TBOOL Flush() { return TTRUE; }
		virtual TBOOL StartRepeat(int param_1, float param_2, float param_3);
		virtual TBOOL StopRepeat(int param_1);
		virtual TBOOL StopAllRepeats();
		virtual void ThrowRepeatEvent(TGenericEmitter& emitter, RepeatInfo* repeatInfo, float flt);
		virtual TBOOL IsForceFeedbackDevice() { return TFALSE; }

		TBOOL IsAquired() const
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
			m_bIsAquired = TFALSE;
		}

		TInputInterface* m_pInterface;
		TBOOL m_bIsAquired;              // 0x39 de blob 0x35 JPOG
	};
}