#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

using namespace Toshi;

int TInputDeviceKeyboard::ProcessVirtualButtons(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
{
	emitter.Throw(TInputInterface::InputEvent(this, 0x20001, TInputInterface::InputEvent::EventType_Unk3, m_fMag1, m_fMag2));
	TIMPLEMENT();
	return 0;
}

void Toshi::TInputDeviceKeyboard::SetMap(int a_iMapID, float x, float y, float z, float w)
{
	TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
	m_iMapArray[a_iMapID] = { x, y, z, w };
}

void Toshi::TInputDeviceKeyboard::GetMap(int a_iMapID, float& x, float& y, float& z, float& w)
{
	TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
	x = m_iMapArray[a_iMapID].x;
	y = m_iMapArray[a_iMapID].y;
	z = m_iMapArray[a_iMapID].z;
	w = m_iMapArray[a_iMapID].w;
}
