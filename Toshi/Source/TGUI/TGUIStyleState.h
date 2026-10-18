#pragma once
#include "TGUIStateRule.h"

#include <Toshi/TNodeList.h>

TOSHI_NAMESPACE_START

class TGUIComponent;

//-----------------------------------------------------------------------------
// Purpose: the components of a root component of a style for the widget
// states that match a rule (a "state" block: "Focused = true;" and the
// "component" blocks). The components are painted in their order.
//-----------------------------------------------------------------------------
class TGUIStyleState : public TNodeList<TGUIStyleState>::TNode
{
public:
	TGUIStyleState( const TGUIStateRule& a_rcRule );
	TGUIStyleState( const TGUIStyleState& a_rcOther );
	~TGUIStyleState();

	TGUIStyleState* Clone() const;

	void AddComponent( TGUIComponent* a_pComponent );

	// $JPOG: FUNCTION TGuiInterface.dll+00006a40
	void SetStateRule( const TGUIStateRule& a_rcRule ) { m_oRule = a_rcRule; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006a70
	void GetStateRule( TGUIStateRule& a_rOutRule ) const { a_rOutRule = m_oRule; }

	const TGUIStateRule& GetStateRule() const { return m_oRule; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006a60
	TNodeList<TGUIComponent>&       GetComponents() { return m_oComponents; }
	const TNodeList<TGUIComponent>& GetComponents() const { return m_oComponents; }

private:
	TGUIStateRule            m_oRule;       // 0x0C
	TNodeList<TGUIComponent> m_oComponents; // 0x14
};

TOSHI_NAMESPACE_END
