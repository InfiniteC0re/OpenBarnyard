#pragma once
#include "TGUIStyleParameterSpec.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a parameter of a style ("param = {LABEL};" and the "param" of the
// elements): a value the widgets of the style give to the elements, like the
// text of a label. The spec makes it a property of the widgets.
//
// NOTE: the vtable pointer is after the fields in Nicktoons (GCC).
//-----------------------------------------------------------------------------
class TGUIStyleParameterInfo
{
public:
	TGUIStyleParameterInfo();
	virtual ~TGUIStyleParameterInfo();

	TGUIStyleParameterInfo& operator=( const TGUIStyleParameterInfo& a_rcOther );

	const TPString8& GetName() const { return m_strName; }
	const TClass*    GetType() const { return m_pType; }
	TINT             GetIndex() const { return m_iIndex; }

	TGUIStyleParameterSpec* GetSpec() const { return m_pSpec; }

public:
	TPString8               m_strName; // 0x00
	const TClass*           m_pType;   // 0x04 type of the values (TYPE_UNDEF until an element uses it)
	TINT                    m_iIndex;  // 0x08 index in the style
	TGUIStyleParameterSpec* m_pSpec;   // 0x0C
};

TOSHI_NAMESPACE_END
