#include "pch.h"
#include "AWaitActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "ARandom.h"
#include "Quests/AQuestManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c28c0, CreateObject 005c2dc0
// NOTE: GetClass 005c28c0, CreateObject 005c2dc0, CreateObjectInPlace 005c2e40
TDEFINE_CLASS( AWaitActivity );

TPSTRING8_DECLARE( quest );
TPSTRING8_DECLARE( tier );

// NOTE: retail leaves m_fPeriod, m_eQuestState and m_uiTier uninitialised
AWaitActivity::AWaitActivity()
{
}

// $Barnyard: FUNCTION 005c2a30
// $Barnyard: FUNCTION 005c2eb0
AWaitActivity::~AWaitActivity()
{
}

// $Barnyard: FUNCTION 005c2ad0
TBOOL AWaitActivity::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strQuest, TPS8( quest ) );
	a_pProperties->GetOptionalPropertyValue( m_strUnitExempt, TPS8D( "unitexempt" ) );

	const PBPropertyValue* pPeriod = a_pProperties->GetProperty( "period" );

	if ( pPeriod )
	{
		if ( pPeriod->GetType() == PBPropertyValue::Type::Float )
			m_fPeriod = pPeriod->GetFloat();
		else
			m_fPeriod = TFLOAT( pPeriod->GetInteger() );
	}
	else if ( m_strQuest.IsEmpty() )
	{
		const PBPropertyValue* pRandom = a_pProperties->GetProperty( "random" );

		if ( !pRandom )
			return TFALSE;

		// A random period in the range
		const PBPropertyValueArray* pRange = pRandom->GetArray();
		const PBPropertyValue*      pMin   = pRange->GetValue( 0 );
		const PBPropertyValue*      pMax   = pRange->GetValue( 1 );

		TFLOAT fMin = ( pMin->GetType() == PBPropertyValue::Type::Float ) ? pMin->GetFloat() : TFLOAT( pMin->GetInteger() );
		TFLOAT fMax = ( pMax->GetType() == PBPropertyValue::Type::Float ) ? pMax->GetFloat() : TFLOAT( pMax->GetInteger() );

		m_fPeriod = TMath::LERP( fMin, fMax, ARandom::GetSingleton()->m_oRandom.GetFloat() );
	}
	else
	{
		m_fPeriod = 0.0f;
	}

	if ( !m_strQuest.IsEmpty() )
	{
		TPString8 strQuestState;
		a_pProperties->GetOptionalPropertyValue( strQuestState, TPS8D( "queststate" ) );

		const TCHAR* szQuestState = strQuestState.GetString();

		if ( TStringManager::String8CompareNoCase( szQuestState, "complete" ) == 0 )
			m_eQuestState = QUESTSTATE_COMPLETE;
		else if ( TStringManager::String8CompareNoCase( szQuestState, "fail" ) == 0 )
			m_eQuestState = QUESTSTATE_FAIL;
		else if ( TStringManager::String8CompareNoCase( szQuestState, "assigned" ) == 0 )
			m_eQuestState = QUESTSTATE_ASSIGNED;
	}

	TINT iTier = TIER_ANY;
	a_pProperties->GetOptionalPropertyValue( iTier, TPS8( tier ) );
	m_uiTier = TUINT8( iTier );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c28d0
AInteractionActivity::RESULT AWaitActivity::Update( AInteractionSequence* a_pSequence )
{
	// Units of the exempt type end the wait
	if ( !m_strUnitExempt.IsEmpty() )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			if ( TStringManager::String8CompareNoCase( it->GetOwner()->GetType()->GetName().GetString(), m_strUnitExempt.GetString() ) == 0 )
				return RESULT_DONE;
		}
	}

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	// The wait only happens in the tier
	if ( m_uiTier != TIER_ANY )
	{
		if ( m_uiTier != pQuestManager->GetCurAbsQuestTier() )
			return RESULT_DONE;
	}

	TFLOAT fTime = a_pSequence->GetActivityState().fTime;

	if ( !m_strQuest.IsEmpty() )
	{
		if ( fTime < m_fPeriod )
			return RESULT_RUNNING;

		if ( m_eQuestState == QUESTSTATE_COMPLETE )
		{
			return pQuestManager->IsQuestComplete( m_strQuest, 1 ) ? RESULT_DONE : RESULT_RUNNING;
		}

		if ( m_eQuestState == QUESTSTATE_ASSIGNED )
		{
			return pQuestManager->FindAssignedQuest( m_strQuest, 1 ) ? RESULT_DONE : RESULT_RUNNING;
		}

		return ( m_eQuestState == QUESTSTATE_FAIL ) ? RESULT_DONE : RESULT_RUNNING;
	}

	return ( fTime < m_fPeriod ) ? RESULT_RUNNING : RESULT_DONE;
}
