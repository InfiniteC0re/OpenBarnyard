#include "ToshiPCH.h"
#include "TInputInterface.h"

namespace Toshi
{
	int TInputDevice::ProcessRepeats(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
	{
		TIMPLEMENT();
		return 0;
	}

	TBOOL TInputDevice::StartRepeat(int param_1, float param_2, float param_3)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TInputDevice::StopRepeat(int param_1)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TBOOL TInputDevice::StopAllRepeats()
	{
		TIMPLEMENT();
		return TFALSE;
	}

	void TInputDevice::ThrowRepeatEvent(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, RepeatInfo* repeatInfo, float flt)
	{
		emitter.Throw(TInputInterface::InputEvent(this, repeatInfo->m_iDoodad, TInputInterface::EVENT_TYPE_REPEAT));
	}
}