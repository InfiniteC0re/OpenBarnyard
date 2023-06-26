#include "pch.h"
#include "AInputHelperSimple.h"
#include "Toshi/Input/TInputDeviceKeyboard.h"

using namespace Toshi;

void AInputHelperSimple::AddDoodad(size_t doodad)
{
	TASSERT(0 != m_iArraySize);
	TASSERT(m_iArrayIndex < m_iArraySize);

	m_pArray[m_iArrayIndex].m_iUnk = doodad;
	m_iArrayIndex++;
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
