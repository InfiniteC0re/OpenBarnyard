#include "pch.h"
#include "AInputHelperSimple.h"
#include "Toshi/Input/TInputDeviceKeyboard.h"

using namespace Toshi;

void AInputHelperSimple::AddDoodad(size_t doodad)
{
	TASSERT(0 != m_iArraySize);
	TASSERT(m_iArrayIndex < m_iArraySize);

	m_pArray[m_iArrayIndex * 2] = doodad;
	m_iArrayIndex += 1;
}

void AInputHelperSimple::Update()
{
	if (m_pInputDevice != TNULL)
	{
		if (m_pInputDevice->GetClass()->IsA(TGetClass(TInputDeviceKeyboard)))
		{
			TIMPLEMENT();
		}
	}
}
