#include "pch.h"
#include "AMakeInstanceActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AVisibleUnit.h"
#include "AUnitManager.h"
#include "AInteractionSequence.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/TStringManager.h>

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/ARegrowthManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ba190, CreateObject 005ba1d0, CreateObjectInPlace 005ba260
TDEFINE_CLASS( AMakeInstanceActivity );

TPSTRING8_DECLARE( maxradius );

// NOTE: inlined into CreateObject 005ba1d0 / CreateObjectInPlace 005ba260
AMakeInstanceActivity::AMakeInstanceActivity()
    : m_eFlags( FLAGS_MAKE | FLAGS_REGROW )
    , m_fMaxRadius( 1.7f )
{
	m_auiTypes[ 0 ] = 0;
	m_auiTypes[ 1 ] = 0;
}

// $Barnyard: FUNCTION 005ba1a0
// $Barnyard: FUNCTION 005ba1b0
AMakeInstanceActivity::~AMakeInstanceActivity()
{
}

// $Barnyard: FUNCTION 005ba2e0
TBOOL AMakeInstanceActivity::Create( const PBProperties* a_pProperties )
{

	FLAGS eFlags = FLAGS_NONE;
	TBOOL bValue = TFALSE;

	if ( a_pProperties->GetOptionalPropertyValue( bValue, TPS8D( "make" ) ) && bValue == TTRUE )
		eFlags = FLAGS_MAKE;

	if ( a_pProperties->GetOptionalPropertyValue( bValue, TPS8D( "regrow" ) ) && bValue == TTRUE )
		eFlags |= FLAGS_REGROW;

	if ( a_pProperties->GetOptionalPropertyValue( bValue, TPS8D( "destroy" ) ) && bValue == TTRUE )
		eFlags |= FLAGS_DESTROY;

	if ( eFlags != FLAGS_NONE )
		m_eFlags = eFlags;

	TPString8 strType;

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		if ( TStringManager::String8CompareNoCase( it->GetName().GetString(), "type" ) == 0 )
		{
			strType = it->GetValue()->GetTPString8();

			// "fruit", "crate", "light", "heavy", "milk", "dayrespawn", "non_trees" or a type of ARegrowthManager
			ARegrowthManager::TypeMask oMask = ARegrowthManager::TypeMask::FromName( strType );
			TREINTERPRETCAST( ARegrowthManager::TypeMask&, m_auiTypes ) |= oMask;
		}
	}

	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8( maxradius ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ba540
AInteractionActivity::RESULT AMakeInstanceActivity::Update( AInteractionSequence* a_pSequence )
{

	AUnit*                          pSpawnedUnit = TNULL;
	ARegrowthManager::GrowthObject* pObject      = TNULL;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( !pUnit )
			continue;

		// The unit the previous object was made into is in the category too
		if ( pSpawnedUnit == pUnit )
			break;

		ASteer*  pSteer = pUnit->GetSteer();
		TVector2 vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
		TINT     iType;

		// The object in front of the unit
		pObject = ARegrowthManager::GetSingleton()->FindObjectInFront( *pSteer->GetPosition(), vecDirection, TREINTERPRETCAST( const ARegrowthManager::TypeMask&, m_auiTypes ), iType, m_fMaxRadius, 0.8f );

		if ( !pObject )
			continue;

		if ( m_eFlags & FLAGS_REGROW )
		{
			ARegrowthManager::GetSingleton()->TakeObject( iType, pObject );
			ARegrowthManager::GetSingleton()->StartGrowing( iType, pObject, 0.0f );
		}
		else if ( m_eFlags & FLAGS_DESTROY )
		{
			ARegrowthManager::GetSingleton()->TakeObject( iType, pObject );
		}

		if ( m_eFlags & FLAGS_MAKE )
		{
			// Spawn the unit of the object in its place
			pSpawnedUnit = AUnitManager::GetSingleton()->SpawnUnit( ARegrowthManager::GetSingleton()->GetUnitType( iType ), pObject->vecPosition, TVector4::VEC_POSX );

			AVisibleUnit* pVisibleUnit = TSTATICCAST( AVisibleUnit, pSpawnedUnit );
			pVisibleUnit->GetModelInstance()->SetTransform( pObject->vecPosition, pObject->quatRotation );

			TMatrix44 matTransform;
			pVisibleUnit->GetModelInstance()->GetLocalTransform( matTransform );

			// NOTE: retail leaves w uninitialised
			TVector4 vecUnitDirection( -matTransform.m_f21, -matTransform.m_f22, -matTransform.m_f23, 1.0f );
			pSpawnedUnit->GetSteer()->SetDirection( vecUnitDirection );

			if ( pVisibleUnit->GetCollisionObject() )
				pVisibleUnit->GetCollisionObject()->SetType( ACollisionObject::TYPE_SIMPLE );

			a_pSequence->AddUnit( pSpawnedUnit );
		}
	}

	return pObject ? RESULT_DONE : RESULT_FINISH_SEQUENCE;
}
