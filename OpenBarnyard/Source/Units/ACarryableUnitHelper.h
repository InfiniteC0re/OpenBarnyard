#pragma once
#include <Toshi/Typedefs.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

class AUnit;
class AObjectTraits;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// Part of a unit that can be carried (ACarryableUnit keeps one at +0x98 and
// returns it from AUnit::GetCarryableHelper): who holds the unit, the carry
// visuals it attaches to the holder and the AObjectTraits triggers (WhenThrow,
// WhenUse, WhenEquip, WhenBreak) it fires.
// vtable 0x007286fc.
// NOTE: original method names are unknown.
//-----------------------------------------------------------------------------
class ACarryableUnitHelper
{
public:
	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE   = 0,
		FLAGS_THROWN = BITFLAG( 0 ), // set by Throw, cleared by Equip
		FLAGS_BROKEN = BITFLAG( 1 ), // the WhenBreak trigger was fired
	};

	static constexpr TINT MAX_NUM_ATTACHED_OBJECTS = 10;

public:
	ACarryableUnitHelper( AUnit* a_pUnit );
	virtual ~ACarryableUnitHelper(); // +0x00

	virtual TBOOL          Use();                                        // +0x04, fires WhenUse
	virtual TBOOL          StartThrow();                                 // +0x08, the idle player starts the throw (APlayerSteer::STATE_THROW)
	virtual void           Throw();                                      // +0x0c, fires WhenThrow
	virtual TBOOL          Equip();                                      // +0x10, fires WhenEquip
	virtual TBOOL          Carry( TBOOL a_bAttachUnit, TINT a_iUnused ); // +0x14, attaches the unit (and copies of it for a bunch) to the holder's hand
	virtual TBOOL          UnEquip();                                    // +0x18, fires ObjectUnEquip if the object has WhenEquip
	virtual TBOOL          PutAway();                                    // +0x1c
	virtual TBOOL          Update( TFLOAT a_fDeltaTime );                // +0x20, fires WhenBreak once a thrown object hits the ground
	virtual AObjectTraits* GetObjectTraits();                            // +0x24

	// The "MaxCarry" of the object traits
	TINT GetMaxCarry();

	// Detaches the unit from the bone of the holder (if it's there) and destroys the carry visuals
	void DetachFromHolder();

	// Destroys the carry visuals owned by the unit and the ones owned by nothing
	void DestroyAttachedObjects();

	// Destroys the carry visuals owned by nothing
	void DestroyFreeAttachedObjects();

	// Starts a delay of a_fTime seconds of game time
	void  SetDelay( TFLOAT a_fTime );
	TBOOL IsDelayOver();

	AUnit* GetHolder() const { return m_pHolder; }
	void   SetHolder( AUnit* a_pHolder ) { m_pHolder = a_pHolder; }
	AUnit* GetUnit() const { return m_pUnit; }
	FLAGS  GetFlags() const { return m_eFlags; }

protected:
	FLAGS                                                         m_eFlags;             // 0x04
	AUnit*                                                        m_pHolder;            // 0x08, the unit that carries the object (or has it in its inventory)
	AUnit*                                                        m_pUnit;              // 0x0c, the carried unit
	TINT                                                          m_iUnk10;             // 0x10, not initialised
	TFLOAT                                                        m_fDelayEndTime;      // 0x14, AGameTimeManager total time
	Toshi::T2Vector<AAnimatableObject*, MAX_NUM_ATTACHED_OBJECTS> m_vecAttachedObjects; // 0x18, copies of the unit attached to the holder
};
