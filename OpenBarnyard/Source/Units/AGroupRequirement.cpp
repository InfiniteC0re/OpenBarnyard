#include "pch.h"
#include "AGroupRequirement.h"
#include "AInteractionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005abc20, CreateObject 005abe60, CreateObjectInPlace 005abec0
TDEFINE_CLASS( AGroupRequirement );

TPSTRING8_DECLARE( boolean );
TPSTRING8_DECLARE( requirements );

// $Barnyard: FUNCTION 005abc30
AGroupRequirement::AGroupRequirement()
    : m_vecRequirements( GetGlobalAllocator(), 8, 8 )
    , m_eBoolean( BOOLEAN_AND )
{
}

// $Barnyard: FUNCTION 005abc90
// $Barnyard: FUNCTION 005abf00
AGroupRequirement::~AGroupRequirement()
{
}

// $Barnyard: FUNCTION 005abf20
TBOOL AGroupRequirement::Create( const PBProperties* a_pProperties )
{
	AInteractionManager* pManager = AInteractionManager::GetSingleton();

	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		TPString8              strName( it->GetName().GetString() );
		const PBPropertyValue* pValue = it->GetValue();

		if ( strName == TPS8( boolean ) )
		{
			const TCHAR* szValue = pValue->GetString();

			if ( szValue[ 0 ] == 'o' && szValue[ 1 ] == 'r' )
				m_eBoolean = BOOLEAN_OR;
		}
		else if ( strName == TPS8( requirements ) )
		{
			if ( pValue->GetType() == PBPropertyValue::Type::Array )
			{
				const PBPropertyValueArray* pArray = pValue->GetArray();

				for ( TUINT i = 0; i < pArray->GetSize(); i++ )
					m_vecRequirements.PushBack( pManager->FindOrCreateRequirement( pArray->GetValue( i )->GetTPString8() ) );
			}
			else if ( pValue->GetType() == PBPropertyValue::Type::String )
			{
				m_vecRequirements.PushBack( pManager->FindOrCreateRequirement( pValue->GetTPString8() ) );
			}
		}
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005abd00
void AGroupRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	if ( m_eBoolean == BOOLEAN_OR )
		m_oRoles.Clear();

	AInteractionManager* pManager = AInteractionManager::GetSingleton();

	T2_FOREACH( m_vecRequirements, it )
	{
		AInteractionRequirement* pRequirement = pManager->GetRequirement( *it );

		// Evaluate once for every set of participants (same as AInteraction::EvaluateRequirements)
		if ( pRequirement->m_uiParticipantsId != a_pParticipants->uiId )
		{
			pRequirement->m_oRoles.Reset();
			pRequirement->m_uiParticipantsId = a_pParticipants->uiId;
			pRequirement->Evaluate( a_pParticipants );
		}

		if ( pRequirement->m_eForcedState == FORCEDSTATE_SATISFIED )
		{
			pRequirement->m_oRoles.Reset();
			pRequirement->m_oRoles.fWeight = 1.0f;
		}
		else if ( pRequirement->m_eForcedState == FORCEDSTATE_NOT_SATISFIED )
		{
			pRequirement->m_oRoles.Clear();
			pRequirement->m_oRoles.fWeight = 0.0f;
		}

		if ( m_eBoolean == BOOLEAN_OR )
			m_oRoles.Merge( pRequirement->m_oRoles );
		else if ( m_eBoolean == BOOLEAN_AND )
			m_oRoles.Combine( pRequirement->m_oRoles );
	}

	if ( m_bInvert == TTRUE )
	{
		// Only the masks are inverted, not the weight
		for ( TINT i = 0; i < AInteractionRoles::NUM_ROLES; i++ )
		{
			for ( TINT k = 0; k < AInteractionRoles::NUM_WORDS; k++ )
				m_oRoles.aauiMasks[ i ][ k ] = ~m_oRoles.aauiMasks[ i ][ k ];
		}
	}
}
