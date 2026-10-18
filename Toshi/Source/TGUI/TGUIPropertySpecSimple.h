#pragma once
#include "TGUIPropertySpec.h"
#include "TGUIDeserialisable.h" // complete for the member function pointers (MSVC represents them differently for incomplete classes)

#include <Plugins/PPropertyParser/PPropertyValue.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: TGUIPropertySpec that reads and writes the property with member
// functions of the TGUIDeserialisable.
//-----------------------------------------------------------------------------
class TGUIPropertySpecSimple : public TGUIPropertySpec
{
public:
	using t_Setter    = void ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue );
	using t_Getter    = PPropertyValue ( TGUIDeserialisable::* )() const;
	using t_Validator = TBOOL ( TGUIDeserialisable::* )( const PPropertyValue& a_rcValue );

public:
	TGUIPropertySpecSimple( const TPString8& a_rcName );
	TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint );
	TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, const TClass* a_pBaseClass );
	TGUIPropertySpecSimple( const TPString8& a_rcName, t_Setter a_fnSet, t_Getter a_fnGet, const TClass* a_pType, const TPString8& a_rcUnknown, TBOOL a_bAutoInstall, const TClass* a_pSourceClass, TINT a_iSortHint, TINT a_iEnumCount, const TCHAR* const* a_ppEnumNames );
	TGUIPropertySpecSimple( const TGUIPropertySpecSimple& a_rcOther );
	~TGUIPropertySpecSimple();

	virtual TGUIPropertySpecSimple* Clone() const override;
	virtual void                    set( TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;
	virtual PPropertyValue          get( const TGUIDeserialisable* a_pObject ) const override;
	virtual TBOOL                   validate( const TGUIDeserialisable* a_pObject, const PPropertyValue& a_rcValue ) const override;

	TGUIPropertySpecSimple& operator=( const TGUIPropertySpecSimple& a_rcOther );

private:
	t_Setter    m_fnSet;      // 0x28
	t_Getter    m_fnGet;      // 0x30
	t_Validator m_fnValidate; // 0x38 NOTE: only set by the copy constructor (from an uninitialised value)
};

TOSHI_NAMESPACE_END
