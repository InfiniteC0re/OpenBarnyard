#include "ToshiPCH.h"
#include "PPropertyName.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006b42c0
PPropertyName::PPropertyName()
{
}

// $Barnyard: FUNCTION 006b42d0
PPropertyName::PPropertyName( const TPString8& a_rcName )
{
	Split( a_rcName );
}

// $Barnyard: FUNCTION 006b4000
void PPropertyName::Split( const TPString8& a_rcString )
{
	const TString8& rcString = a_rcString.GetString8();
	TINT            iOpenPos = rcString.Find( '[', 0 );

	if ( iOpenPos == -1 )
	{
		m_strName = a_rcString;
		return;
	}

	m_strName = rcString.Mid( 0, iOpenPos ).GetString();

	TINT iClosePos = rcString.Find( ']', iOpenPos );

	if ( iClosePos != -1 )
		m_strSubName = rcString.Mid( iOpenPos + 1, iClosePos - iOpenPos - 1 ).GetString();
}

// $Barnyard: FUNCTION 006b4190
TPString8 PPropertyName::GetString() const
{
	TString8 strResult = m_strName.GetString8();

	if ( m_strSubName.GetString8().Length() != 0 )
		strResult += ( TString8( "[" ) + m_strSubName.GetString() + "]" ).GetString();

	return TPString8( strResult.GetString() );
}

// $Barnyard: FUNCTION 006b4260
PPropertyName& PPropertyName::operator=( const PPropertyName& a_rcOther )
{
	m_strName    = a_rcOther.m_strName;
	m_strSubName = a_rcOther.m_strSubName;
	return *this;
}

// $Barnyard: FUNCTION 006b3fa0
PPropertyName::PPropertyName( const TPString8& a_rcName, const TPString8& a_rcSubName )
    : m_strName( a_rcName )
    , m_strSubName( a_rcSubName )
{
}

// $Barnyard: FUNCTION 006b3fd0
PPropertyName::PPropertyName( const PPropertyName& a_rcOther )
    : m_strName( a_rcOther.m_strName )
    , m_strSubName( a_rcOther.m_strSubName )
{
}

// $Barnyard: FUNCTION 006b1c00
PPropertyName::~PPropertyName()
{
}

// $Barnyard: FUNCTION 006b3dc0
TBOOL PPropertyName::operator==( const PPropertyName& a_rcOther ) const
{
	return m_strName.Compare( a_rcOther.m_strName ) == 0 && m_strSubName.Compare( a_rcOther.m_strSubName ) == 0;
}

// $Barnyard: FUNCTION 006b3e00
TBOOL PPropertyName::operator!=( const PPropertyName& a_rcOther ) const
{
	return m_strName.Compare( a_rcOther.m_strName ) != 0 || m_strSubName.Compare( a_rcOther.m_strSubName ) != 0;
}

// $Barnyard: FUNCTION 006b3e40
TBOOL PPropertyName::operator<( const PPropertyName& a_rcOther ) const
{
	TINT iResult = m_strName.Compare( a_rcOther.m_strName );

	if ( iResult < 0 ) return TTRUE;
	if ( iResult > 0 ) return TFALSE;

	return m_strSubName.Compare( a_rcOther.m_strSubName ) < 0;
}

// $Barnyard: FUNCTION 006b3e80
TBOOL PPropertyName::operator>( const PPropertyName& a_rcOther ) const
{
	TINT iResult = m_strName.Compare( a_rcOther.m_strName );

	if ( iResult > 0 ) return TTRUE;
	if ( iResult < 0 ) return TFALSE;

	return m_strSubName.Compare( a_rcOther.m_strSubName ) > 0;
}

// $Barnyard: FUNCTION 006b3ec0
TString8 operator+( const TString8& a_rcString, const PPropertyName& a_rcName )
{
	TString8 strResult( a_rcString );
	strResult += a_rcName.GetName().GetString8().GetString();

	if ( a_rcName.GetSubName().GetString8().Length() != 0 )
	{
		TString8 strSubName = TString8( "[" ) + a_rcName.GetSubName().GetString8().GetString() + "]";
		strResult += strSubName;
	}

	return strResult;
}
