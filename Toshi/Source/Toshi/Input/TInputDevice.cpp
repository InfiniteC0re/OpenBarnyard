#include "ToshiPCH.h"
#include "TInputInterface.h"

namespace Toshi
{
    int TInputDevice::ProcessRepeats(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
    {
        return 0;
    }

    TBOOL TInputDevice::StartRepeat(int param_1, float param_2, float param_3)
    {
        return TFALSE;
    }

    TBOOL TInputDevice::StopRepeat(int param_1)
    {
        return TFALSE;
    }

    TBOOL TInputDevice::StopAllRepeats()
    {
        return TFALSE;
    }

    void TInputDevice::ThrowRepeatEvent(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, RepeatInfo* repeatInfo, float flt)
    {
        emitter.Throw(TInputInterface::InputEvent(this, repeatInfo->m_iDoodad, TInputInterface::InputEvent::EventType::EventType_Repeat));
    }
}