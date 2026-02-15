#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c4700
T2GUIContext::T2GUIContext()
{
	m_pRenderer           = TNULL;
	m_pRootElement        = TNULL;
	m_pCreatedRootElement = TNULL;
	m_pDebugCanvas        = TNULL;

	m_pCreatedRootElement = new T2GUIElement;

	if ( m_pCreatedRootElement )
	{
		m_pCreatedRootElement->SetDimensions( 640.0f, 512.0f );
		m_pCreatedRootElement->SetTransform( 0.0f, 0.0f, 0.0f );
	}

	m_pDebugCanvas = new T2GUIElement;

	if ( m_pDebugCanvas )
	{
		m_pDebugCanvas->SetDimensions( 640.0f, 512.0f );
		m_pDebugCanvas->SetTransform( 0.0f, 0.0f, 0.0f );
	}

	m_pRootElement = m_pCreatedRootElement;
}

// $Barnyard: FUNCTION 006c47f0
void T2GUIContext::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pRootElement )
	{
		m_pRootElement->Tick( a_fDeltaTime );
	}

	m_pDebugCanvas->Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006c4820
void T2GUIContext::Render()
{
	if ( m_pRootElement )
	{
		T2GUI::GetRenderer()->BeginScene();

		m_pRootElement->PreRender();
		m_pRootElement->Render();
		m_pRootElement->PostRender();

		m_pDebugCanvas->PreRender();
		m_pDebugCanvas->Render();
		m_pDebugCanvas->PostRender();

		T2GUI::GetRenderer()->EndScene();
	}
}

TOSHI_NAMESPACE_END
