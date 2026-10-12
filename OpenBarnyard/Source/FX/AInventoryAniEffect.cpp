#include "pch.h"
#include "AInventoryAniEffect.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AUnitManager.h"
#include "Units/AVisibleUnit.h"
#include "Units/AObjectTraits.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectType.h"
#include "Player/APlayerProfileManager.h"
#include "Cameras/ACameraManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00631060, CreateObject 00631120, CreateObjectInPlace 006311a0 (the constructor is inlined in both)
TDEFINE_CLASS( AInventoryAniEffect );

TPSTRING8_DECLARE( barnbuck );
TPSTRING8_DECLARE( car_coin );
TPSTRING8_DECLARE( precisioncoin );

AInventoryAniEffect::AInventoryAniEffect()
    : m_bTakeOut( TFALSE )
    , m_pObject( TNULL )
    , m_iCount( 0 )
    , m_pCallBack( TNULL )
{
}

// $Barnyard: FUNCTION 00631220
// $Barnyard: FUNCTION 00631080
AInventoryAniEffect::~AInventoryAniEffect()
{
}

// NOTE: shared with ADieAniEffect::GetUnit
// $Barnyard: FUNCTION 00631070
AUnit* AInventoryAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00631380
TBOOL AInventoryAniEffect::Update( TFLOAT a_fDeltaTime )
{
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	if ( m_pObject )
	{
		// An object of a mini game
		const TPString8& rcTypeName = m_pObject->GetObjectType()->GetName();

		if ( rcTypeName == TPS8( barnbuck ) || rcTypeName == TPS8( car_coin ) )
		{
			if ( m_iCount > 0 )
				pProfile->AddMiniGamePickups( m_iCount );
			else
				pProfile->AddPickupsToTarget( 1 );

			m_pObject->SetSkeletonUpdating( TFALSE, TTRUE );

			if ( m_pCallBack )
				m_pCallBack->OnObjectCollected( m_pObject );
		}
	}
	else if ( m_oCollector && m_oCollector->GetUnit() && m_oUnit && m_oUnit->GetUnit() )
	{
		AUnit*           pUnit      = m_oUnit ? m_oUnit->GetUnit() : TNULL;
		const TPString8& rcTypeName = pUnit->GetType()->GetName();

		// Pick ups go to the counter
		TINT iPickups = 0;

		if ( rcTypeName == TPS8D( "buck_1" ) )
			iPickups = 1;
		else if ( rcTypeName == TPS8D( "buck_5" ) )
			iPickups = 5;
		else if ( rcTypeName == TPS8D( "buck_20" ) )
			iPickups = 20;

		if ( iPickups != 0 )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );

			if ( m_iCount > 0 )
				pProfile->AddMiniGamePickups( iPickups );
			else
				pProfile->AddPickupsToTarget( iPickups );

			AUnitManager::GetSingleton()->DestroyUnit( m_oUnit->GetUnit() );
		}
		else if ( rcTypeName == TPS8( precisioncoin ) )
		{
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );

			if ( m_iCount > 0 )
				pProfile->AddMiniGamePickups( 1 );
			else
				pProfile->AddPickupsToTarget( 1 );

			TSTATICCAST( AVisibleUnit, m_oUnit->GetUnit() )->SetVisible( TFALSE );
		}
		else
		{
			// Everything else goes to the inventory of the collector
			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( m_oCollector ? m_oCollector->GetUnit() : TNULL ) );
			AObjectTraits*            pTraits    = TSTATICCAST( AObjectTraits, pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_APPLE );

			if ( !pTraits->m_strInventoryForStore.IsEmpty() )
			{
				AInventoryComponent* pCategory;

				if ( m_bTakeOut )
				{
					pCategory = pInventory->GetCategory( pTraits->m_strInventoryForUse );

					AUnit* pCollector = m_oCollector ? m_oCollector->GetUnit() : TNULL;

					if ( pCollector->IsPlayer() )
					{
						APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, ( m_oCollector ? m_oCollector->GetUnit() : TNULL )->FindComponent( &TGetClass( APlayerControlComponent ) ) );

						if ( pControlComponent )
							pControlComponent->SetEnabled( TTRUE );
					}
				}
				else
				{
					pCategory = pInventory->GetCategory( pTraits->m_strInventoryForStore );
				}

				pCategory->AddUnit( pUnit, 1, AInventoryComponentInterface::ms_strDefaultInventory );
			}
		}
	}

	if ( !ACameraManager::GetSingleton()->IsActive() )
		ACameraManager::GetSingleton()->Activate( TTRUE );

	Finish();
	return TTRUE;
}

// $Barnyard: FUNCTION 00631240
void AInventoryAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit = a_pUnit->GetSelfProxy();
	AUnitManager::GetSingleton()->SetUnitPoolUsed( m_oUnit->GetUnit(), TFALSE );
}

// $Barnyard: FUNCTION 006312f0
void AInventoryAniEffect::SetCollector( AUnit* a_pCollector )
{
	m_oCollector = a_pCollector->GetSelfProxy();
}
