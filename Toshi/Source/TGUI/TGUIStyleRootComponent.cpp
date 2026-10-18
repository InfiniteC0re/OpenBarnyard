#include "ToshiPCH.h"
#include "TGUIStyleRootComponent.h"
#include "TGUIComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inlined in TGUIStyle::Create in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00006970
TGUIStyleRootComponent::TGUIStyleRootComponent()
    : m_iMarginLeft( 0 )
    , m_iMarginRight( 0 )
    , m_iMarginTop( 0 )
    , m_iMarginBottom( 0 )
{
}

// NOTE: inlined in the TGUIStyle destructor in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00006820
TGUIStyleRootComponent::~TGUIStyleRootComponent()
{
	m_oStates.DeleteAll();
}

// $NicktoonsPS2: FUNCTION 00369e10
// $JPOG: FUNCTION TGuiInterface.dll+00015d00
TGUIStyleRootComponent& TGUIStyleRootComponent::operator=( const TGUIStyleRootComponent& a_rcOther )
{
	m_iMarginLeft   = a_rcOther.m_iMarginLeft;
	m_iMarginRight  = a_rcOther.m_iMarginRight;
	m_iMarginTop    = a_rcOther.m_iMarginTop;
	m_iMarginBottom = a_rcOther.m_iMarginBottom;

	for ( auto it = a_rcOther.m_oStates.Begin(); it != a_rcOther.m_oStates.End(); it++ )
		m_oStates.InsertTail( it->Clone() );

	return *this;
}

// $NicktoonsPS2: FUNCTION 00369cc8
// $JPOG: FUNCTION TGuiInterface.dll+00015c40
TGUIStyleState* TGUIStyleRootComponent::FindState( const TGUIStateRule& a_rcRule )
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
	{
		if ( it->GetStateRule() == a_rcRule )
			return it;
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 00369d60
// $JPOG: FUNCTION TGuiInterface.dll+00015c80
TGUIStyleState* TGUIStyleRootComponent::CreateState( const TGUIStateRule& a_rcRule )
{
	TGUIStyleState* pState = FindState( a_rcRule );

	if ( !pState )
	{
		pState = new TGUIStyleState( a_rcRule );
		m_oStates.InsertTail( pState );
	}

	return pState;
}

// NOTE: inlined in TGUIStyle::GetState
TGUIStyleState* TGUIStyleRootComponent::GetState( TUINT a_uiState ) const
{
	for ( auto it = m_oStates.Begin(); it != m_oStates.End(); it++ )
	{
		if ( it->GetStateRule().Matches( a_uiState ) )
			return it;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
