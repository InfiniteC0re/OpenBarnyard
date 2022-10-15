#pragma once

#include "Toshi/Core/TObject.h"
#include "Toshi/Core/TGenericDList.h"

class ABaseEntity : Toshi::TObject, Toshi::T2DList<ABaseEntity>::TNode
{
	static constexpr int ENTITY_HASH_SIZE = 0x3FF;

	enum FLAG
	{
		FLAG_Created   = BITFIELD(0),
		FLAG_DontTick  = BITFIELD(8),
		FLAG_LowLOD    = BITFIELD(9),
		FLAG_MediumLOD = BITFIELD(10),
		FLAG_HighLOD   = BITFIELD(11),
		FLAG_LODMask   = FLAG_LowLOD | FLAG_MediumLOD | FLAG_HighLOD,
	};

	static int sm_EntityHash[ENTITY_HASH_SIZE];

	int m_State;                                           // 0xC
	int m_iID;                                             // 0xE
	uint8_t m_Flags;                                       // 0x70
	Toshi::T2DList<ABaseEntity>::TNode* m_pEntityCellPair; // 0x7C
};

