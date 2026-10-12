#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Moves the units of the sequence, "type" picks how.
//-----------------------------------------------------------------------------
class AWalkActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AWalkActivity, AInteractionActivity );

	// Names in the table at 0x00775330
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_STOP         = 0, // "stop"
		TYPE_CHECKCENTRAL = 1, // "checkcentral"
		TYPE_CENTRAL      = 2, // "central"
		TYPE_MANUAL       = 3, // "manual"
		TYPE_BIPOLAR      = 4, // "bipolar"
		TYPE_BIPOLARRUN   = 5, // "bipolarrun"
		TYPE_TOCAMERA     = 6, // "tocamera"
		TYPE_FACETARGET   = 7, // "facetarget"
		TYPE_WALKBEHIND   = 8, // "walkbehind"
		TYPE_NUMOF        = 9,
	};

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_SNAP_POSITION_TO_GOAL   = BITFLAG( 0 ), // "snappositiontogoal"
		FLAGS_IGNORE_INVALID_POSITION = BITFLAG( 1 ), // "ignoreinvalidposition"
		FLAGS_NO_FAIL                 = BITFLAG( 2 ), // "nofail"
	};

public:
	AWalkActivity();
	~AWalkActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                // +0x10
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c
	virtual TBOOL  IsWalkActivity() OVERRIDE { return TTRUE; }           // +0x38, RETURNS_TRUE

	TFLOAT GetMaxRadius() const { return m_fMaxRadius; }

private:
	// The scratch memory of the activity (GetDataSize)
	struct Data
	{
		TUINT8          Unk00[ 0xc ]; // 0x00
		Toshi::TVector3 vPosition;    // 0x0c, "manualpoint"
		Toshi::TVector3 vDirection;   // 0x18, "manualpoint"
		TBOOL           bTurning;     // 0x24, TYPE_MANUAL: the units turn to the point first
		TFLOAT          fTimeLeft;    // 0x28, the activity is over once it runs out
	};

	// A unit placed around the middle by PlaceAroundCenter
	struct Slot
	{
		Toshi::TVector3 vDirection; // 0x00, from the middle
		TFLOAT          fRadius;    // 0x0c, of the unit
	};

	static constexpr TINT MAX_NUM_SLOTS = 32;

	// The middle of the units of the category
	Toshi::TVector3& GetCenter( Toshi::TVector3& a_rOutCenter, AInteractionSequence* a_pSequence );

	// Slot whose direction is the closest one to a_rcDirection (XZ), TNULL if there's none
	static Slot* FindClosestSlot( const Toshi::TVector4& a_rcDirection, Slot* a_pSlots, TINT a_iNumSlots );

	// Turns a_rcDirection (by the angle the units of a_pSlot and a_fRadius need at a_fMinRadius) away from a_pSlot
	static Toshi::TVector3& TurnAwayFromSlot( Toshi::TVector3& a_rOut, const Slot* a_pSlot, TFLOAT a_fRadius, TFLOAT a_fMinRadius, const Toshi::TVector4& a_rcDirection );

	TBOOL  IsNoUnitSeeking( AInteractionSequence* a_pSequence );
	TBOOL  IsAnyUnitTurning( AInteractionSequence* a_pSequence );
	TBOOL  IsAnyOtherUnitTurning( AInteractionSequence* a_pSequence );
	TBOOL  IsAnyOtherUnitSeeking( AInteractionSequence* a_pSequence );
	TBOOL  SeekPosition( AUnit* a_pUnit, const Toshi::TVector3& a_rcPosition, TFLOAT a_fTolerance );
	TBOOL  SeekPositionFacing( AUnit* a_pUnit, const Toshi::TVector3& a_rcPosition, const Toshi::TVector3& a_rcDirection, TFLOAT a_fUnused );
	TBOOL  SeekAroundPosition( AInteractionSequence* a_pSequence, const Toshi::TVector3& a_rcPosition, TFLOAT a_fTolerance );
	TBOOL  SeekPositionFacingAll( AInteractionSequence* a_pSequence, const Toshi::TVector3& a_rcPosition, const Toshi::TVector3& a_rcDirection, TFLOAT a_fUnused );
	TBOOL  CheckSpace( AInteractionSequence* a_pSequence, const Toshi::TVector3& a_rcCenter, TFLOAT a_fMinRadius, TFLOAT a_fMaxRadius );
	TBOOL  PlaceAroundCenter( AInteractionSequence* a_pSequence, const Toshi::TVector3& a_rcCenter, TFLOAT a_fMinRadius, TFLOAT a_fMaxRadius );
	void   CreateObstruction( AInteractionSequence* a_pSequence );
	RESULT StartStop( AInteractionSequence* a_pSequence );
	RESULT StartCheckCentral( AInteractionSequence* a_pSequence );
	RESULT StartCentral( AInteractionSequence* a_pSequence );
	RESULT StartManual( AInteractionSequence* a_pSequence );
	RESULT StartBipolarRun( AInteractionSequence* a_pSequence );
	RESULT StartToCamera( AInteractionSequence* a_pSequence );
	RESULT StartFaceTarget( AInteractionSequence* a_pSequence );
	RESULT StartWalkBehind( AInteractionSequence* a_pSequence );
	RESULT UpdateCentral( AInteractionSequence* a_pSequence );
	RESULT UpdateBipolar( AInteractionSequence* a_pSequence );
	RESULT UpdateManual( AInteractionSequence* a_pSequence );

private:
	TYPE                           m_eType;                   // 0x08, "type"
	TFLOAT                         m_fInitialForward;         // 0x0c, "initialforward"
	TFLOAT                         m_fInitialGoalToleranceSq; // 0x10, "initialgoaltolerance" squared
	TFLOAT                         m_fGoalToleranceSq;        // 0x14, "goaltolerance" squared
	TFLOAT                         m_fMinRadius;              // 0x18, "minradius" (or "radius")
	TFLOAT                         m_fMaxRadius;              // 0x1c, "maxradius" (or "radius")
	AInteractionSequence::CATEGORY m_eTarget;                 // 0x20, "facetarget" / "target"
	FLAGS                          m_uiFlags;                 // 0x24
};

TSTATICASSERT( sizeof( AWalkActivity ) == 0x28 );
