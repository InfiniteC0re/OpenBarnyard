#pragma once
#include "ACarryableUnit.h"
#include "MiniGame/AVeggiePatchDefenderGame.h"

#include <Toshi/T2DList.h>

//-----------------------------------------------------------------------------
// A veggie of the Veggie Patch minigame (unit types "vpd_carrot", "vpd_melon",
// "vpd_cabbage" and "vpd_pumpkin"), created by AVeggiePatchDefenderGame in its
// VeggieSlot. The raccoons pick them up and carry them out of the patch.
// TClass 00782... (parent ACarryableUnit, size 0x100), vtables 0071feb0,
// 0071fea4 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AVeggiePatchVeggie
    : public ACarryableUnit
    , public Toshi::T2DList<AVeggiePatchVeggie>::Node
{
public:
	TDECLARE_CLASS( AVeggiePatchVeggie, ACarryableUnit );

	enum STATE_ : TINT
	{
		STATE_GROWING, // 0, in its slot
		STATE_CARRIED, // 1, by a raccoon
		STATE_DROPPED, // 2, thrown by a raccoon (AThrowItemSteer)
		STATE_FADING,  // 3, landed, fades out and grows back in a free slot
		STATE_LOST,    // 4, taken out of the patch
	};

public:
	AVeggiePatchVeggie();
	~AVeggiePatchVeggie();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	// Puts the veggie in a_pSlot
	void Plant( AVeggiePatchDefenderGame::VeggieSlot* a_pSlot, AVeggiePatchDefenderGame::Patch* a_pPatch );

	// A raccoon picks the veggie up. Returns TFALSE if it's not in its slot.
	TBOOL PickUp();

	// A raccoon drops the veggie at a_rcPosition
	void Drop( const Toshi::TVector4& a_rcPosition );

	// A raccoon has taken the veggie out of the patch
	void SetLost();

	// Hides the veggie
	void Hide();

	AVeggiePatchDefenderGame::VEGGIE_ GetKind() const { return m_eKind; }
	STATE_                            GetState() const { return m_eState; }

public:
	AVeggiePatchDefenderGame::VEGGIE_     m_eKind;       // 0xe4
	STATE_                                m_eState;      // 0xe8
	AVeggiePatchDefenderGame::VeggieSlot* m_pSlot;       // 0xec
	AVeggiePatchDefenderGame::Patch*      m_pPatch;      // 0xf0
	TINT                                  m_iUnkf4;      // 0xf4
	TFLOAT                                m_fTimer;      // 0xf8
	TINT                                  m_iDustEffect; // 0xfc, "dustpuff"
};
