#include "pch.h"
#include "AGiftRequirement.h"
#include "AGift.h"
#include "AGiftComponent.h"
#include "AInteractionEventManager.h"
#include "AInteractionGiftEvent.h"
#include "AUnit.h"
#include "AUnitType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ab2e0, CreateObject 005ab850
// NOTE: GetClass 005ab2e0, CreateObject 005ab850, CreateObjectInPlace 005ab8e0
TDEFINE_CLASS( AGiftRequirement );

// $Barnyard: FUNCTION 005ab5d0
// $Barnyard: FUNCTION 005ab960
AGiftRequirement::~AGiftRequirement()
{
}

// $Barnyard: FUNCTION 005ab640
TBOOL AGiftRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pMatch = a_pProperties->GetProperty( "match" );

	if ( pMatch )
	{
		m_strMatch = pMatch->GetTPString8();

		if ( m_strMatch == TPS8D( "any" ) )
			m_bAny = TTRUE;
	}

	const PBPropertyValue* pTaker = a_pProperties->GetProperty( "taker" );

	if ( pTaker )
	{
		m_eTaker         = AInteractionSequence::GetCategoryFromName( pTaker->GetTPString8().GetString() );
		m_bTakerAndGiver = TTRUE;
	}

	if ( m_bTakerAndGiver )
		m_eGiver = AInteractionSequence::GetCategoryFromName( a_pProperties->GetProperty( "giver" )->GetTPString8().GetString() );

	const PBPropertyValue* pMinTimeHeld = a_pProperties->GetProperty( "mintimeheld" );

	if ( pMinTimeHeld )
		m_fMinTimeHeld = ( pMinTimeHeld->GetType() == PBPropertyValue::Type::Float ) ? pMinTimeHeld->GetFloat() : TFLOAT( pMinTimeHeld->GetInteger() );

	const PBPropertyValue* pMaxTimeHeld = a_pProperties->GetProperty( "maxtimeheld" );

	if ( pMaxTimeHeld )
		m_fMaxTimeHeld = ( pMaxTimeHeld->GetType() == PBPropertyValue::Type::Float ) ? pMaxTimeHeld->GetFloat() : TFLOAT( pMaxTimeHeld->GetInteger() );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ab2f0
void AGiftRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	if ( m_bTakerAndGiver )
	{
		// Nobody can take any role unless the gift is found
		TINT  iNumUnits = a_pParticipants->iNumUnits;
		TBOOL bDefault  = m_bInvert == TTRUE;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bDefault );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bDefault );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bDefault );
		}

		// NOTE: the categories are used as the indices of the units
		if ( iNumUnits <= TMath::Max( m_eTaker, m_eGiver ) )
			return;

		AUnit* pTaker = a_pParticipants->Get( m_eTaker );
		AUnit* pGiver = a_pParticipants->Get( m_eGiver );

		AInteractionEventManager*          pEventManager = AInteractionEventManager::GetSingleton();
		AInteractionEventManager::Iterator oIterator;

		for ( AInteractionGiftEvent* pEvent = TSTATICCAST( AInteractionGiftEvent, pEventManager->FindFirstEvent( &TGetClass( AInteractionGiftEvent ), oIterator ) );
		      pEvent;
		      pEvent = TSTATICCAST( AInteractionGiftEvent, pEventManager->FindNextEvent( &TGetClass( AInteractionGiftEvent ), oIterator ) ) )
		{
			TFLOAT fTime = pEvent->GetTime();

			if ( fTime < m_fMinTimeHeld || ( m_fMaxTimeHeld > 0.0f && fTime > m_fMaxTimeHeld ) )
				continue;

			// NOTE: the type names are compared by pointer (pooled strings)
			if ( pEvent->GetReceiverTypeName() == pTaker->GetType()->GetName().GetString() && pEvent->GetGiverTypeName() == pGiver->GetType()->GetName().GetString() )
			{
				SetUnitAllowed( m_eTaker, m_eTaker, TTRUE );
				SetUnitAllowed( m_eGiver, m_eGiver, TTRUE );
				return;
			}

			if ( pEvent->GetReceiverTypeName() == pGiver->GetType()->GetName().GetString() && pEvent->GetGiverTypeName() == pTaker->GetType()->GetName().GetString() )
			{
				SetUnitAllowed( m_eGiver, m_eTaker, TTRUE );
				SetUnitAllowed( m_eTaker, m_eGiver, TTRUE );
				return;
			}
		}
	}
	else
	{
		TINT iNumUnits = a_pParticipants->iNumUnits;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TBOOL           bAllowed       = TFALSE;
			AGiftComponent* pGiftComponent = TSTATICCAST( AGiftComponent, a_pParticipants->Get( i )->FindComponentExact( &TGetClass( AGiftComponent ) ) );

			if ( pGiftComponent )
			{
				AGift* pGift = pGiftComponent->GetGift();

				if ( pGift && ( pGift->GetInteraction() == m_strMatch || m_bAny ) )
				{
					TFLOAT fAge = pGift->GetAge();

					if ( fAge > m_fMinTimeHeld && ( m_fMaxTimeHeld <= 0.0f || fAge < m_fMaxTimeHeld ) )
						bAllowed = TTRUE;
				}
			}

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}
}
