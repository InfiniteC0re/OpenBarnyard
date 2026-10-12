#pragma once
#include "AUnitStaticData.h"
#include "ADriveGroup.h"

//-----------------------------------------------------------------------------
// Static data of a unit type's drives ("ADriveSystem" block, a list of "group"
// blocks). Gives every unit of the type an ADriveComponent, which AUnitBehaviour
// asks for the drive the unit currently wants most.
//
// TClass 0x00783fa8 (version 0.1, size 0x1c), vtable 0x007248e4.
//-----------------------------------------------------------------------------
class ADriveSystem : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ADriveSystem, AUnitStaticData );

public:
	ADriveSystem();
	~ADriveSystem();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	// Returns the drive with the highest value for the unit, TNULL if no drive is above 0.
	ADrive* SelectDrive( AUnit* a_pUnit, AInteractionActivityData* a_pData );

	// Returns the drive called a_rcName, TNULL if there's none.
	ADrive* FindDrive( const Toshi::TPString8& a_rcName );

	TINT GetDataSize() const { return m_iDataSize; }

private:
	TINT         m_iNumGroups; // 0x0c
	ADriveGroup* m_pGroups;    // 0x10
	TFLOAT       m_fUnk14;     // 0x14, 2.0f, never read
	TINT         m_iDataSize;  // 0x18, size of the per unit data the functions need
};

TSTATICASSERT( sizeof( ADriveSystem ) == 0x1c );
