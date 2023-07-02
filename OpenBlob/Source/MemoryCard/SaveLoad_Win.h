#pragma once

#include "EnSaveData.h"
#include "SaveLoad.h"

class SaveLoadSKU : public SaveLoad
{
public:
	// Not in the correct order but names should be correct
	enum Operation : int
	{
		OPERATION_UNK = -1,
		OPERATION_SAVE,
		OPERATION_NEWGAMELOAD, // Correct position
		OPERATION_NOPROMPTSAVE,
		OPERATION_AUTOSAVE,
		OPERATION_UNK2,
		OPERATION_UNK3,
		OPERATION_UNK4,
		OPERATION_UNK5,
		OPERATION_UNK6,
		OPERATION_UNK7,
	};

	int8_t m_error; // 0x10
	Operation m_Operation; // 0x14
	EnSaveData* m_pSaveData; // 0x18

	void OnUpdate();
};

