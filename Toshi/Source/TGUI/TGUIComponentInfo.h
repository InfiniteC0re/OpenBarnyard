#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: description of a component class (TGUIComponent), registered to
// the TGUIManager by the static initialiser of the class: its name (the
// "type" of the component blocks), a label and the names of its elements (the
// "element[name]" blocks), in the order of the elements of the component.
//-----------------------------------------------------------------------------
class TGUIComponentInfo : public TNodeList<TGUIComponentInfo>::TNode
{
public:
	TGUIComponentInfo();
	TGUIComponentInfo( const TGUIComponentInfo& a_rcOther );
	~TGUIComponentInfo();

	TGUIComponentInfo& operator=( const TGUIComponentInfo& a_rcOther );

	// Unregisters the class and releases its data
	void Clear();

	// Sets the name and finds the TClass with that name
	void SetComponentName( const TPString8& a_rcName );
	void SetComponentLabel( const TPString8& a_rcLabel );
	void AddElementName( const TPString8& a_rcName );

	// $JPOG: FUNCTION TGuiInterface.dll+00006f90
	const TPString8& GetComponentName() const { return m_strComponentName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006f80
	const TPString8& GetComponentLabel() const { return m_strComponentLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006f70
	const TClass* GetTClass() const { return m_pClass; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006f60
	TINT GetElementCount() const { return m_aElementNames.Size(); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006f50
	const TPString8& GetElementName( TINT a_iIndex ) const { return m_aElementNames[ a_iIndex ]; }

private:
	void FreeElementNames();

private:
	TPString8         m_strComponentName;  // 0x0C
	TPString8         m_strComponentLabel; // 0x10
	const TClass*     m_pClass;            // 0x14
	TArray<TPString8> m_aElementNames;     // 0x18
};

TOSHI_NAMESPACE_END
