#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Context.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// $Barnyard: FUNCTION 006c4700
AGUI2Context::AGUI2Context()
{
	m_pRenderer           = TNULL;
	m_pRootElement        = TNULL;
	m_pCreatedRootElement = TNULL;
	m_pDebugCanvas        = TNULL;

	m_pCreatedRootElement = new AGUI2Element;

	if ( m_pCreatedRootElement )
	{
		m_pCreatedRootElement->SetDimensions( 640.0f, 512.0f );
		m_pCreatedRootElement->SetTransform( 0.0f, 0.0f, 0.0f );
	}

	m_pDebugCanvas = new AGUI2Element;

	if ( m_pDebugCanvas )
	{
		m_pDebugCanvas->SetDimensions( 640.0f, 512.0f );
		m_pDebugCanvas->SetTransform( 0.0f, 0.0f, 0.0f );
	}

	m_pRootElement = m_pCreatedRootElement;
}

// $Barnyard: FUNCTION 006c47f0
void AGUI2Context::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pRootElement )
	{
		m_pRootElement->Tick( a_fDeltaTime );
	}

	m_pDebugCanvas->Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006c4820
void AGUI2Context::Render()
{
	if ( m_pRootElement )
	{
		AGUI2::GetRenderer()->BeginScene();

		m_pRootElement->PreRender();
		m_pRootElement->Render();
		m_pRootElement->PostRender();

		m_pDebugCanvas->PreRender();
		m_pDebugCanvas->Render();
		m_pDebugCanvas->PostRender();

		AGUI2::GetRenderer()->EndScene();
	}
}
