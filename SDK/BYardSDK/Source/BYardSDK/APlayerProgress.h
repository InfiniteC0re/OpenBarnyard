#pragma once
#include "THookedSingleton.h"
#include "ASaveSlot.h"

struct ASaveBlock
{
	TUINT8    m_aHeader[ 0x44 ];
	ASaveSlot m_aSlots[ 4 ];
};

class APlayerProgress
    : public THookedSingleton<APlayerProgress, 0x007b5ea8>
{
public:
	static constexpr TINT SLOT_COUNT = 4;

	ASaveSlot* GetSaveSlot( TINT a_iSlot ) { return m_pBlock ? &m_pBlock->m_aSlots[ a_iSlot ] : TNULL; }

	TINT GetActiveSlot() const { return m_iActiveSlot; }
	void SetActiveSlot( TINT a_iSlot ) { m_iActiveSlot = a_iSlot; }

	void ClearActiveSlot();

	void StartNewGame();
	void LoadGame();

private:
	TUINT8      m_aUnk1[ 0x1c ];  // 0x00
	ASaveBlock* m_pBlock;         // 0x1c
	TINT        m_iActiveSlot;    // 0x20
};
