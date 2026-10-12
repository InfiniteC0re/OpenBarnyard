#pragma once
#include <Toshi/TObject.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class AUnit;
struct AInteractionActivityData;

//-----------------------------------------------------------------------------
// Part of an ADrive ("need", "avail" or "filter" block): computes a value for a
// unit, or modifies the drive's value (filters). The subclass is picked by the
// block's "type". Functions that keep state per unit reserve it in the unit's
// ADriveComponent data while they are created (a_pDataSize).
//
// TClass 0x00783f48 (version 0.1, size 0x4, not creatable), vtable 0x0072488c.
//-----------------------------------------------------------------------------
class ADriveFunction
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADriveFunction, Toshi::TObject );

public:
	ADriveFunction();
	~ADriveFunction();

	virtual TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize ) { return TTRUE; }     // +0x0c, RETURNS_TRUE 00558f80
	virtual void  InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData ) {}                     // +0x10, DOES_NOTHING 0054df00
	virtual void  Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue ) = 0; // +0x14

	// Creates the function named by the "type" of a_pProperties, TNULL if there's none or it fails.
	static ADriveFunction* CreateFromProperties( const PBProperties* a_pProperties, TINT* a_pDataSize );
};

TSTATICASSERT( sizeof( ADriveFunction ) == 0x4 );
