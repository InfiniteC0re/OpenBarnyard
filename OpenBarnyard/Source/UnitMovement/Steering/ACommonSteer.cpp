#include "pch.h"
#include "ACommonSteer.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AUnitCollisionComponentData.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "ASteerTraits.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACommonSteer );

// $Barnyard: FUNCTION 0055f8f0
ACommonSteer::ACommonSteer()
    : m_pNetwork( TNULL )
{
	// NOTE: m_oFlags is zeroed by TBitField's constructor
}

// $Barnyard: FUNCTION 0055f7f0
// $Barnyard: FUNCTION 0055f7e0
ACommonSteer::~ACommonSteer()
{
}

// $Barnyard: FUNCTION 0055f910
void ACommonSteer::SetInteracting( TINT a_iUnk )
{
	TBOOL bValue = TBOOL( a_iUnk );

	if ( m_oFlags.IsSet( FLAG_INTERACTING ) != bValue )
	{
		m_oFlags.Set( FLAG_INTERACTING, bValue );

		if ( !m_oFlags.IsSet( FLAG_INTERACTING ) )
		{
			SetIgnoreInvalidPosition( 0 );
			ResumeMovement();
		}
	}
}

// $Barnyard: FUNCTION 0055f960
TFLOAT ACommonSteer::GetRadius()
{
	AUnitStaticData* pCollisionData = GetParentUnit().GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) );

	if ( pCollisionData )
		return TSTATICCAST( AUnitCollisionComponentData, pCollisionData )->GetCollisionRadius();

	return m_pTraits->m_fRadius;
}

// $Barnyard: FUNCTION 0055f810
TBOOL ACommonSteer::IsInSameOrLinkedArea( AUnit* a_pUnit )
{
	AInteriorArea* pArea = GetCurrentArea();

	if ( !a_pUnit->GetSteer()->IsA( &TGetClass( ACommonSteer ) ) )
		return TTRUE;

	AInteriorArea* pOtherArea = TSTATICCAST( ACommonSteer, a_pUnit->GetSteer() )->GetCurrentArea();

	if ( !pArea || !pOtherArea )
		return TFALSE;

	if ( pArea->m_iIndex == pOtherArea->m_iIndex )
		return TTRUE;

	return pArea->FindLink( pOtherArea->m_iIndex ) != TNULL;
}
