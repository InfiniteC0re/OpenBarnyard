#pragma once
#include "Memory/AMemory.h"

#include <Toshi/Input/TInputInterface.h>

class AInputHelperSimple
{
public:
	AInputHelperSimple(size_t arraySize, Toshi::TInputDevice* pInputDevice)
	{
		Initialise(arraySize, pInputDevice);
	}

	~AInputHelperSimple()
	{
		delete m_pArray;
	}

	void Initialise(size_t a_iDoodadCount, Toshi::TInputDevice* pInputDevice)
	{
		TASSERT(0 != a_iDoodadCount, "ERROR: Initialising with zero array size");
		m_iArrayIndex = 0;
		m_iArraySize = a_iDoodadCount;
		m_pInputDevice = pInputDevice;
		
		m_pArray = new (AMemory::ms_apMemoryBlocks[0]) Toshi::TInputDevice::DoodadProperties[m_iArraySize];
		Toshi::TUtil::MemClear(m_pArray, m_iArraySize * sizeof(Toshi::TInputDevice::DoodadProperties));
	}

	void AddDoodad(size_t doodad);

	void Update();

private:
	Toshi::TInputDevice::DoodadProperties* m_pArray; // 0x0
	size_t m_iArrayIndex;                            // 0x4
	size_t m_iArraySize;                             // 0x8
	Toshi::TInputDevice* m_pInputDevice;             // 0xC
};