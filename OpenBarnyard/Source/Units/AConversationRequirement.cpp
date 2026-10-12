#include "pch.h"
#include "AConversationRequirement.h"
#include "AInteractionComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a9170, CreateObject 005a91f0, CreateObjectInPlace 005a9250
TDEFINE_CLASS( AConversationRequirement );

TPSTRING8_DECLARE( action );
TPSTRING8_DECLARE( lastplayerinteraction );
TPSTRING8_DECLARE( lastplayerinteractionfreq );
TPSTRING8_DECLARE( compare );
TPSTRING8_DECLARE( time );
TPSTRING8_DECLARE( timespan );

// $Barnyard: FUNCTION 005a9180
AConversationRequirement::AConversationRequirement()
    : m_uiFlags( FLAGS_NONE )
    , m_fTimeSpan( 0.0f )
    , m_fTime( 0.0f )
    , m_oCompare( AInteractionComparison::OPERATOR_ALL )
{
}

// $Barnyard: FUNCTION 005a91e0
// $Barnyard: FUNCTION 005a9290
AConversationRequirement::~AConversationRequirement()
{
}

// $Barnyard: FUNCTION 005a93a0
TBOOL AConversationRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strValue;
	a_pProperties->GetOptionalPropertyValue( strValue, TPS8( action ) );

	if ( strValue == TPS8( lastplayerinteraction ) )
		m_uiFlags |= FLAGS_LAST_INTERACTION;
	else if ( strValue == TPS8( lastplayerinteractionfreq ) )
		m_uiFlags |= FLAGS_LAST_INTERACTION_FREQ;

	if ( m_uiFlags & ( FLAGS_LAST_INTERACTION | FLAGS_LAST_INTERACTION_FREQ ) )
	{
		if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8( compare ) ) )
			m_oCompare.SetOperator( strValue );

		a_pProperties->GetOptionalFloatOrIntValue( m_fTime, TPS8( time ) );

		if ( m_uiFlags & FLAGS_LAST_INTERACTION_FREQ )
			a_pProperties->GetOptionalFloatOrIntValue( m_fTimeSpan, TPS8( timespan ) );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a92b0
void AConversationRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	if ( m_uiFlags & FLAGS_LAST_INTERACTION )
	{
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AInteractionComponent ) ) );
			TBOOL                  bAllowed              = TFALSE;

			if ( pInteractionComponent )
				bAllowed = m_oCompare.Compare( pInteractionComponent->GetInterval(), m_fTime );

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}
	else if ( m_uiFlags & FLAGS_LAST_INTERACTION_FREQ )
	{
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AInteractionComponent* pInteractionComponent = TSTATICCAST( AInteractionComponent, a_pParticipants->Get( i )->FindComponent( &TGetClass( AInteractionComponent ) ) );
			TBOOL                  bAllowed              = TFALSE;

			if ( pInteractionComponent )
				bAllowed = m_oCompare.Compare( TFLOAT( pInteractionComponent->GetNumInteractionsWithin( m_fTimeSpan ) ), m_fTime );

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}
}
