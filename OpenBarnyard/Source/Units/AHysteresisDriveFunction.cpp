#include "pch.h"
#include "AHysteresisDriveFunction.h"
#include "AInteractionSequence.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054ebe0
// NOTE: GetClass 0054ebe0, CreateObject 0054ec20, CreateObjectInPlace 0054eca0 (the constructor is inlined in both)
TDEFINE_CLASS( AHysteresisDriveFunction );

// $Barnyard: FUNCTION 0054ebf0
// $Barnyard: FUNCTION 0054ec00
AHysteresisDriveFunction::~AHysteresisDriveFunction()
{
}

// $Barnyard: FUNCTION 0054ed90
TBOOL AHysteresisDriveFunction::Create( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	a_pProperties->GetOptionalFloatOrIntValue( m_fMinRange, "minrange" );
	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRange, "maxrange" );

	// Reserve the last value
	m_iDataOffset = *a_pDataSize;
	*a_pDataSize += sizeof( TFLOAT );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054ed00
void AHysteresisDriveFunction::InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData )
{
	*(TFLOAT*)( (TUINT8*)a_pData->pData + m_iDataOffset ) = -1.0f;
}

// $Barnyard: FUNCTION 0054ed20
void AHysteresisDriveFunction::Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue )
{
	TFLOAT* pLastValue = (TFLOAT*)( (TUINT8*)a_pData->pData + m_iDataOffset );

	if ( *a_pValue < m_fMinRange || m_fMaxRange < *a_pValue )
	{
		// Outside of the range
		*pLastValue = *a_pValue;
	}
	else if ( *pLastValue < 0.0f )
	{
		*pLastValue = *a_pValue;
	}
	else
	{
		// Inside of the range, snap to the closest bound
		TFLOAT fValue;

		if ( m_fMaxRange - *a_pValue < *a_pValue - m_fMinRange )
			fValue = m_fMaxRange;
		else
			fValue = m_fMinRange;

		*a_pValue   = fValue;
		*pLastValue = fValue;
	}
}
