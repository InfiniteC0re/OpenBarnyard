#pragma once

#include "Toshi2/T2String8.h"
#include "Toshi/Utils/TUtil.h"

class EnSaveData
{
public:

	const int SAVEGAME_SLOT_COUNT = 3;

	// Size should be 0x318 or 792 in decimal
	struct SaveGameSlot
	{
		bool m_bUsedSlot; // should be 0x0 not sure
	};

	struct SaveData
	{
		struct Header
		{
			char m_id[4]; // 0x0 "BB"
			uint32_t m_size; // 0x4
			int m_crc; // 0x8
		} header;
		
		SaveGameSlot* m_SaveGameSlots; // 0xC
	};

	unsigned char* m_buffer;
	int m_size;
	SaveData* m_savedata; // 0xC

	bool Validate();
};

