#include "pch.h"

#if defined( TRENDERINTERFACE_DX8 ) || defined( TRENDERINTERFACE_DX11 )
#  include "AImGuiState.h"
#  include "AImGui.h"
#  include "Tasks/ARootTask.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AImGuiState );

AImGuiState::AImGuiState()
    : m_bWasPaused( TFALSE )
{
}

TBOOL AImGuiState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// The input goes to ImGui, see AImGui::ProcessInput
	return TFALSE;
}

TBOOL AImGuiState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return TTRUE;
}

void AImGuiState::OnInsertion()
{
	m_bWasPaused = ARootTask::GetSingleton()->IsPaused();
	ARootTask::GetSingleton()->SetPaused( TTRUE );

	BaseClass::OnInsertion();
}

void AImGuiState::OnRemoval()
{
	if ( !m_bWasPaused )
		ARootTask::GetSingleton()->SetPaused( TFALSE );

	BaseClass::OnRemoval();
}

void AImGuiState::OnActivate()
{
	AImGui::GetSingleton()->Enable( TTRUE );
	AImGui::GetSingleton()->LockInput();

	BaseClass::OnActivate();
}

void AImGuiState::OnDeactivate()
{
	AImGui::GetSingleton()->Enable( TFALSE );
	AImGui::GetSingleton()->UnlockInput();

	BaseClass::OnDeactivate();
}

#endif // TRENDERINTERFACE_DX8 || TRENDERINTERFACE_DX11
