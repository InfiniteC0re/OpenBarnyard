#pragma once
#include "TGUIPropertySpec.h"
#include "TGUIDeserialisable.h" // complete for the member function pointers (MSVC represents them differently for incomplete classes)

#include <Plugins/PPropertyParser/PPropertyValue.h>

TOSHI_NAMESPACE_START

class TGUIStyleParameterInfo;

//-----------------------------------------------------------------------------
// Purpose: property spec of a style parameter (TGUIStyleParameterInfo): the
// widgets of the style read and write the parameter with member functions
// that also take the parameter.
//
// NOTE: the name is unknown, Nicktoons only has its vtable. The functions are
// TGUIWidget::Set_style_parameter and Get_style_parameter (part C of the TGUI
// port), set by TGUIStyleParameterInfo.
//-----------------------------------------------------------------------------
class TGUIStyleParameterSpec : public TGUIPropertySpec
{
public:
	using t_Setter    = void ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue, TGUIStyleParameterInfo* a_pParameter );
	using t_Getter    = PPropertyValue ( TGUIDeserialisable::* )( TGUIStyleParameterInfo* a_pParameter ) const;
	using t_Validator = TBOOL ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue, TGUIStyleParameterInfo* a_pParameter );

public:
	TGUIStyleParameterSpec( const TPString8& a_rcName, const TClass* a_pType, const TPString8& a_rcCategory, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, t_Setter a_fnSet, t_Getter a_fnGet, TGUIStyleParameterInfo* a_pParameter );
	TGUIStyleParameterSpec( const TGUIStyleParameterSpec& a_rcOther );
	~TGUIStyleParameterSpec();

	virtual TGUIStyleParameterSpec* Clone() const override;
	virtual void                    set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;
	virtual PPropertyValue          get( const TGUIDeserialisable* a_pObject ) const override;
	virtual TBOOL                   validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;

	// Copies the spec and the functions, the parameter stays the same
	// NOTE: inlined in TGUIStyleParameterInfo::operator=
	void Assign( const TGUIStyleParameterSpec& a_rcOther );

private:
	t_Setter                m_fnSet;      // 0x28
	t_Getter                m_fnGet;      // 0x30
	t_Validator             m_fnValidate; // 0x38 only set by the copies
	TGUIStyleParameterInfo* m_pParameter; // 0x40
};

TOSHI_NAMESPACE_END
