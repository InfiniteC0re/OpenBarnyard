#pragma once
#include "ACollisionObjectSimple.h"

#include <Toshi/T2Vector.h>

class AUnit;
class AVisibleUnit;
class AModelInstance;

// Collision sphere of a unit, following its steer (or one of its bones).
// Created by AVisibleUnit::Create from the unit type's AUnitCollisionComponentData.
class ACollisionObjectUnit
    : public ACollisionObjectSimple
{
public:
	TDECLARE_CLASS( ACollisionObjectUnit, ACollisionObjectSimple );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_CREATED                 = BITFLAG( 0 ), // set by Create
		FLAGS_IGNORE_OBJECT_COLLISION = BITFLAG( 1 ), // AUnitCollisionComponentData::m_bIgnoreObjectCollision, collision tests always fail
	};

	static constexpr TINT MAX_NUM_IGNORED_UNITS = 5;

public:
	ACollisionObjectUnit();
	~ACollisionObjectUnit();

	// Sets the object up from the unit's AUnitCollisionComponentData and activates it.
	TBOOL Create( AVisibleUnit* a_pUnit, AModelInstance* a_pModelInstance, TBOOL a_bIgnoreObjectCollision );

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual void  Update( void* a_pUnk ) OVERRIDE;                                                                                                                                                                                                          // +0x1c, moves the sphere to the unit
	virtual TBOOL CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE; // +0x24
	virtual TBOOL CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE;                     // +0x28
	virtual TBOOL CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) OVERRIDE;                                                                                             // +0x2c
	virtual void  GetVelocity( Toshi::TVector4& a_rOutVelocity ) OVERRIDE;                                                                                                                                                                                  // +0x44, velocity of the unit's steer
	virtual void* GetOwnerUnit() OVERRIDE;                                                                                                                                                                                                                  // +0x50, returns the unit
	virtual void  TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) OVERRIDE;                                                                                                                                                            // +0x58
	virtual void  SetVelocity( void* a_pVelocity ) OVERRIDE;                                                                                                                                                                                                // +0x5c, sets the steer's direction and speed

	AVisibleUnit* GetUnit() const { return m_pUnit; }

	// Collisions with a_pUnit are ignored
	void AddIgnoredUnit( AUnit* a_pUnit );
	void SetBone( TINT a_iBone ) { m_iBone = a_iBone; }

private:
	// Whether a_pOther belongs to a unit this object ignores, or ignores the unit of this object.
	TBOOL IsIgnoredUnitObject( ACollisionObjectUnit* a_pOther );

private:
	AVisibleUnit*                                  m_pUnit;           // 0x38
	FLAGS                                          m_eFlags;          // 0x3c
	Toshi::T2Vector<AUnit*, MAX_NUM_IGNORED_UNITS> m_vecIgnoredUnits; // 0x40
	TINT                                           m_iBone;           // 0x58, -1 to follow the steer
};

TSTATICASSERT( sizeof( ACollisionObjectUnit ) == 0x5c );
