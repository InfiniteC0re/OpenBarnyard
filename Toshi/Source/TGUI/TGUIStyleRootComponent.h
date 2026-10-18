#pragma once
#include "TGUIStyleState.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a root component of a style ("component[background]",
// "component[foreground]"... the names come from the widget class): margins
// and the states. The first state whose rule matches the state of the widget
// is painted.
//-----------------------------------------------------------------------------
class TGUIStyleRootComponent
{
public:
	TGUIStyleRootComponent();
	~TGUIStyleRootComponent();

	// Copies the margins and appends clones of the states of a_rcOther
	TGUIStyleRootComponent& operator=( const TGUIStyleRootComponent& a_rcOther );

	// The state with this exact rule
	TGUIStyleState* FindState( const TGUIStateRule& a_rcRule );

	// The state with this exact rule, created at the end of the states if it doesn't exist
	TGUIStyleState* CreateState( const TGUIStateRule& a_rcRule );

	// The first state whose rule matches a_uiState
	TGUIStyleState* GetState( TUINT a_uiState ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00006870
	void AddState( TGUIStyleState* a_pState ) { m_oStates.InsertTail( a_pState ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006890
	TNodeList<TGUIStyleState>&       GetStates() { return m_oStates; }
	const TNodeList<TGUIStyleState>& GetStates() const { return m_oStates; }

	// $JPOG: FUNCTION TGuiInterface.dll+000068a0
	void GetMargins( TINT& a_rLeft, TINT& a_rRight, TINT& a_rTop, TINT& a_rBottom )
	{
		a_rLeft   = m_iMarginLeft;
		a_rRight  = m_iMarginRight;
		a_rTop    = m_iMarginTop;
		a_rBottom = m_iMarginBottom;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00006910
	void SetMargins( TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom )
	{
		m_iMarginLeft   = a_iLeft;
		m_iMarginRight  = a_iRight;
		m_iMarginTop    = a_iTop;
		m_iMarginBottom = a_iBottom;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000068e0
	TINT GetMarginLeft() const { return m_iMarginLeft; }

	// $JPOG: FUNCTION TGuiInterface.dll+000068d0
	TINT GetMarginRight() const { return m_iMarginRight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006900
	TINT GetMarginTop() const { return m_iMarginTop; }

	// $JPOG: FUNCTION TGuiInterface.dll+000068f0
	TINT GetMarginBottom() const { return m_iMarginBottom; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006940
	void SetMarginLeft( TINT a_iMargin ) { m_iMarginLeft = a_iMargin; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006930
	void SetMarginRight( TINT a_iMargin ) { m_iMarginRight = a_iMargin; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006960
	void SetMarginTop( TINT a_iMargin ) { m_iMarginTop = a_iMargin; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006950
	void SetMarginBottom( TINT a_iMargin ) { m_iMarginBottom = a_iMargin; }

private:
	TNodeList<TGUIStyleState> m_oStates;       // 0x00
	TINT                      m_iMarginLeft;   // 0x10
	TINT                      m_iMarginRight;  // 0x14
	TINT                      m_iMarginTop;    // 0x18
	TINT                      m_iMarginBottom; // 0x1C
};

TOSHI_NAMESPACE_END
