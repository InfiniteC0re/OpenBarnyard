#include "pch.h"
#include "AUnitCollisionComponentData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061f940, CreateObject 0061fa30, CreateObjectInPlace 0061f950
TDEFINE_CLASS( AUnitCollisionComponentData );

TPSTRING8_DECLARE( CollisionRadius );
TPSTRING8_DECLARE( Height );
TPSTRING8_DECLARE( SphereRaise );
TPSTRING8_DECLARE( IgnoreObjectCollision );
TPSTRING8_DECLARE( CollideFlags );

// $Barnyard: FUNCTION 0061f9a0
TBOOL AUnitCollisionComponentData::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalFloatOrIntValue( m_fCollisionRadius, TPS8( CollisionRadius ) );

	m_fHeight = m_fCollisionRadius;
	a_pProperties->GetOptionalPropertyValue( m_fHeight, TPS8( Height ) );

	m_fSphereRaise = 0.0f;
	a_pProperties->GetOptionalPropertyValue( m_fSphereRaise, TPS8( SphereRaise ) );

	m_bIgnoreObjectCollision = TFALSE;
	a_pProperties->GetOptionalPropertyValue( m_bIgnoreObjectCollision, TPS8( IgnoreObjectCollision ) );

	m_iCollideFlags = -1;
	a_pProperties->GetOptionalPropertyValue( m_iCollideFlags, TPS8( CollideFlags ) );

	return TTRUE;
}
