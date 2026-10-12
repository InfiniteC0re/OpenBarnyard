#include "pch.h"
#include "AAnimalAttributeDriveFunction.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054e920
// NOTE: GetClass 0054e920, CreateObject 0054e940, CreateObjectInPlace 0054e9c0 (the constructor is inlined in both)
TDEFINE_CLASS( AAnimalAttributeDriveFunction );

// $Barnyard: FUNCTION 0054e930
// $Barnyard: FUNCTION 0054ea30
AAnimalAttributeDriveFunction::~AAnimalAttributeDriveFunction()
{
}

// $Barnyard: FUNCTION 0054ea90
TBOOL AAnimalAttributeDriveFunction::Create( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	TPString8 strAttribute;

	if ( a_pProperties->GetOptionalPropertyValue( strAttribute, "attribute" ) )
		m_eAttribute = AAnimalAttributes::GetAttributeFromName( strAttribute.GetString() );

	if ( m_eAttribute == AAnimalAttributes::ATTRIBUTE_NUMOF )
		return TFALSE;

	a_pProperties->GetOptionalPropertyValue( m_bInvert, "invert" );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054ea50
void AAnimalAttributeDriveFunction::Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue )
{
	AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, a_pUnit->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

	if ( pAttributes )
	{
		TFLOAT fValue = pAttributes->GetAttribute( m_eAttribute );
		*a_pValue     = fValue;

		if ( m_bInvert )
			*a_pValue = 1.0f - fValue;
	}
}
