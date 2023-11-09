#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Context.h"

AGUI2Context::AGUI2Context()
{
	m_pRenderer = TNULL;
	m_pRootElement = TNULL;
	m_pCreatedRootElement = TNULL;
	m_pOverlayElement = TNULL;

	m_pCreatedRootElement = new AGUI2Element;

	if (m_pCreatedRootElement)
	{
		m_pCreatedRootElement->SetDimensions(640.0f, 512.0f);
		m_pCreatedRootElement->SetTransform(0.0f, 0.0f, 0.0f);
	}

	m_pOverlayElement = new AGUI2Element;

	if (m_pOverlayElement)
	{
		m_pOverlayElement->SetDimensions(640.0f, 512.0f);
		m_pOverlayElement->SetTransform(0.0f, 0.0f, 0.0f);
	}

	m_pRootElement = m_pCreatedRootElement;
}

void AGUI2Context::Tick(TFLOAT a_fDeltaTime)
{
	if (m_pRootElement)
	{
		m_pRootElement->Tick(a_fDeltaTime);
	}

	m_pOverlayElement->Tick(a_fDeltaTime);
}

void AGUI2Context::Render()
{
	if (m_pRootElement)
	{
		AGUI2::GetRenderer()->BeginScene();

		m_pRootElement->PreRender();
		m_pRootElement->Render();
		m_pRootElement->PostRender();

		m_pOverlayElement->PreRender();
		m_pOverlayElement->Render();
		m_pOverlayElement->PostRender();

		AGUI2::GetRenderer()->EndScene();
	}
}
