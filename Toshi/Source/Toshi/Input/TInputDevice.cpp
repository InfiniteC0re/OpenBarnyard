#include "ToshiPCH.h"
#include "TInputDevice.h"

bool Toshi::TInputDevice::StartRepeat(int param_1, float param_2, float param_3)
{
    return false;
}

bool Toshi::TInputDevice::StopRepeat(int param_1)
{
    return false;
}

bool Toshi::TInputDevice::StopAllRepeats()
{
    return false;
}

void Toshi::TInputDevice::ThrowRepeatEvent(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, RepeatInfo* repeatInfo, float flt)
{
    auto inputEvent = TInputInterface::InputEvent(this, repeatInfo->m_iDoodad, TInputInterface::InputEvent::EventType::EventType_Repeat);
    emitter.Throw(&inputEvent);
}
