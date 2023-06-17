#include "ToshiPCH.h"
#include "TInputDevice.h"

namespace Toshi
{
    int TInputDevice::ProcessRepeats(TGenericEmitter& emitter, float flt)
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

    void TInputDevice::ThrowRepeatEvent(TEmitter<TInputInterface, InputEvent>& emitter, RepeatInfo* repeatInfo, float flt)
    {
        emitter.Throw(InputEvent(this, repeatInfo->m_iDoodad, InputEvent::EventType::EventType_Repeat));
    }
}