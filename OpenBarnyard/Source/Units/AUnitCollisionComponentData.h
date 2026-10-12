#pragma once
#include "AUnitStaticData.h"

// Collision shape of a unit type, read from its "AUnitCollisionComponentData" properties.
class AUnitCollisionComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitCollisionComponentData, AUnitStaticData );

public:
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	TFLOAT GetCollisionRadius() const { return m_fCollisionRadius; }

public:
	TFLOAT m_fCollisionRadius;       // 0x0c
	TFLOAT m_fSphereRaise;           // 0x10
	TFLOAT m_fHeight;                // 0x14, defaults to the radius
	TBOOL  m_bIgnoreObjectCollision; // 0x18
	TINT   m_iCollideFlags;          // 0x1c, "CollideFlags", ACollisionObject::TYPE_ flags of the unit (-1: the default type)
};

TSTATICASSERT( sizeof( AUnitCollisionComponentData ) == 0x20 );
