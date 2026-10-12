#include "pch.h"
#include "APlayerSetActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "Player/APlayerManager.h"
#include "Input/AInputHandler.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ba9e0, CreateObject 005baa00, CreateObjectInPlace 005baa80
TDEFINE_CLASS( APlayerSetActivity );

// NOTE: inlined into CreateObject 005baa00 / CreateObjectInPlace 005baa80
APlayerSetActivity::APlayerSetActivity()
    : m_iPlayerSet( -1 )
    , m_iNumAdditionalAIPlayers( 0 )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005ba9f0
// $Barnyard: FUNCTION 005bab00
APlayerSetActivity::~APlayerSetActivity()
{
}

// $Barnyard: FUNCTION 005baba0
TBOOL APlayerSetActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strPlayerSet;
	a_pProperties->GetOptionalPropertyValue( strPlayerSet, TPS8D( "playerset" ) );
	m_iPlayerSet = APlayerManager::GetPlayerSetIndex( strPlayerSet );

	a_pProperties->GetOptionalPropertyValue( m_iNumAdditionalAIPlayers, TPS8D( "additionalaiplayers" ) );
	m_eFlags |= FLAGS_ADD_PLAYERS | FLAGS_RESET;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005baf60
AInteractionActivity::RESULT APlayerSetActivity::Update( AInteractionSequence* a_pSequence )
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	APlayerSet*     pPlayerSet     = ( m_iPlayerSet < 0 ) ? pPlayerManager->GetCurrentPlayerSet() : pPlayerManager->GetPlayerSet( m_iPlayerSet );

	if ( m_eFlags & FLAGS_ADD_PLAYERS )
	{
		if ( m_eFlags & FLAGS_RESET )
			pPlayerSet->Reset();

		AddPlayers( a_pSequence, pPlayerSet, TTRUE );
		AddPlayers( a_pSequence, pPlayerSet, TFALSE );

		// NOTE: retail's APlayerSet::AddAIPlayer (00620310) takes an unused argument, -1 here
		for ( TINT i = 0; i < m_iNumAdditionalAIPlayers; i++ )
			pPlayerSet->AddAIPlayer();

		pPlayerSet->FinishUpAddingAIs();
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005bacd0
void APlayerSetActivity::AddPlayers( AInteractionSequence* a_pSequence, APlayerSet* a_pPlayerSet, TBOOL a_bHumans )
{
	// NOTE: APlayerManager::GetPlayer (0061fae0) inlined
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerUnit( 0 );

	if ( !a_bHumans )
	{
		TINT iAIPlayer = a_pPlayerSet->GetNumAIPlayers();

		for ( TINT i = GetNumAIUnits( a_pSequence ); i > 0; i-- )
			a_pPlayerSet->AddAIPlayer();

		a_pPlayerSet->FinishUpAddingAIs();

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit != pPlayer )
			{
				APlayerSlot* pSlot   = a_pPlayerSet->GetAIPlayerSlot( iAIPlayer );
				pSlot->m_strUnitType = TPString8( pUnit->GetType()->GetName().GetString() );
				iAIPlayer++;
			}
		}
	}
	else
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit == pPlayer )
			{
				APlayerSlot* pSlot   = a_pPlayerSet->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );
				pSlot->m_strUnitType = TPString8( pUnit->GetType()->GetName().GetString() );
			}
		}
	}
}

// $Barnyard: FUNCTION 005bab20
TINT APlayerSetActivity::GetNumAIUnits( AInteractionSequence* a_pSequence )
{
	// NOTE: APlayerManager::GetPlayer (0061fae0) inlined
	AUnit* pPlayer   = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerUnit( 0 );
	TINT   iNumUnits = 0;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		if ( it->GetOwner() != pPlayer )
			iNumUnits++;
	}

	return iNumUnits;
}
