#include "ToshiPCH.h"
#include "TGUIPropertySpecSimple.h"
#include "TGUIDeserialisable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $NicktoonsPS2: FUNCTION 00387920
// $JPOG: FUNCTION TGuiInterface.dll+00013050
TGUIPropertySpecSimple::TGUIPropertySpecSimple( const TPString8& a_rcName )
    : TGUIPropertySpec( a_rcName )
    , m_fnSet( TNULL )
    , m_fnGet( TNULL )
    , m_fnValidate( TNULL )
{
}

// $NicktoonsPS2: FUNCTION 00387978
// $JPOG: FUNCTION TGuiInterface.dll+00012e10
TGUIPropertySpecSimple::TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint )
    : TGUIPropertySpec( a_rcName, a_pType, a_rcUnknown, a_bAutoInstall, a_pSourceClass, a_iSortHint )
    , m_fnSet( a_fnSet )
    , m_fnGet( a_fnGet )
    , m_fnValidate( TNULL )
{
}

// NOTE: stripped from Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00012e60
TGUIPropertySpecSimple::TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, const TClass* a_pBaseClass )
    : TGUIPropertySpec( a_rcName, a_pType, a_rcUnknown, a_bAutoInstall, a_pSourceClass, a_iSortHint, a_pBaseClass )
    , m_fnSet( a_fnSet )
    , m_fnGet( a_fnGet )
    , m_fnValidate( TNULL )
{
}

// $NicktoonsPS2: FUNCTION 003879f0
// $JPOG: FUNCTION TGuiInterface.dll+00012ec0
TGUIPropertySpecSimple::TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, TINT a_iEnumCount, const TCHAR* const* a_ppEnumNames )
    : TGUIPropertySpec( a_rcName, a_pType, a_rcUnknown, a_bAutoInstall, a_pSourceClass, a_iSortHint, a_iEnumCount, a_ppEnumNames )
    , m_fnSet( a_fnSet )
    , m_fnGet( a_fnGet )
    , m_fnValidate( TNULL )
{
}

// $NicktoonsPS2: FUNCTION 00387a78
// $JPOG: FUNCTION TGuiInterface.dll+00012f60
TGUIPropertySpecSimple::TGUIPropertySpecSimple( const TGUIPropertySpecSimple& a_rcOther )
    : TGUIPropertySpec( a_rcOther )
    , m_fnSet( a_rcOther.m_fnSet )
    , m_fnGet( a_rcOther.m_fnGet )
    , m_fnValidate( a_rcOther.m_fnValidate )
{
}

// $NicktoonsPS2: FUNCTION 00387ae8
// $JPOG: FUNCTION TGuiInterface.dll+00013040
TGUIPropertySpecSimple::~TGUIPropertySpecSimple()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00012f20
TGUIPropertySpecSimple& TGUIPropertySpecSimple::operator=( const TGUIPropertySpecSimple& a_rcOther )
{
	TGUIPropertySpec::operator=( a_rcOther );
	m_fnSet      = a_rcOther.m_fnSet;
	m_fnGet      = a_rcOther.m_fnGet;
	m_fnValidate = a_rcOther.m_fnValidate;
	return *this;
}

// $NicktoonsPS2: FUNCTION 00387b10
// $JPOG: FUNCTION TGuiInterface.dll+00013010
TGUIPropertySpecSimple* TGUIPropertySpecSimple::Clone() const
{
	return new TGUIPropertySpecSimple( *this );
}

// $NicktoonsPS2: FUNCTION 00387b58
// $JPOG: FUNCTION TGuiInterface.dll+00012ff0
void TGUIPropertySpecSimple::set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	( a_pObject->*m_fnSet )( a_rcValue );
}

// $NicktoonsPS2: FUNCTION 00387bb0
// $JPOG: FUNCTION TGuiInterface.dll+00012fd0
PPropertyValue TGUIPropertySpecSimple::get( const TGUIDeserialisable* a_pObject ) const
{
	return ( a_pObject->*m_fnGet )();
}

// $NicktoonsPS2: FUNCTION 00387c10
// $JPOG: FUNCTION TGuiInterface.dll+00012fb0
TBOOL TGUIPropertySpecSimple::validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	// NOTE: the original calls the validator unconditionally, no constructor sets it
	if ( !m_fnValidate )
		return TTRUE;

	return ( const_cast<TGUIDeserialisable*>( a_pObject )->*m_fnValidate )( a_rcValue );
}

TOSHI_NAMESPACE_END
