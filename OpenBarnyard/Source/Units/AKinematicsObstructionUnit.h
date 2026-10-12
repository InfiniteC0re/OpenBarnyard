#pragma once
#include "AKinematicsObstruction.h"
#include "AUnitProxy.h"

#include <Toshi/TRefCounted.h>

// Obstruction that follows a unit: its steer, or one of its model's bones.
// Created by AVisibleUnit::Create for units whose AObjectTraits are flagged
// "Obstruction" (and for units without AObjectTraits).
class AKinematicsObstructionUnit : public AKinematicsObstruction
{
public:
	TDECLARE_CLASS( AKinematicsObstructionUnit, AKinematicsObstruction );

public:
	// NOTE: needed by TDEFINE_CLASS, retail can only create it from a unit proxy
	AKinematicsObstructionUnit() = default;
	AKinematicsObstructionUnit( Toshi::TRef<AUnitProxy> a_pUnitProxy );
	~AKinematicsObstructionUnit();

	//-----------------------------------------------------------------------------
	// AKinematicsObstruction
	//-----------------------------------------------------------------------------
	virtual const Toshi::TVector4* GetPosition() OVERRIDE;                                           // +0x10
	virtual Toshi::TVector4        GetDirection() OVERRIDE;                                          // +0x14
	virtual Toshi::TVector4        GetVelocity() OVERRIDE;                                           // +0x18
	virtual TFLOAT                 GetSpeed() OVERRIDE;                                              // +0x1c
	virtual TFLOAT                 GetRadius() OVERRIDE;                                             // +0x20
	virtual const Toshi::TVector2* GetHalfSize() OVERRIDE;                                           // +0x24
	virtual Toshi::TVector4&       PredictPosition( TINT a_iUnk, Toshi::TVector4& a_rOut ) OVERRIDE; // +0x28
	virtual SHAPE                  GetShape() OVERRIDE { return m_iShape; }                          // +0x2c, 004ffb60 (shared)
	virtual TYPE                   GetObstructionType() OVERRIDE { return TYPE_UNIT; }               // +0x30, 004c4a20 (shared, returns 1)

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_USE_BONE = BITFLAG( 0 ), // the position is the one of the C_Pelvis bone
	};

	// Sets (a_bSet) or clears a_eFlags
	void SetFlags( FLAGS a_eFlags, TBOOL a_bSet );

	AUnit* GetUnit() const { return m_pUnitProxy.Get()->GetUnit(); }

private:
	FLAGS                   m_eFlags = 0;            // 0x04
	Toshi::TRef<AUnitProxy> m_pUnitProxy;            // 0x08
	SHAPE                   m_iShape = SHAPE_CIRCLE; // 0x0c
	TINT                    m_iBone  = -1;           // 0x10
};

TSTATICASSERT( sizeof( AKinematicsObstructionUnit ) == 0x14 );
