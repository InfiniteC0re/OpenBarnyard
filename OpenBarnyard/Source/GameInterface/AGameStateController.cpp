#include "pch.h"
#include "AGameStateController.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "ACheatActivator.h"
#include "ALoadScreen.h"
#include "Cameras/ACameraManager.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGameStateController );

static TINT          s_iCurrMiniGame       = -1;
static TINT          s_iNextMiniGame       = -1;
static TBOOL         s_bLoadingMiniGame    = TFALSE;
static const TClass* s_pPrevGameStateClass = TNULL;

// $Barnyard: FUNCTION 00429250
AGameStateController::AGameStateController()
    : m_UnkVector( m_VectorData, TARRAYSIZE( m_VectorData ), 0 )
{
	// Incomplete
	m_eFlagsRaw     = 0;
	m_flOverlayGoal = 0.0f;
}

// $Barnyard: FUNCTION 004292f0
AGameStateController::~AGameStateController()
{
	TASSERT( m_oStates.Size() == 0 );
}

// $Barnyard: FUNCTION 00429e90
TBOOL AGameStateController::OnCreate()
{
	InsertGameState( new AGameState );

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_oOverlay.SetDimensions( fWidth, fHeight );
	m_oOverlay.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oOverlay.SetInFront();
	m_oOverlay.Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 0042a770
TBOOL AGameStateController::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_eFlagsRaw & FLAGS_UNK4 )
	{
		m_eFlagsRaw &= ~FLAGS_UNK4;

		if ( HASANYFLAG( m_eFlagsRaw, FLAGS_UNK12 ) )
		{
			TTODO( "FUN_00424000(TFALSE)" );
			return TTRUE;
		}
		else
		{
			TTODO( "FUN_00423de0((TCHAR *)0x0,TTRUE,(TPooledString8 *)0x0)" );
			return TTRUE;
		}
	}

	if ( m_oStates.Size() > 1 )
	{
		// Update transition state first
		if ( UpdateTransition( a_fDeltaTime ) )
			return TTRUE;

		// If no transition and input is allowed, update gamestate
		if ( !HASANYFLAG( m_eFlagsRaw, FLAGS_IGNORE_INPUT | FLAGS_AWAITING_TRANSITION | FLAGS_FADING_IN ) )
			( *m_oStates.Back() )->OnUpdate( a_fDeltaTime );
	}

	UpdateScreenOverlay();
	return TTRUE;
}

// $Barnyard: FUNCTION 00429380
void AGameStateController::OnDestroy()
{
	TASSERT( m_oStates.Size() == 1 );

	auto pGameState = *m_oStates.Back();
	m_oStates.PopBack();

	pGameState->Destroy();
}

// $Barnyard: FUNCTION 00429b00
void AGameStateController::InsertGameState( AGameState* a_pGameState )
{
	TGlobalEmitter<AGameStateControllerEvent>::Throw(
	    AGameStateControllerEvent( a_pGameState, AGameStateControllerEvent::Type_GameStateBeingAdded )
	);

	m_oStates.PushBack( a_pGameState );
	a_pGameState->OnInsertion();
	a_pGameState->Activate();
}

// $Barnyard: FUNCTION 0042a090
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

// $Barnyard: FUNCTION 00429f50
void AGameStateController::PushState( AGameState* a_pGameState )
{
	if ( m_oStates.Size() > 1 )
	{
		( *m_oStates.Back() )->Deactivate();
		( *m_oStates.Back() )->Suspend();
	}

	InsertGameState( a_pGameState );
}

// $Barnyard: FUNCTION 00429fb0
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

// $Barnyard: FUNCTION 0042a870
void AGameStateController::PopCurrentGameState()
{
	TASSERT( m_oStates.Size() > 1 );

	if ( m_oStates.Size() > 1 )
	{
		PopState( *m_oStates.Back() );
	}
}

// $Barnyard: FUNCTION 00429490
void AGameStateController::UpdateScreenOverlay()
{
	TBOOL bAddOverlay = TFALSE;

	if ( m_oStates.Size() > 1 && ( *m_oStates.Back() )->GetOverlay() != AGameState::OVERLAY_1 )
	{
		OverlayData* pOverlay = GetOverlayParams( ( *m_oStates.Back() )->GetOverlay() );
		m_oOverlay.SetColour( TCOLOR4( pOverlay->uiColorR, pOverlay->uiColorG, pOverlay->uiColorB, pOverlay->uiColorA ) );

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

// $Barnyard: FUNCTION 0042a150
void AGameStateController::ResetStack()
{
	UpdateScreenOverlay();
	m_eFlagsRaw = 0;

	TBOOL bIsFirst = TTRUE;

	while ( 1 < m_oStates.Size() )
	{
		auto pGameState = *m_oStates.Back();
		m_oStates.PopBack();

		pGameState->Destroy( bIsFirst );
		bIsFirst = TFALSE;
	}
}

// $Barnyard: FUNCTION 00429580
void AGameStateController::SetFlags( TUINT16 a_eFlags )
{
	m_eFlagsRaw |= a_eFlags;
	m_flSoundVolume = 1.0f;

	if ( m_eFlagsRaw & FLAGS_UNK8 )
	{
		m_eFlagsRaw &= ~FLAGS_UNK8;
		m_flOverlayAlpha = m_flOverlayGoal;
	}
	else
	{
		m_flOverlayGoal  = 0.0f;
		m_flOverlayAlpha = ( a_eFlags == 4 ) ? 1.0f : 0.0f;
	}

	OverlayData oData;
	oData.uiColorA = TUINT8( m_flOverlayAlpha * 255.0f );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;
	SetOverlayParams( AGameState::OVERLAY_3, oData );

	( *m_oStates.Back() )->SetOverlay( AGameState::OVERLAY_3 );
	UpdateScreenOverlay();
}

// $Barnyard: FUNCTION 004280b0
void AGameStateController::SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	ms_aOverlays[ a_eOverlay ] = a_rcParams;
}

// $Barnyard: FUNCTION 004280a0
AGameStateController::OverlayData* AGameStateController::GetOverlayParams( AGameState::OVERLAY a_eOverlay )
{
	TASSERT( a_eOverlay < AGameState::OVERLAY_NUMOF );
	return &ms_aOverlays[ a_eOverlay ];
}

// $Barnyard: FUNCTION 0042a320
TBOOL AGameStateController::UpdateTransition( TFLOAT a_flDeltaTime )
{
	TASSERT( m_oStates.Size() >= 1 );
	AGameState* pCurrentState = *m_oStates.Back();

	if ( m_eFlagsRaw & FLAGS_AWAITING_TRANSITION )
	{
		// Transition is planned
		m_flOverlayAlpha = a_flDeltaTime + a_flDeltaTime + m_flOverlayAlpha;

		// Update volume if needed
		if ( m_eFlagsRaw & FLAGS_MUTING )
		{
			m_flSoundVolume = m_flSoundVolume - ( a_flDeltaTime + a_flDeltaTime );
			ASoundManager::GetSingleton()->SetGlobalVolume( m_flSoundVolume );
		}

		// Check if the transition is over
		if ( m_flOverlayAlpha > 1.0 )
		{
			// Screen is black, can change game state
			m_eFlagsRaw &= ~( FLAGS_AWAITING_TRANSITION | FLAGS_MUTING );
			m_flOverlayAlpha = 1.0;
			UpdateScreenOverlay();
			pCurrentState->m_eOverlayColorIndex = 0;

			if ( m_eFlags.bIgnoreInput )
			{
				// Input is ignored
				pCurrentState->m_HUDParams.SetFlags( 0 );
				TTODO( "Update AHudManager?" );
				//( **(code**)( *AHudManager::ms_pInstance + 0x1c ) )( a_flDeltaTime );

				m_eFlags.bIgnoreInput = TFALSE;

				if ( m_eFlagsRaw & FLAGS_UNK3 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK3;
					TASSERT( TFALSE && "FUN_00423de0" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_TRANSITION_TO_MINIGAME )
				{
					m_eFlagsRaw &= ~FLAGS_TRANSITION_TO_MINIGAME;
					StartMiniGame( s_iNextMiniGame, TTRUE );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_UNK7 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK7;
					TASSERT( TFALSE && "FUN_00425410" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_UNK9 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK9;
					TASSERT( TFALSE && "FUN_00423c70" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_UNK11 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK11;
					pCurrentState->Remove();
					TASSERT( TFALSE && "FUN_00429c00" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				if ( m_eFlagsRaw & FLAGS_UNK12 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK12;
					(*m_oStates.Back())->m_GUIElement.Hide();

					TASSERT( TFALSE && "FUN_00424000" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}

				pCurrentState->m_GUIElement.Hide();
				TASSERT( TFALSE && "FUN_004257f0" );

				m_eFlags.UNK10 = TFALSE;
				return TTRUE;
			}
			else
			{
				// Input is not ignored
				if ( m_eFlagsRaw & FLAGS_UNK5 )
				{
					m_eFlagsRaw &= ~FLAGS_UNK5;
					TASSERT( TFALSE && "FUN_00423910" );

					m_eFlags.UNK10 = TFALSE;
					return TTRUE;
				}
			}
		}
	}
	else
	{
		// No transition
		if ( m_eFlagsRaw & FLAGS_FADING_IN )
		{
			m_flOverlayAlpha = m_flOverlayAlpha - ( a_flDeltaTime + a_flDeltaTime );

			if ( m_flOverlayAlpha < 0.0 )
			{
				m_eFlagsRaw &= ~FLAGS_FADING_IN;
				m_flOverlayAlpha = 0.0;
				UpdateScreenOverlay();
				pCurrentState->m_eOverlayColorIndex = 0;

				TTODO( "Non critical AQuestManager things here" );

				// 				if ( ( ( AQuestManager::ms_pSingleton != 0 ) &&
				// 				       ( *(int*)( AQuestManager::ms_pSingleton + 0x1bc ) == 3 ) ) &&
				// 				     ( *(int*)( AQuestManager::ms_pSingleton + 0x1b8 ) == 0x10 ) )
				// 				{
				// 					pTVar9 = &SaveLoadSKU::s_Class;
				// 					pTVar6 = (TClass*)( *(code*)*pCurrentState->vftable )();
				// 					bVar5  = Toshi::TClass::IsA( pTVar6, pTVar9 );
				// 					if ( ( bVar5 ) &&
				// 					     ( pCurrentState = FindGameStateOnStack( &ASimState ), pCurrentState != (AGameState*)0x0 ) )
				// 					{
				// 						pCurrentState->m_eOverlayColorIndex = 0;
				// 					}
				// 				}
			}
		}
	}

	// Update overlay color
	OverlayData oData;
	oData.uiColorA = TUINT8( m_flOverlayAlpha * 255.0f );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;
	SetOverlayParams( 2, oData );

	return TFALSE;
}

// $Barnyard: FUNCTION 004293d0
TBOOL AGameStateController::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	if ( m_eFlags.bIgnoreInput ) return TTRUE;

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

	return pGameState->ProcessInput( a_pEvent );
}

// $Barnyard: FUNCTION 004239d0
void AGameStateController::StartMiniGame( TINT a_iMiniGame, TBOOL a_bRightNow )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	if ( a_bRightNow )
	{
		g_oLoadScreen.StartLoading( 100, TTRUE );

		s_pPrevGameStateClass = pController->GetCurrentState()->GetClass();
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		ACameraManager::GetSingleton()->FUN_0045c290();

		ARootTask::GetSingleton()->StopRenderMainScene( TTRUE );

		s_bLoadingMiniGame = TTRUE;
		s_iCurrMiniGame    = a_iMiniGame;

		AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
		APlayerManager::GetSingleton()->ResetPlayerIterator();
	}
	else
	{
		pController->m_eFlags.bIgnoreInput          = TTRUE;
		pController->m_eFlags.bTransitionToMiniGame = TTRUE;
		pController->SetFlags( AGameStateController::FLAGS_AWAITING_TRANSITION );

		s_iNextMiniGame = a_iMiniGame;
	}
}
