#pragma once
#include "ADriveFunction.h"

//-----------------------------------------------------------------------------
// Drive filter that keeps a value from flickering inside "minrange".."maxrange":
// once the value enters the range it sticks to the bound it is closest to. The
// last value is kept in the unit's ADriveComponent data.
//
// TClass 0x00784008 (version 0.1, size 0x10), vtable 0x00724960.
//-----------------------------------------------------------------------------
class AHysteresisDriveFunction
    : public ADriveFunction
{
public:
	TDECLARE_CLASS( AHysteresisDriveFunction, ADriveFunction );

public:
	// NOTE: inlined into CreateObject 0054ec20 / CreateObjectInPlace 0054eca0
	AHysteresisDriveFunction()
	    : m_iDataOffset( -1 )
	    , m_fMinRange( 0.5f )
	    , m_fMaxRange( 0.5f )
	{}

	~AHysteresisDriveFunction();

	virtual TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize ) OVERRIDE;                  // +0x0c
	virtual void  InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData ) OVERRIDE;                   // +0x10
	virtual void  Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue ) OVERRIDE; // +0x14

private:
	TINT   m_iDataOffset; // 0x04, of the last value in the unit data
	TFLOAT m_fMinRange;   // 0x08, "minrange"
	TFLOAT m_fMaxRange;   // 0x0c, "maxrange"
};

TSTATICASSERT( sizeof( AHysteresisDriveFunction ) == 0x10 );
