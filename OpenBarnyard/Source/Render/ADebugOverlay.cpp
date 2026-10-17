#include "pch.h"
#include "ADebugOverlay.h"

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
#  include "ARenderer.h"
#  include "Cameras/ACameraManager.h"
#  include "GameInterface/AGameState.h"
#  include "Input/AInputMapManager.h"
#  include "Tasks/ARootTask.h"
#  include "Helpers/AGameTimeManager.h"
#  include "Player/APlayerManager.h"
#  include "Animation/AAnimatableObject.h"
#  include "Units/AUnitManager.h"
#  include "Units/AUnitNameManager.h"
#  include "Units/AAnimatedUnit.h"
#  include "Units/AUnitBehaviour.h"
#  include "Units/AUnitPlayer.h"
#  include "Units/AUnitType.h"
#  include "Units/Projectiles/AProjectileLauncher.h"
#  include "Units/Projectiles/ALaunchAmmo.h"
#  include "UnitMovement/Steering/ASteer.h"
#  include "UnitMovement/Steering/ASteerKinematics.h"
#  include "FSM/AFSMState.h"

#  include <Platform/DX8/TRenderInterface_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ADebugOverlay::ADebugOverlay()
    : m_pDebugText( TNULL )
    , m_fGroundTime( 0.0f )
    , m_bEnabled( TFALSE )
{
	// "-debugoverlay" starts with the overlay on
	if ( TStringManager::String8FindString( GetCommandLineA(), "-debugoverlay" ) )
		m_bEnabled = TTRUE;
}

ADebugOverlay::~ADebugOverlay()
{
	if ( m_pDebugText )
	{
		TRenderD3DInterface::Interface()->DestroyDebugText();
		m_pDebugText = TNULL;
	}
}

void ADebugOverlay::SetEnabled( TBOOL a_bEnabled )
{
	m_bEnabled = a_bEnabled;
}

void ADebugOverlay::ProcessInput( AGameState* a_pGameState, const TInputInterface::InputEvent* a_pEvent )
{
	if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || !a_pGameState->GetInputMap() )
		return;

	// The controls file binds DebugMenu to the ` key (and the pad's Select) in almost every input map
	AInputMapManager* pInputManager = AInputMapManager::GetSingleton();
	pInputManager->PushInputMap( a_pGameState->GetInputMap() );

	AInputCommandArray oCommands;
	pInputManager->GetEventCommands( a_pEvent, oCommands );
	pInputManager->PopInputMap();

	for ( TINT i = 0; i < oCommands.iNumCommands; i++ )
	{
		if ( oCommands.aCommands[ i ] == AInputCommand_DebugMenu )
		{
			SetEnabled( !m_bEnabled );
			break;
		}
	}

	oCommands.Clear();
}

void ADebugOverlay::Render( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return;

	TRenderD3DInterface* pRender = TRenderD3DInterface::Interface();

	if ( !m_pDebugText )
	{
		m_pDebugText = pRender->CreateDebugText();

		if ( !m_pDebugText )
			return;
	}

	ARootTask* pRootTask = ARootTask::GetSingleton();

	if ( pRootTask->IsGameSystemCreated() )
	{
		RenderPlayer( a_fDeltaTime );

		if ( AGameTimeManager::IsSingletonCreated() )
			RenderGameTime();

		ACamera* pCamera = ACameraManager::IsSingletonCreated() ? ACameraManager::GetSingleton()->GetCurrentCamera() : TNULL;

		if ( pCamera )
		{
			const TVector4& rcCameraPos = pCamera->GetMatrix().GetTranslation();

			TCHAR szLine[ 128 ];
			TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Camera Position(%f,%f,%f)", rcCameraPos.x, rcCameraPos.y, rcCameraPos.z );
			m_pDebugText->AddString( szLine, INFO_COLOUR );

			// Names and states of the units
			if ( pRootTask->IsRenderWorld() )
				RenderUnitLabels( pCamera );
		}
	}

	// Frame time and FPS at the bottom
	if ( a_fDeltaTime > 0.0f )
	{
		TRenderInterface::DISPLAYPARAMS* pDisplayParams = pRender->GetCurrentDisplayParams();

		TCHAR szLine[ 64 ];
		TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Total = %.3f (%.1f FPS)", a_fDeltaTime * 1000.0f, 1.0f / a_fDeltaTime );
		m_pDebugText->PrintString( pDisplayParams->uiWidth / 2, pDisplayParams->uiHeight - LINE_HEIGHT * 2, szLine, INFO_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );
	}

	m_pDebugText->Render();
}

void ADebugOverlay::RenderPlayer( TFLOAT a_fDeltaTime )
{
	AUnit* pPlayer = APlayerManager::IsSingletonCreated() ? APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) : TNULL;

	if ( !pPlayer )
		return;

	TCHAR szLine[ 128 ];

	// Time since the player landed
	ASteerKinematics* pKinematics = pPlayer->GetSteer() ? pPlayer->GetSteer()->GetKinematics() : TNULL;

	if ( pKinematics )
	{
		// Same limit as the jump check of APlayerSteer
		if ( pKinematics->GetAirTime() >= 0.2f )
			m_fGroundTime = 0.0f;
		else
			m_fGroundTime += a_fDeltaTime;

		TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "GroundTime = [%f]", m_fGroundTime );
		m_pDebugText->AddString( szLine, INFO_COLOUR );
	}

	// Milk of the active launcher
	AProjectileLauncher* pLauncher = pPlayer->IsA( &TGetClass( AUnitPlayer ) ) ? TSTATICCAST( AUnitPlayer, pPlayer )->GetActiveLauncher() : TNULL;

	if ( pLauncher && pLauncher->GetAmmo() )
	{
		ALaunchAmmo* pAmmo = pLauncher->GetAmmo();

		TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Charge: %f.2, of %f.2 / %f.2", pAmmo->GetAmmo(), TFLOAT( pAmmo->GetMaxAmmo() ), pAmmo->GetRechargeRate() );
		m_pDebugText->AddString( szLine, INFO_COLOUR );
	}

	// Animations
	if ( pPlayer->IsA( &TGetClass( AAnimatedUnit ) ) )
	{
		AAnimatableObject* pObject = TSTATICCAST( AAnimatedUnit, pPlayer )->GetAnimatableObject();

		if ( pObject )
			RenderAnimations( pObject );
	}

	// The velocity the player's steer is asked for (the movement input turned into a direction and a speed)
	if ( pKinematics )
	{
		const TVector4* pFacing = pKinematics->GetDesiredFacing();
		TFLOAT          fSpeed  = pKinematics->GetDesiredSpeed();

		TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "ManualSteer(%f,%f,%f)", pFacing->x * fSpeed, pFacing->y * fSpeed, pFacing->z * fSpeed );
		m_pDebugText->AddString( szLine, STEER_COLOUR );
	}
}

void ADebugOverlay::RenderAnimations( AAnimatableObject* a_pObject )
{
	AToshiAnimationInterface& rAnimInterface = a_pObject->GetToshiAnimInterface();

	for ( TINT i = 0; i < 2; i++ )
	{
		TBOOL                        bOverlay = ( i == 1 );
		T2DList<AToshiAnimationRef>& rList    = bOverlay ? rAnimInterface.GetOverlayAnimRefs() : rAnimInterface.GetAnimRefs();

		for ( auto it = rList.Begin(); it != rList.End(); it++ )
		{
			AToshiAnimationRef* pRef       = it;
			ANamedAnimation*    pNamedAnim = pRef->GetNamedAnimation();

			if ( !pNamedAnim )
				continue;

			// [Base] idle_lp (Idle) [Time = 0.796 / 1.833] [Weight = 1.000] [Speed = 1] (Loop) [Next Break = 0]
			TCHAR szLine[ 256 ];
			TStringManager::String8Format(
			    szLine,
			    TARRAYSIZE( szLine ),
			    "[%s] %s (%s) [Time = %.3f / %.3f] [Weight = %.3f] [Speed = %g] (%s) [Next Break = %d]",
			    bOverlay ? "Overlay" : "Base",
			    pNamedAnim->GetName().GetString(),
			    pNamedAnim->GetExportedName().GetString(),
			    rAnimInterface.GetAnimTime( pRef ),
			    rAnimInterface.GetAnimDuration( pRef ),
			    rAnimInterface.GetAnimWeight( pRef ),
			    rAnimInterface.GetAnimSpeed( pRef ),
			    rAnimInterface.IsAnimLooping( pRef ) ? "Loop" : "Once",
			    pRef->GetBreakpointIndex()
			);

			m_pDebugText->AddString( szLine, ANIMATION_COLOUR );
		}
	}
}

void ADebugOverlay::RenderGameTime()
{
	AGameTimeManager* pTimeManager = AGameTimeManager::GetSingleton();

	TCHAR szLine[ 128 ];

	TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Total Actual Time = %.3f", pTimeManager->GetTotalTime() );
	m_pDebugText->AddString( szLine, INFO_COLOUR );

	TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Time of Day: %.3f", pTimeManager->GetDayTime() );
	m_pDebugText->AddString( szLine, INFO_COLOUR );

	TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Day/Month/Year: %d/%d/%d", pTimeManager->GetDay(), pTimeManager->GetMonth(), pTimeManager->GetYear() );
	m_pDebugText->AddString( szLine, INFO_COLOUR );

	TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Day Phase: %d", pTimeManager->GetCurrentDayPhase() );
	m_pDebugText->AddString( szLine, INFO_COLOUR );

	TStringManager::String8Format( szLine, TARRAYSIZE( szLine ), "Hour/Minute/Second: %d:%d:%d", pTimeManager->GetCurrentHour(), pTimeManager->GetCurrentMinute(), pTimeManager->GetCurrentSecond() );
	m_pDebugText->AddString( szLine, INFO_COLOUR );
}

void ADebugOverlay::RenderUnitLabels( ACamera* a_pCamera )
{
	if ( !AUnitManager::IsSingletonCreated() )
		return;

	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderD3DInterface::Interface()->GetCurrentDisplayParams();
	ARenderer*                       pRenderer      = ARenderer::GetSingleton();
	const TVector4&                  rcCameraPos    = a_pCamera->GetMatrix().GetTranslation();

	TFLOAT fScreenWidth  = TFLOAT( pDisplayParams->uiWidth );
	TFLOAT fScreenHeight = TFLOAT( pDisplayParams->uiHeight );

	auto& llUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		AUnit* pUnit = it;

		if ( pUnit->IsDying() || !pUnit->GetSteer() )
			continue;

		const TVector4* pPosition = pUnit->GetSteer()->GetPosition();

		TFLOAT fX = pPosition->x - rcCameraPos.x;
		TFLOAT fY = pPosition->y - rcCameraPos.y;
		TFLOAT fZ = pPosition->z - rcCameraPos.z;

		if ( fX * fX + fY * fY + fZ * fZ > MAX_UNIT_DISTANCE * MAX_UNIT_DISTANCE )
			continue;

		// The name and the state above the unit (-Y is up)
		TVector4 vAbove( pPosition->x, pPosition->y - UNIT_LABEL_HEIGHT, pPosition->z, 1.0f );
		TVector2 vLabelPos;

		if ( !pRenderer->ProjectToScreen( a_pCamera, vAbove, vLabelPos ) ||
		     vLabelPos.x < 0.0f || vLabelPos.x >= fScreenWidth || vLabelPos.y < 0.0f || vLabelPos.y >= fScreenHeight )
			continue;

		TPString8 strName;

		if ( AUnitNameManager::IsSingletonCreated() )
			strName = AUnitNameManager::GetSingleton()->GetUnitName( pUnit->GetId() );

		if ( strName.IsEmpty() )
			strName = pUnit->GetType()->GetName();

		TINT iX = TINT( vLabelPos.x );
		TINT iY = TINT( vLabelPos.y );
		m_pDebugText->PrintString( iX, iY, strName.GetString(), UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );

		AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponent( &TGetClass( AUnitBehaviour ) ) );
		AFSMState*      pState     = pBehaviour ? pBehaviour->GetFSMInstance()->GetCurrentState() : TNULL;

		if ( pState )
			m_pDebugText->PrintString( iX, iY + LINE_HEIGHT, pState->GetName().GetString(), UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );

		TVector4 vFeet( pPosition->x, pPosition->y, pPosition->z, 1.0f );
		TVector2 vFeetPos;

		if ( pBehaviour && pRenderer->ProjectToScreen( a_pCamera, vFeet, vFeetPos ) )
		{
			TCHAR szDepth[ 16 ];
			TStringManager::String8Format( szDepth, TARRAYSIZE( szDepth ), "%d", pBehaviour->GetFSMInstance()->GetStackDepth() );
			m_pDebugText->PrintString( TINT( vFeetPos.x ), TINT( vFeetPos.y ), szDepth, UNIT_COLOUR, TDebugText::ENTRYFLAGS_CENTERED );
		}
	}
}

#endif // !TOSHI_FINAL && TRENDERINTERFACE_DX8
