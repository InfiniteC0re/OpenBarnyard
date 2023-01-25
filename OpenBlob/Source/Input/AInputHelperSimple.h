#pragma once
#include "Toshi/Input/TInputDevice.h"
#include "Memory/AMemory.h"

class AInputHelperSimple
{
public:
	AInputHelperSimple(size_t arraySize, Toshi::TInputDevice* pInputDevice)
	{
		Initialise(arraySize, pInputDevice);
	}

	~AInputHelperSimple()
	{
		TFree(m_pArray);
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
	uint32_t* m_pArray;
	size_t m_iArrayIndex;
	size_t m_iArraySize;
	Toshi::TInputDevice* m_pInputDevice;
};