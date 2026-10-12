#include "pch.h"
#include "AGift_DeprecatedActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AAnimalAttributes.h"
#include "AUnitBehaviour.h"
#include "AGift.h"
#include "AGiftComponent.h"
#include "AInteractionGiftEvent.h"
#include "AInteractionEventManager.h"
#include "AInteractionSequence.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b70a0, CreateObject 005b74f0, CreateObjectInPlace 005b7570
TDEFINE_CLASS( AGift_DeprecatedActivity );

// NOTE: inlined into CreateObject
AGift_DeprecatedActivity::AGift_DeprecatedActivity()
    : m_bSwap( TFALSE )
    , m_bRemove( TFALSE )
    , m_bSuggestFsm( TFALSE )
{
}

// $Barnyard: FUNCTION 005b70b0
// $Barnyard: FUNCTION 005b75f0
AGift_DeprecatedActivity::~AGift_DeprecatedActivity()
{
}

// $Barnyard: FUNCTION 005b7180
TBOOL AGift_DeprecatedActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pObject = a_pProperties->GetProperty( "object" );

	if ( pObject )
		m_strObject = pObject->GetTPString8();

	a_pProperties->GetOptionalPropertyValue( m_bSwap, TPS8D( "swap" ) );

	const PBPropertyValue* pRemove = a_pProperties->GetProperty( "remove" );

	if ( pRemove )
		m_bRemove = pRemove->GetRaw() != TNULL;

	const PBPropertyValue* pSuggestFsm = a_pProperties->GetProperty( "suggestfsm" );

	if ( pSuggestFsm )
	{
		m_strSuggestFsm = pSuggestFsm->GetTPString8();
		m_bSuggestFsm   = TTRUE;
	}

	if ( m_bSuggestFsm )
	{
		// NOTE: retail doesn't check if the triggers are set
		m_strForceTrigger    = a_pProperties->GetProperty( "forcetrigger" )->GetTPString8();
		m_strOverrideTrigger = a_pProperties->GetProperty( "overridetrigger" )->GetTPString8();
	}

	return AInteractionActivityBoolean::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b7610
AInteractionActivity::RESULT AGift_DeprecatedActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( m_bRemove )
	{
		// Throws the gift of the unit away
		AUnit* pUnit = a_pSequence->GetUnit( m_eCategory );

		if ( pUnit )
		{
			AGiftComponent* pGiftComponent = TSTATICCAST( AGiftComponent, pUnit->FindComponentExact( &TGetClass( AGiftComponent ) ) );

			if ( pGiftComponent )
			{
				AGift* pGift = pGiftComponent->TakeGift();

				if ( pGift )
					delete pGift;
			}
		}
	}
	else if ( m_bSwap )
	{
		// The giver passes its gift (or a new one) to the receiver
		AUnit* pGiver    = a_pSequence->GetUnit( 0 );
		AUnit* pReceiver = a_pSequence->GetUnit( 1 );

		if ( m_eCategory != 0 )
		{
			AUnit* pTemp = pGiver;
			pGiver       = pReceiver;
			pReceiver    = pTemp;
		}

		AGift* pGift = TSTATICCAST( AGiftComponent, pGiver->FindComponentExact( &TGetClass( AGiftComponent ) ) )->TakeGift();

		if ( !pGift )
			pGift = new AGift( m_strObject );

		if ( TSTATICCAST( AGiftComponent, pReceiver->FindComponentExact( &TGetClass( AGiftComponent ) ) )->GiveGift( pGift ) == TTRUE )
		{
			AInteractionGiftEvent* pEvent = TSTATICCAST( AInteractionGiftEvent, AInteractionEventManager::GetSingleton()->NewActiveEvent( &TGetClass( AInteractionGiftEvent ) ) );
			pEvent->SetGiver( pGiver );
			pEvent->SetReceiver( pReceiver );
			pEvent->SetGiftType( pGift->GetInteraction() );

			if ( m_bSuggestFsm )
				SuggestFsm( pReceiver );

			UpdateTrue( a_pSequence );
		}
		else
		{
			if ( pGift )
				delete pGift;

			UpdateFalse( a_pSequence );
		}
	}
	else
	{
		// Every unit of the category gets a gift
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AGiftComponent* pGiftComponent = TSTATICCAST( AGiftComponent, it->GetOwner()->FindComponentExact( &TGetClass( AGiftComponent ) ) );

			if ( pGiftComponent )
				pGiftComponent->GiveGift( new AGift( m_strObject ) );
		}
	}

	return AInteractionActivity::Update( a_pSequence );
}

// $Barnyard: FUNCTION 005b7400
void AGift_DeprecatedActivity::SuggestFsm( AUnit* a_pUnit )
{
	AAnimalAttributes::ATTRIBUTE eSuggestion = AAnimalAttributes::GetAttributeFromName( TPS8D( "suggestion" ).GetString() );
	AAnimalAttributes*           pAttributes = TSTATICCAST( AAnimalAttributes, a_pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Retail expects the unit to have attributes
	if ( pAttributes )
#endif
		pAttributes->SetAttribute( eSuggestion, 1.0f );

	AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, a_pUnit->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

	if ( pBehaviour )
	{
		pBehaviour->SetSuggestion( m_strSuggestFsm, TNULL, TFALSE );
		pBehaviour->ResetDriveTimer();
	}

	AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, a_pUnit->FindComponentExact( &TGetClass( AInteractionComponent ) ) );
	pInteraction->SetForcedInteraction( m_strForceTrigger, m_strOverrideTrigger, 360.0f );
}
