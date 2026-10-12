#include "pch.h"
#include "ADrive.h"
#include "ADriveFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054d8a0, retail registers RETURNS_ZERO as both create functions and an alignment of 1
TDEFINE_CLASS_NORUNTIME( ADrive );

// $Barnyard: FUNCTION 0054d9b0
ADrive::ADrive()
    : m_fNeedFactor( 1.0f )
    , m_fAvailFactor( 0.0f )
    , m_pNeed( TNULL )
    , m_pAvail( TNULL )
    , m_pFilter( TNULL )
{
}

// $Barnyard: FUNCTION 0054d9e0
// $Barnyard: FUNCTION 0054dfe0
ADrive::~ADrive()
{
	if ( m_pNeed )
		delete m_pNeed;

	if ( m_pAvail )
		delete m_pAvail;

	if ( m_pFilter )
		delete m_pFilter;
}

// $Barnyard: FUNCTION 0054db80
TBOOL ADrive::Create( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	a_pProperties->GetOptionalPropertyValue( m_strName, "name" );
	a_pProperties->GetOptionalFloatOrIntValue( m_fNeedFactor, "needfactor" );
	a_pProperties->GetOptionalFloatOrIntValue( m_fAvailFactor, "availfactor" );

	const PBProperties* pNeed = a_pProperties->FindProperties( "need" );

	if ( pNeed )
		m_pNeed = ADriveFunction::CreateFromProperties( pNeed, a_pDataSize );

	const PBProperties* pAvail = a_pProperties->FindProperties( "avail" );

	if ( pAvail )
		m_pAvail = ADriveFunction::CreateFromProperties( pAvail, a_pDataSize );

	const PBProperties* pFilter = a_pProperties->FindProperties( "filter" );

	if ( pFilter )
		m_pFilter = ADriveFunction::CreateFromProperties( pFilter, a_pDataSize );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054d960
void ADrive::InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData )
{
	if ( m_pNeed )
		m_pNeed->InitData( a_pUnit, a_pData );

	if ( m_pAvail )
		m_pAvail->InitData( a_pUnit, a_pData );

	if ( m_pFilter )
		m_pFilter->InitData( a_pUnit, a_pData );
}

// $Barnyard: FUNCTION 0054d8c0
ADrive::Rating ADrive::Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData )
{
	TFLOAT fNeed = 0.0f;

	if ( m_pNeed )
		m_pNeed->Evaluate( a_pUnit, a_pData, &fNeed );

	TFLOAT fAvail = 0.0f;

	if ( m_pAvail )
		m_pAvail->Evaluate( a_pUnit, a_pData, &fAvail );

	TFLOAT fValue = ( ( 1.0f - m_fNeedFactor - m_fAvailFactor ) * fNeed + m_fAvailFactor ) * fAvail + fNeed * m_fNeedFactor;

	if ( m_pFilter )
		m_pFilter->Evaluate( a_pUnit, a_pData, &fValue );

	Rating oRating;
	oRating.pDrive = this;
	oRating.fValue = fValue;

	return oRating;
}
