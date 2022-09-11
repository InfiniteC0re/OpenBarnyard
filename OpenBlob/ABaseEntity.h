#pragma once

#include "Toshi/Core/TObject.h"
#include "Toshi/Core/TGenericDList.h"


class ABaseEntity : Toshi::TObject, Toshi::T2DList<ABaseEntity>::TNode
{

	static const int ENTITY_HASH_SIZE = 0x3FF;

	enum Flags
	{
		FLAG_Created = 1,
		FLAG_DontTick = 256,
		FLAG_LowLOD = 0x200,
		FLAG_MediumLOD = 0x400,
		FLAG_HighLOD = 0x800,
		FLAG_LODMask = 0xE00,
	};

	static int sm_EntityHash[ENTITY_HASH_SIZE];

	int m_State; // 0xC
	int m_iID; // 0xE
	uint8_t m_Flags; // 0x70
	Toshi::T2DList<ABaseEntity>::TNode* m_pEntityCellPair; // 0x7C
};

