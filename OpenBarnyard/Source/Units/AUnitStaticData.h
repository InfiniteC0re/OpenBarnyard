#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TDList.h>

#include <Plugins/PPropertyParser/PBProperties.h>

class AUnit;

// Per-type data block built from a named sub-block of a unit type's properties.
// AUnitType::LoadStaticData looks the subclass up by the block's name.
class AUnitStaticData
    : public Toshi::TObject
    , public Toshi::TDList<AUnitStaticData>::TNode
{
public:
	TDECLARE_CLASS( AUnitStaticData, Toshi::TObject );

public:
	AUnitStaticData();
	~AUnitStaticData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) { return TTRUE; } // +0x0c, RETURNS_TRUE
	virtual void  OnUnitCreate( AUnit* a_pUnit ) {}                             // +0x10, DOES_NOTHING
	virtual void  OnUnitDestroy( AUnit* a_pUnit ) {}                            // +0x14, DOES_NOTHING
};
