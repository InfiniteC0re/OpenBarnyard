#include "ToshiPCH.h"
#include "TInputInterface.h"

namespace Toshi {

	TInputDevice::TInputDevice() :
		m_Repeats(0, 16),
		m_Array2(0, 16)
	{
		m_bUnknown = TFALSE;
		m_bIsAcquired = TFALSE;
		m_pInputInterface = TNULL;
	}

	TBOOL TInputDevice::Flush()
	{
		return TTRUE;
	}

	TINT TInputDevice::ProcessRepeats(EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime)
	{
		TIMPLEMENT();
		return 0;
	}

	TBOOL TInputDevice::StartRepeat(TINT param_1, TFLOAT param_2, TFLOAT param_3)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	void TInputDevice::StopRepeat(TINT param_1)
	{
		TIMPLEMENT();
	}

	void TInputDevice::StopAllRepeats()
	{
		m_Repeats.Clear();
	}

	TBOOL TInputDevice::IsForceFeedbackDevice()
	{
		return TFALSE;
	}

	void TInputDevice::ThrowRepeatEvent(EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT flt)
	{
		emitter.Throw(
			TInputInterface::InputEvent(
				this,
				repeatInfo->m_iDoodad,
				TInputInterface::EVENT_TYPE_REPEAT
			)
		);
	}

}
