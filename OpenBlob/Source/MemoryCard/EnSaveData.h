#pragma once

#include "Toshi2/T2String8.h"
#include "Toshi/Utils/TUtil.h"

class EnSaveData
{
public:

	static constexpr int SAVEGAME_SLOT_COUNT = 3;

	// Size should be 0x318 or 792 in decimal
	struct SaveGameSlot
	{
		TBOOL m_bUsedSlot; // should be 0x0 not sure
		uint8_t padding[791];
	};

	struct SaveData
	{
		struct Header
		{
			char m_id[4]; // 0x0 "BB"
			uint32_t m_size; // 0x4
			int m_crc; // 0x8
		} header;
		
		SaveGameSlot m_SaveGameSlots[SAVEGAME_SLOT_COUNT]; // 0xC
	};

	int m_size2;               // 0x0
	int m_size;                // 0x4
	unsigned char* m_buffer;   // 0x8
	SaveData* m_savedata;      // 0xC

	TBOOL Validate();
};

