#pragma once
#include "ADriveFunction.h"
#include "AAnimalAttributes.h"

//-----------------------------------------------------------------------------
// Drive function that returns one of the unit's AAnimalAttributes ("attribute"),
// optionally inverted ("invert").
//
// TClass 0x00783fd8 (version 0.1, size 0xc), vtable 0x00724914.
//-----------------------------------------------------------------------------
class AAnimalAttributeDriveFunction
    : public ADriveFunction
{
public:
	TDECLARE_CLASS( AAnimalAttributeDriveFunction, ADriveFunction );

public:
	// NOTE: inlined into CreateObject 0054e940 / CreateObjectInPlace 0054e9c0
	AAnimalAttributeDriveFunction()
	    : m_eAttribute( AAnimalAttributes::ATTRIBUTE_NUMOF )
	    , m_bInvert( TFALSE )
	{}

	~AAnimalAttributeDriveFunction();

	virtual TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize ) OVERRIDE;                  // +0x0c
	virtual void  Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue ) OVERRIDE; // +0x14

private:
	AAnimalAttributes::ATTRIBUTE m_eAttribute; // 0x04, "attribute"
	TBOOL                        m_bInvert;    // 0x08, "invert"
};

TSTATICASSERT( sizeof( AAnimalAttributeDriveFunction ) == 0xc );
