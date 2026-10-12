#pragma once
#include "AUnit.h"

//-----------------------------------------------------------------------------
// A plain unit that isn't put into the unit hash.
// TClass parent AUnit, size 0x40, vtable 00717c00.
//-----------------------------------------------------------------------------
class AUnitDummy
    : public AUnit
{
public:
	TDECLARE_CLASS( AUnitDummy, AUnit );

public:
	AUnitDummy();
	~AUnitDummy();

	virtual TINT           SetUnk34( TINT a_iValue ) OVERRIDE { return 0; } // +0x34, RETURNS_ZERO 006dc070
	virtual TBOOL          IsUnk34Set() OVERRIDE { return TFALSE; }         // +0x38, RETURNS_FALSE
	virtual TBOOL          Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void           OnDestroy() OVERRIDE {}                          // +0x60, DOES_NOTHING
	virtual Toshi::TClass* GetSteerClass() OVERRIDE;                        // +0x80

	//-----------------------------------------------------------------------------
	// Own methods (unknown, all stubs)
	//-----------------------------------------------------------------------------
	virtual TBOOL Unknown1() { return TFALSE; }        // +0x90, RETURNS_FALSE
	virtual TBOOL Unknown2( void* ) { return TFALSE; } // +0x94, RETURNS_FALSE 006ce000
	virtual TBOOL Unknown3( void* ) { return TFALSE; } // +0x98, RETURNS_FALSE 006ce000
	virtual TBOOL Unknown4() { return TFALSE; }        // +0x9c, RETURNS_FALSE
	virtual void  Unknown5() {}                        // +0xa0, DOES_NOTHING
};
