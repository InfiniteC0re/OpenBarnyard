#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"

#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AAnimatableObject;
class AUnitFacialComponentData;

//-----------------------------------------------------------------------------
// Purpose: Makes the head of an animated unit blink ("blink" overlay) and look
// around, at a position or at another unit, with the "lookl"/"lookr"/"looku"/
// "lookd" overlay animations (only if the animation set has them).
// Created by AUnitFacialComponentData, owned by AGeneralUnitComponentManager.
//
// TClass 0x0078a3f8 (version 1.0, size 0x50), vtable 0x00728dcc.
//-----------------------------------------------------------------------------
class AUnitFacialComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AUnitFacialComponent, AUnitComponent );

	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE               = 0,
		FLAGS_BLINK              = BITFLAG( 0 ), // blinks every 1-3 seconds
		FLAGS_LOOKING            = BITFLAG( 1 ), // turns the head
		FLAGS_LOOK_RANDOM        = BITFLAG( 2 ), // looks in random directions
		FLAGS_LOOK_AT_POSITION   = BITFLAG( 3 ), // looks at m_vLookTarget
		FLAGS_LOOK_AT_UNIT       = BITFLAG( 4 ), // looks at m_pLookTarget (m_vLookTarget is the offset)
		FLAGS_LOOK_TARGET_RANDOM = BITFLAG( 5 ), // looks at the units in front of it
		FLAGS_STOPPING           = BITFLAG( 6 ), // turns the head back before it stops looking
		FLAGS_TIMED              = BITFLAG( 7 ), // stops looking after m_fLookDuration seconds
	};

public:
	AUnitFacialComponent();
	~AUnitFacialComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;              // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                    // +0x10
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Sets the component up for a_pUnit with the data of its type
	TBOOL Create( AUnit* a_pUnit, AUnitFacialComponentData* a_pData );

	// Looks at a_pUnit (at its position + a_rcOffset). Stops after a_fDuration seconds unless it's negative.
	void LookAtUnit( AUnit* a_pUnit, const Toshi::TVector4& a_rcOffset, TFLOAT a_fDuration );

	// Looks at the head of a_pUnit. Stops after a_fDuration seconds unless it's negative.
	void LookAtUnitHead( AUnit* a_pUnit, TFLOAT a_fDuration );

	// Looks at a_rcPosition. Stops after a_fDuration seconds unless it's negative.
	void LookAtPosition( const Toshi::TVector4& a_rcPosition, TFLOAT a_fDuration );

	TBOOL IsLooking() const { return m_uiFlags & FLAGS_LOOKING; }

	// FSM natives
	void SetLookRandom( TFLOAT a_fDuration );
	void SetLookTargetRandom( TFLOAT a_fDuration );
	void StopLook();
	void SetBlink( TBOOL a_bBlink );
	void SetLookTargetByName( const Toshi::TPString8& a_rcName );

	// Returns the position of the head bone of a_pUnit relative to the unit (TFALSE and zero if it has none)
	static TBOOL GetHeadPosition( AUnit* a_pUnit, Toshi::TVector4& a_rOutPosition );

	// Returns the height of the head bone of a_pUnit
	static TFLOAT GetHeadHeight( AUnit* a_pUnit );

private:
	// Starts (a_bLook) or stops looking, resets the target
	TBOOL SetLooking( TBOOL a_bLook );

	// Updates the look direction to look at a_rcTarget
	void UpdateLookTarget( AUnit* a_pOwner, const Toshi::TVector4& a_rcTarget );

	// Returns the closest unit within a_fRadius in front of the owner (the player first)
	// NOTE: a_rcPosition is unused, the owner's position is used instead
	AUnit* FindLookTarget( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fMinDot );

	// Moves a_rfCurrent towards a_rfTarget and sets the weights of the look overlays.
	// Returns TTRUE if a_rfCurrent was already at the target.
	static TBOOL UpdateLookAnim( AAnimatableObject* a_pAnimatableObject, TFLOAT& a_rfCurrent, const TFLOAT& a_rfTarget, const Toshi::TPString8& a_rcNegativeAnim, const Toshi::TPString8& a_rcPositiveAnim, TFLOAT a_fSpeed, TFLOAT a_fDeltaTime );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	FLAGS                     m_uiFlags;          // 0x10
	TFLOAT                    m_fBlinkTimer;      // 0x14
	TFLOAT                    m_fLookYawTarget;   // 0x18, -1 (left limit) .. 1 (right limit)
	TFLOAT                    m_fLookYaw;         // 0x1c
	TFLOAT                    m_fLookPitchTarget; // 0x20, -1 (down limit) .. 1 (up limit)
	TFLOAT                    m_fLookPitch;       // 0x24
	TFLOAT                    m_fLookSpeed;       // 0x28
	TFLOAT                    m_fLookTimer;       // 0x2c
	TFLOAT                    m_fLookDuration;    // 0x30
	TFLOAT                    m_fHeadHeight;      // 0x34
	Toshi::TRef<AUnitProxy>   m_pLookTarget;      // 0x38
	Toshi::TVector4           m_vLookTarget;      // 0x3c
	AUnitFacialComponentData* m_pData;            // 0x4c
};

TSTATICASSERT( sizeof( AUnitFacialComponent ) == 0x50 );

//-----------------------------------------------------------------------------
// Purpose: Static data of the units that have a face ("AUnitFacialComponentData"
// block), creates an AUnitFacialComponent for every unit of the type.
//
// TClass 0x0078a458 (size 0x24), vtable 0x00728e04.
//-----------------------------------------------------------------------------
class AUnitFacialComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitFacialComponentData, AUnitStaticData );

	friend class AUnitFacialComponent;

public:
	AUnitFacialComponentData();
	~AUnitFacialComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

	const Toshi::TPString8& GetBoneName() const { return m_strBoneName; }

private:
	TFLOAT           m_fLookUpLimit;    // 0x0c, "LookUpLimit"
	TFLOAT           m_fLookDownLimit;  // 0x10, "LookDownLimit"
	TFLOAT           m_fLookLeftLimit;  // 0x14, "LookLeftLimit"
	TFLOAT           m_fLookRightLimit; // 0x18, "LookRightLimit"
	TFLOAT           m_fLookSpeed;      // 0x1c, "LookSpeed"
	Toshi::TPString8 m_strBoneName;     // 0x20, "BoneName"
};

TSTATICASSERT( sizeof( AUnitFacialComponentData ) == 0x24 );
