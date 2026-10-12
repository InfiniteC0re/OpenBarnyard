#include "pch.h"
#include "ATreasureRequirement.h"
#include "Helpers/ATreasureManager.h"
#include "Helpers/ARegrowthManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b13a0, CreateObject 005b1410
// NOTE: GetClass 005b13a0, CreateObject 005b1410, CreateObjectInPlace 005b1470
TDEFINE_CLASS( ATreasureRequirement );

// $Barnyard: FUNCTION 005b13b0
ATreasureRequirement::ATreasureRequirement()
    : m_iAmount( 0 )
    , m_oCompare( AInteractionComparison::OPERATOR_ALL )
{
}

// $Barnyard: FUNCTION 005b1400
// $Barnyard: FUNCTION 005b14b0
ATreasureRequirement::~ATreasureRequirement()
{
}

// $Barnyard: FUNCTION 005b15a0
TBOOL ATreasureRequirement::Create( const PBProperties* a_pProperties )
{
	m_iAmount = 0;
	a_pProperties->GetOptionalPropertyValue( m_iAmount, TPS8D( "totalcreated" ) );

	if ( m_iAmount <= 0 )
	{
		a_pProperties->GetOptionalPropertyValue( m_iAmount, TPS8D( "currentamount" ) );

		if ( m_iAmount <= 0 )
		{
			TBOOL bEgg;

			if ( a_pProperties->GetOptionalPropertyValue( bEgg, TPS8D( "egg" ) ) && bEgg )
				m_eMode = MODE_EGG;
		}
		else
		{
			m_eMode = MODE_CURRENT_AMOUNT;
		}
	}
	else
	{
		m_eMode = MODE_TOTAL_CREATED;
	}

	if ( m_eMode != MODE_EGG )
	{
		TPString8 strCompare;

		if ( a_pProperties->GetOptionalPropertyValue( strCompare, TPS8D( "compare" ) ) )
			m_oCompare.SetOperator( strCompare );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b14d0
void ATreasureRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bAllowed = TFALSE;

	if ( m_eMode == MODE_TOTAL_CREATED )
	{
		bAllowed = m_oCompare.Compare( ATreasureManager::GetSingleton()->GetNumAdded( ATreasureManager::TYPE_FLOWERS ), m_iAmount );
	}
	else if ( m_eMode == MODE_CURRENT_AMOUNT )
	{
		bAllowed = m_oCompare.Compare( ATreasureManager::GetSingleton()->CountType( ATreasureManager::TYPE_FLOWERS ), m_iAmount );
	}
	else if ( m_eMode == MODE_EGG )
	{
		bAllowed = ARegrowthManager::GetSingleton()->HasRespawned();
	}

	// NOTE: every role is set, "category" is ignored
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		TBOOL bValue = ( m_bInvert == TTRUE ) ? !bAllowed : bAllowed;

		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bValue );
		m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bValue );
	}
}
