#pragma once
#include <Toshi/TBitField.h>

#include "ASteer.h"

class AInteriorGoal;
class AInteriorNetwork;
class AInteriorArea;

//-----------------------------------------------------------------------------
// Purpose: Intermediate base of most unit steers (AStationarySteer, ABikeSteer,
// ...). Adds a set of flags that the interaction related ASteer virtuals
// (+0x11c..+0x12c) work with, and takes the collision radius from the unit's
// collision component data when it has one.
// NOTE: not creatable at runtime (retail registers RETURNS_ZERO as the create
// functions), vtable 0x007255f8.
//-----------------------------------------------------------------------------
class ACommonSteer : public ASteer
{
public:
	TDECLARE_CLASS( ACommonSteer, ASteer );

	enum FLAG : TINT
	{
		FLAG_INTERACTING             = 0, // toggled by SetInteracting, clearing it resets FLAG_IGNORE_INVALID_POSITION and calls ResumeMovement
		FLAG_IGNORE_INVALID_POSITION = 1, // SetIgnoreInvalidPosition/IsIgnoringInvalidPosition, lets Seek/SeekFacing run without CanSeek
	};

public:
	ACommonSteer();
	~ACommonSteer();

	virtual void SetInteracting( TINT a_iUnk ) OVERRIDE; // +0x11c

	// $Barnyard: FUNCTION 005568b0
	virtual TBOOL IsInteracting() OVERRIDE { return m_oFlags.IsSet( FLAG_INTERACTING ); } // +0x120

	// $Barnyard: FUNCTION 005568d0
	virtual void SetIgnoreInvalidPosition( TINT a_iUnk ) OVERRIDE { m_oFlags.Set( FLAG_IGNORE_INVALID_POSITION, TBOOL( a_iUnk ) ); } // +0x124

	// $Barnyard: FUNCTION 005568c0
	virtual TBOOL IsIgnoringInvalidPosition() OVERRIDE { return m_oFlags.IsSet( FLAG_IGNORE_INVALID_POSITION ); } // +0x12c

	virtual TFLOAT GetRadius() OVERRIDE; // +0x160

	//-----------------------------------------------------------------------------
	// ACommonSteer's own slots (+0x174..+0x194)
	//-----------------------------------------------------------------------------
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) = 0;      // +0x174
	virtual TBOOL StopAndFace( const Toshi::TVector4& a_rcFacing ) { return TFALSE; } // +0x178, RETURNS_FALSE 006ce000

	// $Barnyard: FUNCTION 005568f0
	virtual TBOOL Command_SnapToGoal( Toshi::TPString8 a_strUnk ) { return TFALSE; } // +0x17c

	virtual TBOOL SetGoal( AInteriorGoal* a_pGoal ) { return TFALSE; }                  // +0x180, RETURNS_FALSE 006ce000, AUnitManager::SpawnUnitAtGoal
	virtual TINT  SetNetwork( AInteriorNetwork* a_pNetwork, TINT a_iUnk ) { return 0; } // +0x184, RETURNS_ZERO 00556a00, AUnitManager::SpawnUnitAtGoal

	// Whether a_pUnit's steer stands in the same interior area as this one, or in a linked one.
	virtual TBOOL IsInSameOrLinkedArea( AUnit* a_pUnit ); // +0x188

	virtual void*          GetCurrentNetwork() { return TNULL; }         // +0x18c, RETURNS_ZERO 0054d8b0
	virtual AInteriorArea* GetCurrentArea() { return TNULL; }            // +0x190, RETURNS_ZERO 0054d8b0
	virtual TINT           GetAreaInNetwork( TINT a_iUnk ) { return 0; } // +0x194, RETURNS_ZERO 006dc070

protected:
	void*                m_pNetwork; // 0x18
	Toshi::TBitField<32> m_oFlags;   // 0x1c, see FLAG
};

TSTATICASSERT( sizeof( ACommonSteer ) == 0x20 );
