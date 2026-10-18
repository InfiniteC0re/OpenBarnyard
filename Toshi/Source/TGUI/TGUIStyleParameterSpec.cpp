#include "ToshiPCH.h"
#include "TGUIStyleParameterSpec.h"
#include "TGUIDeserialisable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inlined in the TGUIStyleParameterInfo constructor
TGUIStyleParameterSpec::TGUIStyleParameterSpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcCategory, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, t_Setter a_fnSet, t_Getter a_fnGet, TGUIStyleParameterInfo* a_pParameter )
    : TGUIPropertySpec( a_rcName, a_pType, a_rcCategory, a_bAutoInstall, a_pSourceClass, a_iSortHint )
    , m_fnSet( a_fnSet )
    , m_fnGet( a_fnGet )
    , m_fnValidate( TNULL )
    , m_pParameter( a_pParameter )
{
}

// NOTE: inlined in Clone
TGUIStyleParameterSpec::TGUIStyleParameterSpec( const TGUIStyleParameterSpec& a_rcOther )
    : TGUIPropertySpec( a_rcOther )
    , m_fnSet( a_rcOther.m_fnSet )
    , m_fnGet( a_rcOther.m_fnGet )
    , m_fnValidate( a_rcOther.m_fnValidate )
    , m_pParameter( a_rcOther.m_pParameter )
{
}

// $NicktoonsPS2: FUNCTION 0036e058
TGUIStyleParameterSpec::~TGUIStyleParameterSpec()
{
}

// $NicktoonsPS2: FUNCTION 0036e080
TGUIStyleParameterSpec* TGUIStyleParameterSpec::Clone() const
{
	return new TGUIStyleParameterSpec( *this );
}

// $NicktoonsPS2: FUNCTION 0036e118
void TGUIStyleParameterSpec::set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	( a_pObject->*m_fnSet )( a_rcValue, m_pParameter );
}

// $NicktoonsPS2: FUNCTION 0036e170
PPropertyValue TGUIStyleParameterSpec::get( const TGUIDeserialisable* a_pObject ) const
{
	return ( a_pObject->*m_fnGet )( m_pParameter );
}

// $NicktoonsPS2: FUNCTION 0036e1d8
TBOOL TGUIStyleParameterSpec::validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	// NOTE: the original calls the validator unconditionally, no constructor sets it
	if ( !m_fnValidate )
		return TTRUE;

	return ( const_cast<TGUIDeserialisable*>( a_pObject )->*m_fnValidate )( a_rcValue, m_pParameter );
}

void TGUIStyleParameterSpec::Assign( const TGUIStyleParameterSpec& a_rcOther )
{
	TGUIPropertySpec::operator=( a_rcOther );
	m_fnSet      = a_rcOther.m_fnSet;
	m_fnGet      = a_rcOther.m_fnGet;
	m_fnValidate = a_rcOther.m_fnValidate;
}

TOSHI_NAMESPACE_END
