#pragma once
#include "Memory/AMemory.h"

#include <Toshi/Input/TInputInterface.h>

class AInputHelperSimple
{
public:
	AInputHelperSimple(size_t arraySize, Toshi::TInputDevice* pInputDevice)
	{
		m_iArrayIndex = 0;
		Initialise(arraySize, pInputDevice);
	}

	AInputHelperSimple()
	{
		m_pArray = TNULL;
		m_iArrayIndex = 0;
		m_iArraySize = 0;
		m_pInputDevice = TNULL;
	}

	~AInputHelperSimple()
	{
		delete m_pArray;
	}

	void Initialise(size_t a_iDoodadCount, Toshi::TInputDevice* pInputDevice)
	{
		TASSERT(0 != a_iDoodadCount, "ERROR: Initialising with zero array size");
		m_iArraySize = a_iDoodadCount;
		m_pInputDevice = pInputDevice;
		
		m_pArray = new (AMemory::ms_apMemoryBlocks[0]) DoodadInfo[m_iArraySize];
		Toshi::TUtil::MemClear(m_pArray, m_iArraySize * sizeof(DoodadInfo));
	}

	void AddDoodad(size_t doodad);
	TBOOL IsHeld(size_t doodad);
	TBOOL WasPressed(size_t doodad);

	void Update();

private:

	typedef uint32_t STATE;
	enum STATE_ : STATE
	{
		STATE_NONE,
		STATE_PRESSED,
		STATE_HELD,
		STATE_RELEASED
	};

	struct DoodadInfo
	{
		int m_iDoodad;
		STATE m_iState;
	};

	DoodadInfo* m_pArray;                            // 0x0
	size_t m_iArrayIndex;                            // 0x4
	size_t m_iArraySize;                             // 0x8
	Toshi::TInputDevice* m_pInputDevice;             // 0xC
};