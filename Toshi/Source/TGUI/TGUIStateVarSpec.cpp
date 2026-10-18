#include "ToshiPCH.h"
#include "TGUIStateVarSpec.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inlined in TGUIWidget::DeclareStateVar (the state variables are boolean and auto installed)
TGUIStateVarSpec::TGUIStateVarSpec( const TPString8& a_rcName, const TPString8& a_rcCategory, const TClass* a_pSourceClass, TINT a_iSortHint, t_Setter a_fnSet, t_Getter a_fnGet, TUINT a_uiMask )
    : TGUIPropertySpec( a_rcName, PPropertyValue::TYPE_BOOL, a_rcCategory, TTRUE, a_pSourceClass, a_iSortHint )
    , m_fnSet( a_fnSet )
    , m_fnGet( a_fnGet )
    , m_fnValidate( TNULL )
    , m_uiMask( a_uiMask )
{
}

// NOTE: inlined in Clone
TGUIStateVarSpec::TGUIStateVarSpec( const TGUIStateVarSpec& a_rcOther )
    : TGUIPropertySpec( a_rcOther )
    , m_fnSet( a_rcOther.m_fnSet )
    , m_fnGet( a_rcOther.m_fnGet )
    , m_fnValidate( a_rcOther.m_fnValidate )
    , m_uiMask( a_rcOther.m_uiMask )
{
}

// $NicktoonsPS2: FUNCTION 00376be8
TGUIStateVarSpec::~TGUIStateVarSpec()
{
}

// $NicktoonsPS2: FUNCTION 00376c10
TGUIStateVarSpec* TGUIStateVarSpec::Clone() const
{
	return new TGUIStateVarSpec( *this );
}

// $NicktoonsPS2: FUNCTION 00376ca8
void TGUIStateVarSpec::set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	( a_pObject->*m_fnSet )( a_rcValue, m_uiMask );
}

// $NicktoonsPS2: FUNCTION 00376d00
PPropertyValue TGUIStateVarSpec::get( const TGUIDeserialisable* a_pObject ) const
{
	return ( a_pObject->*m_fnGet )( m_uiMask );
}

// $NicktoonsPS2: FUNCTION 00376d68
TBOOL TGUIStateVarSpec::validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const
{
	// NOTE: the original calls the validator unconditionally, no constructor sets it
	if ( !m_fnValidate )
		return TTRUE;

	return ( const_cast<TGUIDeserialisable*>( a_pObject )->*m_fnValidate )( a_rcValue, m_uiMask );
}

TOSHI_NAMESPACE_END
