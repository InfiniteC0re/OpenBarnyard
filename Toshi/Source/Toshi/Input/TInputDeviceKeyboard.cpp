#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

using namespace Toshi;

int TInputDeviceKeyboard::ProcessVirtualButtons(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
{
	emitter.Throw(TInputInterface::InputEvent(this, 0x20001, TInputInterface::InputEvent::EventType_Unk3, m_fMag1, m_fMag2));
	TIMPLEMENT();
	return 0;
}

void Toshi::TInputDeviceKeyboard::SetID(int a_iMapID, float x, float y, float z, float w)
{
	TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
	m_iMapArray[a_iMapID] = { x, y, z, w };
}
