#include "pch.h"
#include "AAttachActivity.h"
#include "AModelAttachComponent.h"
#include "AAnimatedUnit.h"
#include "AInventoryComponentInterface.h"
#include "AInventoryItem.h"
#include "AUnitManager.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b3670, CreateObject 005b3c90
// NOTE: GetClass 005b3670, CreateObject 005b3c90, CreateObjectInPlace 005b3d20
TDEFINE_CLASS( AAttachActivity );

TPSTRING8_DECLARE( model );
TPSTRING8_DECLARE( childbonename );
TPSTRING8_DECLARE( parentbonename );
TPSTRING8_DECLARE( swap );
TPSTRING8_DECLARE( create );
TPSTRING8_DECLARE( detach );
TPSTRING8_DECLARE( detachndestory );
TPSTRING8_DECLARE( attachinventory );
TPSTRING8_DECLARE( attachmodel );
TPSTRING8_DECLARE( detachmodel );
TPSTRING8_DECLARE( attachcharacter );
TPSTRING8_DECLARE( detachcharacter );
TPSTRING8_DECLARE( attachtolocator );

// 0x0077522c
static TPString8* s_apModeNames[ AAttachActivity::MODE_NUMOF ] = {
	&TPS8( swap ),
	&TPS8( create ),
	&TPS8( detach ),
	&TPS8( detachndestory ),
	&TPS8( attachinventory ),
	&TPS8( attachmodel ),
	&TPS8( detachmodel ),
	&TPS8( attachcharacter ),
	&TPS8( detachcharacter ),
	&TPS8( attachtolocator ),
};

AAttachActivity::AAttachActivity()
    : m_eMode( MODE_NUMOF )
    , m_eAttachCategory( AInteractionSequence::CATEGORY_TARGET )
{
}

// $Barnyard: FUNCTION 005b3680
// $Barnyard: FUNCTION 005b3da0
AAttachActivity::~AAttachActivity()
{
}

// $Barnyard: FUNCTION 005b3730
TBOOL AAttachActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pMode = a_pProperties->GetProperty( "mode" );

	if ( pMode )
	{
		TPString8 strMode = pMode->GetTPString8();

		TINT i = 0;
		for ( ; i < MODE_NUMOF; i++ )
		{
			if ( strMode == *s_apModeNames[ i ] )
				break;
		}

		m_eMode = i;
	}

	a_pProperties->GetOptionalPropertyValue( m_strModel, TPS8( model ) );
	a_pProperties->GetOptionalPropertyValue( m_strChildBone, TPS8( childbonename ) );
	a_pProperties->GetOptionalPropertyValue( m_strParentBone, TPS8( parentbonename ) );

	const PBPropertyValue* pAttachModel = a_pProperties->GetProperty( "attachmodel" );

	if ( pAttachModel )
		m_eAttachCategory = AInteractionSequence::GetCategoryFromName( pAttachModel->GetTPString8().GetString() );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b3860
AInteractionActivity::RESULT AAttachActivity::Update( AInteractionSequence* a_pSequence )
{
	switch ( m_eMode )
	{
		case MODE_SWAP:
		{
			// Moves the model of the type "model" attached to one unit to the other one
			AInteractionSequence::CATEGORY eOtherCategory = m_eAttachCategory;

			if ( eOtherCategory == 8 )
				eOtherCategory = ( m_eCategory != 1 );

			AUnit* pFromUnit = a_pSequence->GetUnitByCategory( m_eCategory );
			AUnit* pToUnit   = a_pSequence->GetUnitByCategory( eOtherCategory );

			if ( m_eCategory != 0 )
				std::swap( pFromUnit, pToUnit );

			AAnimatableObject* pFromObject = TSTATICCAST( AAnimatedUnit, pFromUnit )->GetAnimatableObject();
			AAnimatableObject* pToObject   = TSTATICCAST( AAnimatedUnit, pToUnit )->GetAnimatableObject();
			AAnimatableObject* pModel      = pFromObject->FindAttachedObjectOfType( m_strModel );

			if ( pModel )
			{
				AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
				pManager->DetachObject( pModel, pFromObject );
				pManager->AttachObject( pModel, pToObject, m_strChildBone, m_strParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			}

			break;
		}
		break;
		case MODE_CREATE:
		{
			// Creates "model" and attaches it to the unit
			AAnimatableObject*        pParent  = TSTATICCAST( AAnimatedUnit, a_pSequence->GetUnitByCategory( m_eCategory ) )->GetAnimatableObject();
			AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pModel   = pManager->CreateAnimatableObject( pManager->FindType( m_strModel ), TNULL, 0 );

			// NOTE: retail uses the object before checking it
			pModel->SetSkeletonUpdating( TTRUE, TTRUE );
			pModel->SetVisible( TFALSE );

			if ( pModel )
				AAnimatableObjectManager::GetSingleton()->AttachObject( pModel, pParent, m_strChildBone, m_strParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

			break;
		}
		break;
		case MODE_DETACH:
		case MODE_DETACHNDESTROY:
		{
			// Destroys the model attached to the bone "parentbonename" (or the one of the type "model")
			AAnimatableObject*        pParent  = TSTATICCAST( AAnimatedUnit, a_pSequence->GetUnitByCategory( m_eCategory ) )->GetAnimatableObject();
			AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pModel   = pManager->GetAttachedObjectByBoneName( pParent, m_strParentBone, 0 );

			if ( pModel || ( pModel = pParent->FindAttachedObjectOfType( m_strModel ) ) )
			{
				pManager->DetachObject( pModel, pParent );
				pManager->DestroyAnimatableObject( pModel );
			}

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: retail reads the owner of the model even if there's none
			if ( m_eMode == MODE_DETACHNDESTROY && pModel )
#else
			if ( m_eMode == MODE_DETACHNDESTROY )
#endif
				AUnitManager::GetSingleton()->DestroyUnit( TSTATICCAST( AUnit, pModel->GetOwner() ) );

			break;
		}
		break;
		case MODE_ATTACHINVENTORY:
		{
			// Attaches the unit of the item of the type "model" to the unit
			AAnimatedUnit*  pUnit = TSTATICCAST( AAnimatedUnit, a_pSequence->GetUnitByCategory( m_eCategory ) );
			AInventoryItem* pItem = AInventoryComponentInterface::GetInventory( pUnit )->FindItemByType( m_strModel );

			pUnit->AttachUnitToBone( pItem->GetUnit(), m_strParentBone, m_strChildBone );
			break;
		}
		case MODE_ATTACHMODEL:
		{
			// Attaches the model "model" owned by the unit's AModelAttachComponent
			AUnit*                 pUnit      = a_pSequence->GetUnitByCategory( m_eCategory );
			AModelAttachComponent* pComponent = TSTATICCAST( AModelAttachComponent, pUnit->FindComponent( &TGetClass( AModelAttachComponent ) ) );

			pComponent->AttachModel( m_strModel, m_strParentBone, m_strChildBone );
			break;
		}
		case MODE_DETACHMODEL:
		{
			// Detaches the model "model" owned by the unit's AModelAttachComponent
			AUnit*                 pUnit      = a_pSequence->GetUnitByCategory( m_eCategory );
			AModelAttachComponent* pComponent = TSTATICCAST( AModelAttachComponent, pUnit->FindComponent( &TGetClass( AModelAttachComponent ) ) );

			pComponent->DetachModel( m_strModel );
			break;
		}
		case MODE_ATTACHCHARACTER:
		{
			// Attaches the unit to the bone "parentbonename" of the other unit
			AUnit*         pUnit   = a_pSequence->GetUnitByCategory( m_eCategory );
			AAnimatedUnit* pHolder = TSTATICCAST( AAnimatedUnit, a_pSequence->GetUnitByCategory( m_eAttachCategory ) );

			pHolder->AttachUnit( pUnit, m_strParentBone );
			pUnit->SetNoQuery( TTRUE );
			break;
		}
		break;
		case MODE_DETACHCHARACTER:
		{
			// Detaches the unit from the unit it is attached to
			AUnit*             pUnit   = a_pSequence->GetUnitByCategory( m_eCategory );
			AAnimatableObject* pParent = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetParentObject();

			if ( pParent )
			{
				TSTATICCAST( AAnimatedUnit, pParent->GetOwner() )->DetachUnit( pUnit, TTRUE, TTRUE );
				pUnit->SetNoQuery( TFALSE );
				pUnit->SetHashed( TTRUE );
				pUnit->SetMovement( TTRUE );
			}

			break;
		}
		break;
		default:
			// MODE_ATTACHTOLOCATOR (and MODE_NUMOF)
			{
				// Moves the unit to the terrain locator "parentbonename"
				AUnit* pUnit = a_pSequence->GetUnitByCategory( m_eCategory );
				pUnit->SetNoQuery( TFALSE );

				TMatrix44 matLocator;
				ATerrainInterface::GetSingleton()->GetLocatorTransform( m_strParentBone.GetString(), matLocator );

				TVector4 vPosition  = matLocator.GetTranslation();
				TVector4 vDirection = matLocator.AsBasisVector4( BASISVECTOR_UP );

				pUnit->GetSteer()->SetPosition( vPosition );
				pUnit->SetSteerDirection( vDirection );
				break;
			}
			break;
	}

	return RESULT_DONE;
}
