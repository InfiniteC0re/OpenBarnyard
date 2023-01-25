#pragma once

#include "Toshi/Core/TObject.h"
#include "Toshi2/T2DList.h"

class ABaseEntity : Toshi::TObject, public Toshi::T2DList<ABaseEntity>::Node
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

	virtual ~ABaseEntity() = default;
	virtual void Tick(float a_fTicks) { m_fTicks += a_fTicks; }
	virtual void OnLODChanging() = 0;

	static int sm_EntityHash[ENTITY_HASH_SIZE];

	int m_State;                                           // 0xC
	int m_iID;                                             // 0xE
	float m_fTicks;                                        // 0x6C de blob / 100 globs
	uint8_t m_Flags;                                       // 0x70
	Toshi::T2DList<ABaseEntity>::Node* m_pEntityCellPair;  // 0x7C
};

