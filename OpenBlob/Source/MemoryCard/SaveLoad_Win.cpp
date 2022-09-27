#include "ToshiPCH.h"
#include "SaveLoad_Win.h"

void SaveLoadSKU::OnUpdate()
{
	switch (m_Operation)
	{
	case SaveLoadSKU::OPERATION_SAVE:
		break;
	case SaveLoadSKU::OPERATION_NEWGAMELOAD:
		TASSERT(TFALSE, "OPERATION_NEWGAMELOAD not implemented");
		break;
	case SaveLoadSKU::OPERATION_NOPROMPTSAVE:
		bool bRes = m_pSaveData->Validate();
		if (bRes)
		{
			TIMPLEMENT("m_pSaveData->FUN_005e3bf0()");
			m_error = -1;
			m_Operation = OPERATION_UNK;
		}
		break;
	case SaveLoadSKU::OPERATION_AUTOSAVE:
		break;
	default:
		break;
	}
}
