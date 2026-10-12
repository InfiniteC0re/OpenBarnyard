#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"

class AAnimatableObject;

//-----------------------------------------------------------------------------
// Last effect of the pick up chains: collects the unit into the inventory of
// the collector (pick up units go to the pick up counter instead) or, for an
// object of a mini game, adds the pick ups and notifies the callback.
// Re-activates ACameraManager and finishes on its first update.
//
// TClass 0x007b4de0 (size 0x24), vtable 0x0073955c.
//-----------------------------------------------------------------------------
class AInventoryAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( AInventoryAniEffect, AAniEffect );

	// NOTE: retail name is CallBackInterface (Wii strings)
	class CallBackInterface
	{
	public:
		// Called when the object has been collected
		virtual void OnObjectCollected( AAnimatableObject* a_pObject ) = 0; // +0x00
	};

public:
	AInventoryAniEffect();
	~AInventoryAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual AUnit* GetUnit() OVERRIDE;                     // +0x10
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x14

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Unk_0x28() {} // +0x28, DOES_NOTHING
	virtual void Unk_0x2c() {} // +0x2c, DOES_NOTHING

	// The unit to collect. It's taken out of its pool.
	void SetUnit( AUnit* a_pUnit );

	// The unit whose inventory gets the unit
	void SetCollector( AUnit* a_pCollector );

	// The unit goes to the "InventoryForUse" of its AObjectTraits instead of the "InventoryForStore"
	// and the player controls are re-enabled (used when the item is taken out of the inventory)
	void SetTakeOut( TBOOL a_bTakeOut ) { m_bTakeOut = a_bTakeOut; }

	// Collects an object of a mini game instead of a unit
	void SetObject( AAnimatableObject* a_pObject, TINT a_iCount, CallBackInterface* a_pCallBack )
	{
		m_iCount    = a_iCount;
		m_pObject   = a_pObject;
		m_pCallBack = a_pCallBack;
	}

private:
	Toshi::TRef<AUnitProxy> m_oUnit;      // 0x0c
	Toshi::TRef<AUnitProxy> m_oCollector; // 0x10
	TBOOL                   m_bTakeOut;   // 0x14
	AAnimatableObject*      m_pObject;    // 0x18, used if there's no unit
	TINT                    m_iCount;     // 0x1c, > 0: the pick ups go to APlayerSimProfile::AddMiniGamePickups
	CallBackInterface*      m_pCallBack;  // 0x20
};

TSTATICASSERT( sizeof( AInventoryAniEffect ) == 0x24 );
