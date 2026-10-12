#pragma once
#include "ACharacterUnit.h"
#include "ACarryableUnitHelper.h"

// A character that can be carried (the chicks): an ACharacterUnit with the
// ACarryableUnitHelper of the items. Once it's put down it bounces like an item
// (ABallSteer2) and then gets its previous steer back.
//
// TClass 0x0077e788 (version 1.1, size 0xe4, runtime creatable), vtable 0x007183f0,
// secondary (AGoalCollisionUpdateInterface) vtable 0x007183e0.
class ACharacterUnitCarrable : public ACharacterUnit
{
public:
	TDECLARE_CLASS( ACharacterUnitCarrable, ACharacterUnit );

	// vtable 0x007183b8
	class ACarryableCharacterHelper : public ACarryableUnitHelper
	{
	public:
		ACarryableCharacterHelper( AUnit* a_pUnit )
		    : ACarryableUnitHelper( a_pUnit ) {}

		~ACarryableCharacterHelper();

		virtual TBOOL PutAway() override; // +0x1c, detaches the character from the holder
	};

public:
	ACharacterUnitCarrable();
	~ACharacterUnitCarrable();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*       GetFunctionTable() override;   // +0x0c
	virtual ACarryableUnitHelper* GetCarryableHelper() override; // +0x54

public:
	static AFunctionTable ms_oFunctionTable;

private:
	ACarryableCharacterHelper m_oHelper; // 0xa0
};

TSTATICASSERT( sizeof( ACharacterUnitCarrable ) == 0xe4 );
