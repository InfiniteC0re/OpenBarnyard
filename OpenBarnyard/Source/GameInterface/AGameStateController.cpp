#include "pch.h"
#include "AGameStateController.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "ACheatActivator.h"

TOSHI_NAMESPACE_USING

AGameStateController::AGameStateController() :
	m_UnkVector(m_VectorData, sizeof(m_VectorData) / sizeof(*m_VectorData), 0)
{
	// Incomplete
	m_eFlags = 0;
}

AGameStateController::~AGameStateController()
{
	TASSERT(m_oStates.Size() == 0);
}

TBOOL AGameStateController::OnCreate()
{
	InsertGameState(new AGameState);

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);

	m_Rectangle.SetDimensions(fWidth, fHeight);
	m_Rectangle.SetAttachment(AGUI2Element::Anchor_TopCenter, AGUI2Element::Pivot_TopCenter);
	m_Rectangle.SetInFront();
	m_Rectangle.Hide();

	return TTRUE;
}

TBOOL AGameStateController::OnUpdate(TFLOAT a_fDeltaTime)
{
	auto eFlags = m_eFlags;

	if (eFlags & 0x10)
	{
		m_eFlags = eFlags & 0xffef;

		if (HASFLAG(eFlags & 0x1000))
		{
			TTODO("FUN_00424000(false)");
			return TTRUE;
		}
		else
		{
			TTODO("FUN_00423de0((char *)0x0,true,(TPooledString8 *)0x0)");
			return TTRUE;
		}
	}

	if (m_oStates.Size() > 1)
	{
		TTODO("This section");

		if (!HASFLAG(m_eFlags & 7))
		{
			m_oStates.Back()->OnUpdate(a_fDeltaTime);
		}
	}

	UpdateScreenOverlay();
	return TTRUE;
}

void AGameStateController::OnDestroy()
{
	TASSERT(m_oStates.Size() == 1);

	auto pGameState = m_oStates.Back();
	m_oStates.PopBack();
	pGameState->Destroy();
}

void AGameStateController::InsertGameState(AGameState* a_pGameState)
{
	TGlobalEmitter<AGameStateControllerEvent>::Throw(
		AGameStateControllerEvent(a_pGameState, AGameStateControllerEvent::Type_GameStateBeingAdded)
	);

	m_oStates.PushBack(a_pGameState);
	a_pGameState->OnInsertion();
	a_pGameState->Activate();
}

void AGameStateController::ReplaceState(AGameState* a_pGameState)
{
	if (m_oStates.Size() > 1)
	{
		auto pCurrentGameState = m_oStates.Back();
		m_oStates.PopBack();

		pCurrentGameState->Destroy();
	}

	InsertGameState(a_pGameState);
}

void AGameStateController::PushState(AGameState* a_pGameState)
{
	if (m_oStates.Size() > 1)
	{
		m_oStates.Back()->Deactivate();
		m_oStates.Back()->Suspend();
	}

	InsertGameState(a_pGameState);
}

void AGameStateController::PopState(AGameState* a_pGameState)
{
	auto pCurrentGameState = m_oStates.Back();

	TASSERT(a_pGameState == pCurrentGameState);
	m_oStates.PopBack();

	pCurrentGameState->Destroy();

	if (m_oStates.Size() == 1 && ARootTask::GetSingleton()->IsGameSystemCreated())
	{
		g_oTheApp.Destroy();
		return;
	}

	if (m_oStates.Size() > 1)
	{
		pCurrentGameState = m_oStates.Back();
		pCurrentGameState->OnResume(a_pGameState);
		pCurrentGameState->Activate();
	}
}

void AGameStateController::PopCurrentGameState()
{
	TASSERT(m_oStates.Size() > 1);

	if (m_oStates.Size() > 1)
	{
		PopState(m_oStates.Back());
	}
}

void AGameStateController::UpdateScreenOverlay()
{
	TBOOL bAddOverlay = TFALSE;

	if (m_oStates.Size() > 1)
	{
		TFIXME("Set bAddElement and m_Rectangle color if needed");
	}

	m_Rectangle.Unlink();

	if (bAddOverlay)
	{
		AGUI2::GetRootElement()->AddChildTail(m_Rectangle);
		m_Rectangle.Show();
	}
	else
	{
		m_Rectangle.Hide();
	}
}

void AGameStateController::ResetStack()
{
	UpdateScreenOverlay();
	m_eFlags = 0;

	TBOOL bIsFirst = TTRUE;

	while (1 < m_oStates.Size())
	{
		auto pGameState = m_oStates.Back();
		m_oStates.PopBack();

		pGameState->Destroy(bIsFirst);
		bIsFirst = TFALSE;
	}
}

TBOOL AGameStateController::ProcessInput(const Toshi::TInputInterface::InputEvent* a_pEvent)
{
	if (m_eFlags & 1)
	{
		return TTRUE;
	}

	if (m_oStates.Size() <= 1)
	{
		return TFALSE;
	}

	if (ACheatActivator::IsSingletonCreated())
	{
		TIMPLEMENT_D("Cheat code activation");
	}

	auto pGameState = m_oStates.Back();
	
	if (pGameState->SendInputCommands(a_pEvent))
	{
		return TTRUE;
	}
	else
	{
		return pGameState->ProcessInput(a_pEvent);
	}
}
