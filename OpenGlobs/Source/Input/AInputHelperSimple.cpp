#include "pch.h"
#include "AInputHelperSimple.h"
#include "Toshi/Input/TInputDeviceKeyboard.h"

using namespace Toshi;

void AInputHelperSimple::AddDoodad(size_t doodad)
{
	TASSERT(0 != m_iArraySize);
	TASSERT(m_iArrayIndex < m_iArraySize);

	m_pArray[m_iArrayIndex].m_iDoodad = doodad;
	m_iArrayIndex++;
}

TBOOL AInputHelperSimple::IsHeld(size_t doodad)
{
	for (size_t i = 0; i < m_iArrayIndex; i++)
	{
		if (m_pArray[i].m_iDoodad == doodad)
		{
			return m_pArray[i].m_iState == STATE_HELD;
		}
	}
	return TFALSE;
}

TBOOL AInputHelperSimple::WasPressed(size_t doodad)
{
	for (size_t i = 0; i < m_iArrayIndex; i++)
	{
		if (m_pArray[i].m_iDoodad == doodad)
		{
			return m_pArray[i].m_iState == STATE_PRESSED;
		}
	}
	return TFALSE;
}

void AInputHelperSimple::Update()
{
	TTODO("this->m_pInputDevice->vftable[0x15])() ???");
	if (m_pInputDevice != TNULL && m_pInputDevice->GetClass()->IsA(TGetClass(TInputDeviceKeyboard)))
	{
		for (size_t i = 0; i < m_iArrayIndex; i++)
		{
			switch (m_pArray[i].m_iState)
			{
			case STATE_NONE:
				if (m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
				{
					m_pArray[i].m_iState = STATE_PRESSED;
				}
				break;
			case STATE_PRESSED:
				if (m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
				{
					m_pArray[i].m_iState = STATE_HELD;
				}
				else
				{
					m_pArray[i].m_iState = STATE_RELEASED;
				}
				break;
			case STATE_HELD:
				if (!m_pInputDevice->IsDown(m_pArray[i].m_iDoodad))
				{
					m_pArray[i].m_iState = STATE_RELEASED;
				}
				break;
			case STATE_RELEASED:
				m_pArray[i].m_iState = STATE_NONE;
				break;
			}
		}
	}
}
