#include "ToshiPCH.h"
#include "PPropertyValueArray.h"

#include "Math/TVector2.h"
#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TQuaternion.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// NOTE: 00592e70 is the ctor of the TArray<PPropertyValue> base, only called from here
// $Barnyard: FUNCTION 005968a0
// $Barnyard: FUNCTION 00592e70
PPropertyValueArray::PPropertyValueArray( TINT a_iSize )
{
	m_iNumValues      = 0;
	m_iGrowSize       = a_iSize;
	m_iNumAllocValues = a_iSize;
	m_pValues         = ( a_iSize > 0 ) ? new PPropertyValue[ a_iSize ] : TNULL;
}

// $Barnyard: FUNCTION 005931e0
PPropertyValueArray::~PPropertyValueArray()
{
	if ( m_pValues )
		delete[] m_pValues;
}

// NOTE: retail is a method of the TArray<PPropertyValue> base, also used by other arrays of values
// $Barnyard: FUNCTION 00592f90
void PPropertyValueArray::Resize( TINT a_iNewSize )
{
	if ( a_iNewSize == 0 )
	{
		if ( m_pValues )
			delete[] m_pValues;

		m_pValues         = TNULL;
		m_iNumAllocValues = 0;
		m_iNumValues      = 0;
		return;
	}

	PPropertyValue* pNewValues = new PPropertyValue[ a_iNewSize ];
	TINT            iNumCopy   = TMath::Min( a_iNewSize, m_iNumValues );

	for ( TINT i = 0; i < iNumCopy; i++ )
		pNewValues[ i ] = m_pValues[ i ];

	m_iNumAllocValues = a_iNewSize;

	if ( m_pValues )
		delete[] m_pValues;

	m_pValues = pNewValues;
}

// $Barnyard: FUNCTION 006b16a0
PPropertyValueArray::PPropertyValueArray( const PPropertyValue* a_pValues, TINT a_iNumValues )
{
	m_iGrowSize       = 1;
	m_iNumValues      = 0;
	m_iNumAllocValues = a_iNumValues;
	m_pValues         = ( a_iNumValues > 0 ) ? new PPropertyValue[ a_iNumValues ] : TNULL;

	for ( TINT i = 0; i < a_iNumValues; i++ )
		m_pValues[ i ] = a_pValues[ i ];

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail copies the values but never sets the number of values, so the array stays empty
	m_iNumValues = a_iNumValues;
#endif
}

// $Barnyard: FUNCTION 006afbc0
TBOOL PPropertyValueArray::IsTVector2() const
{
	return CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 2 );
}

// $Barnyard: FUNCTION 006afc00
TBOOL PPropertyValueArray::IsTVector3() const
{
	return CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 3 );
}

// $Barnyard: FUNCTION 006afc40
TBOOL PPropertyValueArray::IsTVector4() const
{
	return CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 4 );
}

// $Barnyard: FUNCTION 006afc80
TBOOL PPropertyValueArray::IsTQuaternion() const
{
	return CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 4 );
}

// $Barnyard: FUNCTION 006afcc0
TBOOL PPropertyValueArray::GetTVector2( TVector2& a_rOut ) const
{
	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 2 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006afdc0
TBOOL PPropertyValueArray::GetTVector3( TVector3& a_rOut ) const
{
	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 3 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		a_rOut.z = m_pValues[ 2 ].GetFloat();
		return TTRUE;
	}

	// The w component of a 4 number array is ignored
	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 4 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		a_rOut.z = m_pValues[ 2 ].GetFloat();
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006aff70
TBOOL PPropertyValueArray::GetTVector4( TVector4& a_rOut ) const
{
	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 3 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		a_rOut.z = m_pValues[ 2 ].GetFloat();
		a_rOut.w = 1.0f;
		return TTRUE;
	}

	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 4 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		a_rOut.z = m_pValues[ 2 ].GetFloat();
		a_rOut.w = m_pValues[ 3 ].GetFloat();
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006b0140
TBOOL PPropertyValueArray::GetTQuaternion( TQuaternion& a_rOut ) const
{
	if ( CanBeTypeArray( PPropertyValue::TYPE_FLOAT, 4 ) )
	{
		a_rOut.x = m_pValues[ 0 ].GetFloat();
		a_rOut.y = m_pValues[ 1 ].GetFloat();
		a_rOut.z = m_pValues[ 2 ].GetFloat();
		a_rOut.w = m_pValues[ 3 ].GetFloat();
		return TTRUE;
	}

	return TFALSE;
}
