#include "pch.h"
#include "ADriveRequirement.h"
#include "AAnimalAttributes.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a94a0, CreateObject 005a94c0
// NOTE: GetClass 005a94a0, CreateObject 005a94c0, CreateObjectInPlace 005a9550
TDEFINE_CLASS( ADriveRequirement );

// $Barnyard: FUNCTION 005a94b0
// $Barnyard: FUNCTION 005a95d0
ADriveRequirement::~ADriveRequirement()
{
}

// $Barnyard: FUNCTION 005a9690
TBOOL ADriveRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pAttribute = a_pProperties->GetProperty( "attribute" );

	if ( pAttribute )
	{
		m_eAttribute = AAnimalAttributes::GetAttributeFromName( pAttribute->GetTPString8().GetString() );

		if ( m_eAttribute == AAnimalAttributes::ATTRIBUTE_NUMOF )
			m_eAttribute = -1;
	}

	const PBPropertyValue* pRange = a_pProperties->GetProperty( "range" );

	if ( pRange )
	{
		const PBPropertyValueArray* pArray = pRange->GetArray();
		const PBPropertyValue*      pMin   = pArray->GetValue( 0 );
		const PBPropertyValue*      pMax   = pArray->GetValue( 1 );

		m_fMin = ( pMin->GetType() == PBPropertyValue::Type::Float ) ? pMin->GetFloat() : TFLOAT( pMin->GetInteger() );
		m_fMax = ( pMax->GetType() == PBPropertyValue::Type::Float ) ? pMax->GetFloat() : TFLOAT( pMax->GetInteger() );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a95f0
void ADriveRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	if ( m_eAttribute < 0 )
		return;

	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit = a_pParticipants->Get( i );

		if ( pUnit->IsPlayer() )
			continue;

		TBOOL              bAllowed    = TFALSE;
		AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

		if ( pAttributes )
		{
			TFLOAT fValue = pAttributes->GetAttribute( m_eAttribute );
			bAllowed      = fValue >= m_fMin && fValue <= m_fMax;
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
