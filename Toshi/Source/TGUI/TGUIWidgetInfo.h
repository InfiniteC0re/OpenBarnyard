#pragma once
#include "TGUIStateVarInfo.h"
#include "TGUIStateRule.h"
#include "TGUIDeserialisable.h"

#include <Toshi/TNodeList.h>
#include <Toshi/TArray.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: description of a widget class, registered to the TGUIManager by
// the static initialiser of the class: its name (the "widget" property of the
// styles), a label, the names of its root components (the "component[name]"
// blocks of the styles), its state variables (the conditions of the "state"
// blocks) and its properties (the "widgetprop" block).
//
// NOTE: the state variables of the base classes are added to the set of each
// class with AddStateVars. Nicktoons has no input actions (JPOG keeps them
// here).
//-----------------------------------------------------------------------------
class TGUIWidgetInfo : public TNodeList<TGUIWidgetInfo>::TNode
{
public:
	using StateVarSet = TArray<TGUIStateVarInfo>;
	using PropertySet = TGUIDeserialisable::PropertySet;

public:
	TGUIWidgetInfo();
	~TGUIWidgetInfo();

	// Unregisters the class and releases its data
	void Clear();

	// Sets the name and finds the TClass with that name
	void SetWidgetName( const TPString8& a_rcName );
	void SetWidgetLabel( const TPString8& a_rcLabel );
	void SetProperties( PropertySet* a_pSet, PropertySet* a_pOrderedSet );
	void SetConcrete( TBOOL a_bConcrete );

	// Adds the state variables of a_rcStateVars to the set (ordered by bit)
	void AddStateVars( const StateVarSet& a_rcStateVars );

	// Adds a state variable to the set (ordered by bit)
	void AddStateVar( const TGUIStateVarInfo& a_rcStateVar );

	// Adds the name of a root component
	void AddComponentName( const TPString8& a_rcName );

	const TGUIStateVarInfo* GetStateVarInfo( const TPString8& a_rcName ) const;
	TUINT                   GetStateVarMask( const TPString8& a_rcName ) const;
	TINT                    GetComponentIndex( const TPString8& a_rcName ) const;

	// "default" for the empty rule, the state variables of the rule ("Focused and not Enabled") otherwise
	TPString8 GetStateExpression( TUINT a_uiMask, TUINT a_uiValue ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00007e10
	TPString8 GetStateExpression( TGUIStateRule a_oRule ) const { return GetStateExpression( a_oRule.uiMask, a_oRule.uiValue ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007ec0
	const TPString8& GetWidgetName() const { return m_strWidgetName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007eb0
	const TPString8& GetWidgetLabel() const { return m_strWidgetLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007ea0
	const TClass* GetTClass() const { return m_pClass; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e90
	TINT GetComponentCount() const { return m_aComponentNames.Size(); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e80
	const TPString8& GetComponentName( TINT a_iIndex ) const { return m_aComponentNames[ a_iIndex ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007df0
	TArray<TPString8>& GetComponentNameSet() { return m_aComponentNames; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e70
	TINT GetStateVarCount() const { return m_aStateVars.Size(); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e60
	const TGUIStateVarInfo* GetStateVarInfo( TINT a_iIndex ) const { return &m_aStateVars[ a_iIndex ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e00
	StateVarSet&       GetStateVarSet() { return m_aStateVars; }
	const StateVarSet& GetStateVarSet() const { return m_aStateVars; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e30
	TBOOL IsConcrete() const { return m_bConcrete; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007e40
	TINT GetInstanceCount() const { return m_iInstanceCount; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007dd0
	void AddInstance() { m_iInstanceCount++; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007dc0
	void DelInstance() { m_iInstanceCount--; }

	// The input actions (TGUIActionMap bits) the widgets of the class use, shown by the input context
	// $JPOG: FUNCTION TGuiInterface.dll+00007e50
	TUINT GetInputActions() const { return m_uiInputActions; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007de0
	void AddInputAction( TUINT a_uiAction ) { m_uiInputActions |= a_uiAction; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007db0
	TINT GetPropertyCount() const { return m_pProperties ? m_pProperties->Size() : 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007da0
	const TGUIPropertySpec* GetPropertyInfo( TINT a_iIndex ) const { return ( *m_pProperties )[ a_iIndex ]; }

	// Binary search of the ordered property set
	const TGUIPropertySpec* GetPropertyInfo( const TPString8& a_rcName ) const;

	PropertySet* GetProperties() const { return m_pProperties; }
	PropertySet* GetOrderedProperties() const { return m_pOrderedProperties; }

private:
	TPString8         m_strWidgetName;      // 0x0C
	TPString8         m_strWidgetLabel;     // 0x10
	const TClass*     m_pClass;             // 0x14
	TArray<TPString8> m_aComponentNames;    // 0x18
	StateVarSet       m_aStateVars;         // 0x28 ordered by bit (TGUIStateVarInfo::Comparator)
	PropertySet*      m_pProperties;        // 0x38
	PropertySet*      m_pOrderedProperties; // 0x3C
	TUINT             m_uiInputActions;     // 0x40
	TBOOL             m_bConcrete;          // 0x44

	// NOTE: only in JPOG
	TINT m_iInstanceCount;
};

TOSHI_NAMESPACE_END
