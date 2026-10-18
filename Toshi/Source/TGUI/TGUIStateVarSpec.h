#pragma once
#include "TGUIPropertySpec.h"
#include "TGUIDeserialisable.h" // complete for the member function pointers (MSVC represents them differently for incomplete classes)

#include <Plugins/PPropertyParser/PPropertyValue.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: property spec of a state variable of a widget class ("Enabled =
// false;"): the widgets read and write the bit of their state with member
// functions that take the mask of the variable.
//
// NOTE: the name is unknown, Nicktoons only has its vtable (TGUIWidget creates
// one per state variable, see TGUIWidget::DeclareStateVar).
//-----------------------------------------------------------------------------
class TGUIStateVarSpec : public TGUIPropertySpec
{
public:
	using t_Setter    = void ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue, TUINT a_uiMask );
	using t_Getter    = PPropertyValue ( TGUIDeserialisable::* )( TUINT a_uiMask ) const;
	using t_Validator = TBOOL ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue, TUINT a_uiMask );

public:
	TGUIStateVarSpec( const TPString8& a_rcName, const TPString8& a_rcCategory, const TClass* a_pSourceClass, TINT a_iSortHint, t_Setter a_fnSet, t_Getter a_fnGet, TUINT a_uiMask );
	TGUIStateVarSpec( const TGUIStateVarSpec& a_rcOther );
	~TGUIStateVarSpec();

	virtual TGUIStateVarSpec* Clone() const override;
	virtual void              set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;
	virtual PPropertyValue    get( const TGUIDeserialisable* a_pObject ) const override;
	virtual TBOOL             validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;

	TUINT GetMask() const { return m_uiMask; }

private:
	t_Setter    m_fnSet;      // 0x28
	t_Getter    m_fnGet;      // 0x30
	t_Validator m_fnValidate; // 0x38 never set
	TUINT       m_uiMask;     // 0x40
};

TOSHI_NAMESPACE_END
