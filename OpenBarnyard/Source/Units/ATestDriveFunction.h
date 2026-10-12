#pragma once
#include "ADriveFunction.h"

//-----------------------------------------------------------------------------
// Drive function for testing: returns a "random" value up to "value" or the
// "fixed" "value", or 0 with a chance of "nullchance".
//
// TClass 0x00784038 (version 0.1, size 0x10), vtable 0x007249b4.
//-----------------------------------------------------------------------------
class ATestDriveFunction
    : public ADriveFunction
{
public:
	TDECLARE_CLASS( ATestDriveFunction, ADriveFunction );

	// Order of the name table at 0x0077421c
	using ACTION = TINT;
	enum ACTION_ : ACTION
	{
		ACTION_RANDOM, // "random"
		ACTION_FIXED,  // "fixed"
		ACTION_NUMOF,  // the value is left untouched
	};

public:
	// NOTE: inlined into CreateObject 0054ee90 / CreateObjectInPlace 0054ef10
	ATestDriveFunction()
	    : m_eAction( ACTION_NUMOF )
	    , m_fValue( 1.0f )
	    , m_fNullChance( 0.0f )
	{}

	~ATestDriveFunction();

	virtual TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize ) OVERRIDE;                  // +0x0c
	virtual void  Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue ) OVERRIDE; // +0x14

private:
	ACTION m_eAction;     // 0x04, "action"
	TFLOAT m_fValue;      // 0x08, "value"
	TFLOAT m_fNullChance; // 0x0c, "nullchance"
};

TSTATICASSERT( sizeof( ATestDriveFunction ) == 0x10 );
