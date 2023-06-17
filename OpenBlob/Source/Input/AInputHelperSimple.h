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

	void Initialise(size_t doodadCount, Toshi::TInputDevice* pInputDevice)
	{
		m_iArrayIndex = 0;
		m_iArraySize = doodadCount;
		m_pInputDevice = pInputDevice;
		TASSERT(m_iArraySize != 0, "ERROR: Initialising with zero array size");

		m_pArray = (uint32_t*)AMemory::ms_apMemoryBlocks[0]->Malloc(m_iArraySize * 8);
		Toshi::TUtil::MemClear(m_pArray, sizeof(m_iArraySize * 8));
	}

	void AddDoodad(size_t doodad);

	void Update();

private:
	uint32_t* m_pArray;                   // 0x0
	size_t m_iArrayIndex;                 // 0x4
	size_t m_iArraySize;                  // 0x8
	Toshi::TInputDevice* m_pInputDevice;  // 0xC
};