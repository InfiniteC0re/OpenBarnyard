#include "ToshiPCH.h"
#include "TGUIStyleState.h"
#include "TGUIComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inlined in TGUIStyleRootComponent::CreateState in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00006a90
TGUIStyleState::TGUIStyleState( const TGUIStateRule& a_rcRule )
    : m_oRule( a_rcRule )
{
}

// The components are cloned
// $NicktoonsPS2: FUNCTION 00369b90
// $JPOG: FUNCTION TGuiInterface.dll+00015bb0
TGUIStyleState::TGUIStyleState( const TGUIStyleState& a_rcOther )
{
	for ( auto it = a_rcOther.m_oComponents.Begin(); it != a_rcOther.m_oComponents.End(); it++ )
		m_oComponents.InsertTail( it->Clone() );

	m_oRule = a_rcOther.m_oRule;
}

// $NicktoonsPS2: FUNCTION 0036df98
// $JPOG: FUNCTION TGuiInterface.dll+00006990
TGUIStyleState::~TGUIStyleState()
{
	m_oComponents.DeleteAll();
}

// $NicktoonsPS2: FUNCTION 00369c88
// $JPOG: FUNCTION TGuiInterface.dll+00019be0
TGUIStyleState* TGUIStyleState::Clone() const
{
	return new TGUIStyleState( *this );
}

// $JPOG: FUNCTION TGuiInterface.dll+00006a10
void TGUIStyleState::AddComponent( TGUIComponent* a_pComponent )
{
	m_oComponents.InsertTail( a_pComponent );
}

TOSHI_NAMESPACE_END
