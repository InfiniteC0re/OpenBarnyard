#include "pch.h"
#include "ADriveGroup.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054df30, CreateObject 0054e070, CreateObjectInPlace 0054e0f0
TDEFINE_CLASS( ADriveGroup );

// $Barnyard: FUNCTION 0054df40
ADriveGroup::ADriveGroup()
    : m_iNumDrives( 0 )
    , m_pDrives( TNULL )
    , m_fSubsumeFactor( 1.0f )
{
}

// $Barnyard: FUNCTION 0054df70
// $Barnyard: FUNCTION 0054e4d0
ADriveGroup::~ADriveGroup()
{
	if ( m_pDrives )
		delete[] m_pDrives;
}

// $Barnyard: FUNCTION 0054e250
TBOOL ADriveGroup::Create( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	TPString8 strDrive = TPS8D( "drive" );

	a_pProperties->GetOptionalFloatOrIntValue( m_fSubsumeFactor, "subsumefactor" );

	// Count the drives first
	m_iNumDrives = 0;

	TINT iNumProperties = a_pProperties->GetPropertyCount();

	for ( TINT i = 0; i < iNumProperties; i++ )
	{
		if ( TPString8( a_pProperties->GetPropertyByIndex( i )->GetName().GetString() ) == strDrive )
			m_iNumDrives++;
	}

	if ( m_iNumDrives > 0 )
	{
		m_pDrives = new ADrive[ m_iNumDrives ];

		TINT iDrive = 0;

		for ( TINT i = 0; i < iNumProperties; i++ )
		{
			const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );

			if ( TPString8( pProperty->GetName().GetString() ) == strDrive )
				m_pDrives[ iDrive++ ].Create( pProperty->GetValue()->GetProperties(), a_pDataSize );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0054e030
void ADriveGroup::InitData( AUnit* a_pUnit, AInteractionActivityData* a_pData )
{
	for ( TINT i = 0; i < m_iNumDrives; i++ )
		m_pDrives[ i ].InitData( a_pUnit, a_pData );
}

// $Barnyard: FUNCTION 0054e1b0
void ADriveGroup::Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, ADrive::Rating* a_pBest )
{
	ADrive::Rating oBest;
	oBest.pDrive = TNULL;
	oBest.fValue = 0.0f;

	// The first drive wins if two have the same value
	for ( TINT i = m_iNumDrives - 1; i >= 0; i-- )
	{
		ADrive::Rating oRating = m_pDrives[ i ].Evaluate( a_pUnit, a_pData );

		if ( oBest.fValue <= oRating.fValue )
			oBest = oRating;
	}

	if ( oBest.fValue * m_fSubsumeFactor >= a_pBest->fValue )
		*a_pBest = oBest;
}

// $Barnyard: FUNCTION 0054e170
ADrive* ADriveGroup::FindDrive( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < m_iNumDrives; i++ )
	{
		if ( m_pDrives[ i ].GetName() == a_rcName )
			return &m_pDrives[ i ];
	}

	return TNULL;
}
