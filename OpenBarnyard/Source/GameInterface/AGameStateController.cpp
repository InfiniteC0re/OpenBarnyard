#include "pch.h"
#include "AGameStateController.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "ACheatActivator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGameStateController );

AGameStateController::AGameStateController()
    : m_UnkVector( m_VectorData, TARRAYSIZE( m_VectorData ), 0 )
{
	// Incomplete
	m_eFlags       = 0;
	m_fOverlayGoal = 0.0f;
}

AGameStateController::~AGameStateController()
{
	TASSERT( m_oStates.Size() == 0 );
}

TBOOL AGameStateController::OnCreate()
{
	InsertGameState( new AGameState );

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_oOverlay.SetDimensions( fWidth, fHeight );
	m_oOverlay.SetAttachment( AGUI2ATTACHMENT_BOTTOMCENTER, AGUI2ATTACHMENT_BOTTOMCENTER );
	m_oOverlay.SetInFront();
	m_oOverlay.Hide();

	return TTRUE;
}

TBOOL AGameStateController::OnUpdate( TFLOAT a_fDeltaTime )
{
	auto eFlags = m_eFlags;

	if ( eFlags & 0x10 )
	{
		m_eFlags = eFlags & 0xffef;

		if ( HASANYFLAG( eFlags, 0x1000 ) )
		{
			TTODO( "FUN_00424000(false)" );
			return TTRUE;
		}
		else
		{
			TTODO( "FUN_00423de0((TCHAR *)0x0,true,(TPooledString8 *)0x0)" );
			return TTRUE;
		}
	}

	if ( m_oStates.Size() > 1 )
	{
		TTODO( "This section" );

		if ( !HASANYFLAG( m_eFlags, 7 ) )
		{
			( *m_oStates.Back() )->OnUpdate( a_fDeltaTime );
		}
	}

	UpdateScreenOverlay();
	return TTRUE;
}

void AGameStateController::OnDestroy()
{
	TASSERT( m_oStates.Size() == 1 );

	auto pGameState = *m_oStates.Back();
	m_oStates.PopBack();

	pGameState->Destroy();
}

void AGameStateController::InsertGameState( AGameState* a_pGameState )
{
	TGlobalEmitter<AGameStateControllerEvent>::Throw(
	    AGameStateControllerEvent( a_pGameState, AGameStateControllerEvent::Type_GameStateBeingAdded )
	);

	m_oStates.PushBack( a_pGameState );
	a_pGameState->OnInsertion();
	a_pGameState->Activate();
}

void AGameStateController::ReplaceState( AGameState* a_pGameState )
{
	if ( m_oStates.Size() > 1 )
	{
		auto pCurrentGameState = *m_oStates.Back();
		m_oStates.PopBack();

		pCurrentGameState->Destroy();
	}

	InsertGameState( a_pGameState );
}

void AGameStateController::PushState( AGameState* a_pGameState )
{
	if ( m_oStates.Size() > 1 )
	{
		( *m_oStates.Back() )->Deactivate();
		( *m_oStates.Back() )->Suspend();
	}

	InsertGameState( a_pGameState );
}

void AGameStateController::PopState( AGameState* a_pGameState )
{
	auto pCurrentGameState = *m_oStates.Back();

	TASSERT( a_pGameState == pCurrentGameState );
	m_oStates.PopBack();

	pCurrentGameState->Destroy();

	if ( m_oStates.Size() == 1 && ARootTask::GetSingleton()->IsGameSystemCreated() )
	{
		g_oTheApp.Destroy();
		return;
	}

	if ( m_oStates.Size() > 1 )
	{
		pCurrentGameState = *m_oStates.Back();
		pCurrentGameState->OnResume( a_pGameState );
		pCurrentGameState->Activate();
	}
}

void AGameStateController::PopCurrentGameState()
{
	TASSERT( m_oStates.Size() > 1 );

	if ( m_oStates.Size() > 1 )
	{
		PopState( *m_oStates.Back() );
	}
}

void AGameStateController::UpdateScreenOverlay()
{
	TBOOL bAddOverlay = TFALSE;

	if ( m_oStates.Size() > 1 && ( *m_oStates.Back() )->GetOverlay() != AGameState::OVERLAY_1 )
	{
		OverlayData* pOverlay = GetOverlayParams( ( *m_oStates.Back() )->GetOverlay() );
		m_oOverlay.SetColour( TCOLOR_ALPHA( pOverlay->uiColorR, pOverlay->uiColorG, pOverlay->uiColorB, pOverlay->uiColorA ) );

		bAddOverlay = TTRUE;
	}

	m_oOverlay.RemoveSelf();

	if ( bAddOverlay )
	{
		AGUI2::GetRootElement()->AddChildTail( m_oOverlay );
		m_oOverlay.Show();
	}
	else
	{
		m_oOverlay.Hide();
	}
}

void AGameStateController::ResetStack()
{
	UpdateScreenOverlay();
	m_eFlags = 0;

	TBOOL bIsFirst = TTRUE;

	while ( 1 < m_oStates.Size() )
	{
		auto pGameState = *m_oStates.Back();
		m_oStates.PopBack();

		pGameState->Destroy( bIsFirst );
		bIsFirst = TFALSE;
	}
}

void AGameStateController::SetFlags( TUINT16 a_eFlags )
{
	m_eFlags |= a_eFlags;
	m_fSoundVolume = 1.0f;

	if ( !( m_eFlags & 256 ) )
	{
		m_fOverlayGoal  = 0.0f;
		m_fOverlayAlpha = ( a_eFlags == 4 ) ? 1.0f : 0.0f;
	}
	else
	{
		m_fOverlayAlpha = m_fOverlayGoal;
		m_eFlags &= 0xfeff;
	}

	OverlayData oData;
	oData.uiColorA = TUINT8( m_fOverlayAlpha * 255.0f );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;
	SetOverlayParams( AGameState::OVERLAY_3, oData );

	( *m_oStates.Back() )->SetOverlay( AGameState::OVERLAY_3 );
	UpdateScreenOverlay();
}

void AGameStateController::SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	ms_aOverlays[ a_eOverlay ] = a_rcParams;
}

AGameStateController::OverlayData* AGameStateController::GetOverlayParams( AGameState::OVERLAY a_eOverlay )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	return &ms_aOverlays[ a_eOverlay ];
}

TBOOL AGameStateController::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	if ( m_eFlags & 1 )
	{
		return TTRUE;
	}

	if ( m_oStates.Size() <= 1 )
	{
		return TFALSE;
	}

	if ( ACheatActivator::IsSingletonCreated() )
	{
		TIMPLEMENT_D( "Cheat code activation" );
	}

	auto pGameState = *m_oStates.Back();

	if ( pGameState->SendInputCommands( a_pEvent ) )
	{
		return TTRUE;
	}
	else
	{
		return pGameState->ProcessInput( a_pEvent );
	}
}
