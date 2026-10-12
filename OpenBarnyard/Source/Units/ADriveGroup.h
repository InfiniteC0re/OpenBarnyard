#pragma once
#include "ADrive.h"

//-----------------------------------------------------------------------------
// A "group" block of an ADriveSystem: the drives that compete with each other.
// The best drive of a group only replaces the best one of the groups evaluated
// before it if its value, scaled by "subsumefactor", is at least as high.
//
// TClass 0x00783f78 (version 0.1, size 0x10), vtable 0x007248b4.
//-----------------------------------------------------------------------------
class ADriveGroup
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADriveGroup, Toshi::TObject );

public:
	ADriveGroup();
	~ADriveGroup();

	// Reads the "drive" blocks of the group.
	TBOOL Create( const PBProperties* a_pProperties, TINT* a_pDataSize );

	// Lets the drives initialise their data of a new unit.
	void InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData );

	// Replaces a_pBest with the best drive of the group if it subsumes it.
	void Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, ADrive::Rating* a_pBest );

	// Returns the drive called a_rcName, TNULL if there's none.
	ADrive* FindDrive( const Toshi::TPString8& a_rcName );

private:
	TINT    m_iNumDrives;     // 0x04
	ADrive* m_pDrives;        // 0x08
	TFLOAT  m_fSubsumeFactor; // 0x0c, "subsumefactor"
};

TSTATICASSERT( sizeof( ADriveGroup ) == 0x10 );
