#include "ToshiPCH.h"
#include "TInputDeviceKeyboard.h"

using namespace Toshi;

int TInputDeviceKeyboard::ProcessVirtualButtons(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt)
{
	emitter.Throw(TInputInterface::InputEvent(this, 0x20001, TInputInterface::InputEvent::EventType_Unk3, m_mags[0], m_mags[1]));
	TIMPLEMENT();
	return 0;
}

void TInputDeviceKeyboard::SetVirtStickDoodads(int a_iMapID, int x, int y, int z, int w)
{
	TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
	m_VirtStickDoodads[a_iMapID][0] = x;
	m_VirtStickDoodads[a_iMapID][1] = y;
	m_VirtStickDoodads[a_iMapID][2] = z;
	m_VirtStickDoodads[a_iMapID][3] = w;
}

void TInputDeviceKeyboard::GetVirtStickDoodads(int a_iMapID, int& x, int& y, int& z, int& w)
{
	TASSERT(a_iMapID >= 0 && a_iMapID < VIRTSTK_DIRECTIONAL_MAPS);
	x = m_VirtStickDoodads[a_iMapID][0];
	y = m_VirtStickDoodads[a_iMapID][1];
	z = m_VirtStickDoodads[a_iMapID][2];
	w = m_VirtStickDoodads[a_iMapID][3];
}

void TInputDeviceKeyboard::UpdateVirtualButtons()
{
}
