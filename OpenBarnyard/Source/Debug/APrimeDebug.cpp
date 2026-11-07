#include "pch.h"
#include "APrimeDebug.h"

#ifdef PRIMEDEBUG
#  include <PDebugSystem.h>
#  include <Console/PConsole.h>
#  include <Console/PKeyboard.h>
#  include <Util/PString.h>

#  ifdef PRIMEDEBUG_DEV
#	include <Render/TCameraObject.h>
#	include <Render/TRenderInterface.h>
#	include <Draw/PDebugPrimitives.h>
#	include <Draw/PDebugText.h>
#	include <Draw/PDebugWindow.h>
#	include <Menu/PDebugMenu.h>
#	include <Profile/PStats.h>
#	include <Input/TInputDeviceKeyboard.h>

#	include "ACheatActivator.h"
#	include "Debug/APrimeDebugFSM.h"
#	include "Debug/APrimeDebugQuest.h"
#	include "Helpers/AGameLoader.h"
#	include "Helpers/AGameTimeManager.h"
#	include "Helpers/AWeatherManager.h"
#	include "FX/AGameTimeFXManager.h"
#	include "Cameras/ACameraManager.h"
#	include "Debug/ADebugMenuState.h"
#	include "GameInterface/ASimState.h"
#	include "GameInterface/SaveLoadSKU.h"
#	include "GUI/AGUISystem.h"
#	include "Input/AInputHandler.h"
#	include "Player/APlayerManager.h"
#	include "Player/APlayerProfileManager.h"
#	include "Debug/ADebugOverlay.h"
#	include "Debug/APrimeDebugRenderer.h"
#	include "Debug/AWorldDebugDraw.h"
#	include "Terrain/ATerrainInterface.h"
#	include "Units/AUnit.h"
#	include "UnitMovement/Steering/ASteer.h"
#	include "GUI/AGUI2.h"
#  endif

#  include "AppBoot.h"
#  include "Assets/AMaterialLibrary.h"
#  include "MiniGame/AMiniGameState.h"
#  include "Render/ASkinShader/ASkinShader.h"

#  include <stdio.h>

#  ifdef PRIMEDEBUG_DEV
#	define STB_IMAGE_WRITE_IMPLEMENTATION
#	define STB_IMAGE_WRITE_STATIC
#	include <stb_image_write.h>
#  endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#  ifdef PRIMEDEBUG_DEV

static APrimeDebugRenderer* s_pRenderer;

// The keys of the tools (PKeyboard) on the keyboard device
static const Toshi::TInputDevice::Doodad s_aKeyDoodads[ PKeyboard::KEY_NUMOF ] = {
	TInputDeviceKeyboard::KEY_0,
	TInputDeviceKeyboard::KEY_1,
	TInputDeviceKeyboard::KEY_2,
	TInputDeviceKeyboard::KEY_3,
	TInputDeviceKeyboard::KEY_4,
	TInputDeviceKeyboard::KEY_5,
	TInputDeviceKeyboard::KEY_6,
	TInputDeviceKeyboard::KEY_7,
	TInputDeviceKeyboard::KEY_8,
	TInputDeviceKeyboard::KEY_9,
	TInputDeviceKeyboard::KEY_A,
	TInputDeviceKeyboard::KEY_B,
	TInputDeviceKeyboard::KEY_C,
	TInputDeviceKeyboard::KEY_D,
	TInputDeviceKeyboard::KEY_E,
	TInputDeviceKeyboard::KEY_F,
	TInputDeviceKeyboard::KEY_G,
	TInputDeviceKeyboard::KEY_H,
	TInputDeviceKeyboard::KEY_I,
	TInputDeviceKeyboard::KEY_J,
	TInputDeviceKeyboard::KEY_K,
	TInputDeviceKeyboard::KEY_L,
	TInputDeviceKeyboard::KEY_M,
	TInputDeviceKeyboard::KEY_N,
	TInputDeviceKeyboard::KEY_O,
	TInputDeviceKeyboard::KEY_P,
	TInputDeviceKeyboard::KEY_Q,
	TInputDeviceKeyboard::KEY_R,
	TInputDeviceKeyboard::KEY_S,
	TInputDeviceKeyboard::KEY_T,
	TInputDeviceKeyboard::KEY_U,
	TInputDeviceKeyboard::KEY_V,
	TInputDeviceKeyboard::KEY_W,
	TInputDeviceKeyboard::KEY_X,
	TInputDeviceKeyboard::KEY_Y,
	TInputDeviceKeyboard::KEY_Z,
	TInputDeviceKeyboard::KEY_F1,
	TInputDeviceKeyboard::KEY_F2,
	TInputDeviceKeyboard::KEY_F3,
	TInputDeviceKeyboard::KEY_F4,
	TInputDeviceKeyboard::KEY_F5,
	TInputDeviceKeyboard::KEY_F6,
	TInputDeviceKeyboard::KEY_F7,
	TInputDeviceKeyboard::KEY_F8,
	TInputDeviceKeyboard::KEY_F9,
	TInputDeviceKeyboard::KEY_F10,
	TInputDeviceKeyboard::KEY_F11,
	TInputDeviceKeyboard::KEY_F12,
	TInputDeviceKeyboard::KEY_F13,
	TInputDeviceKeyboard::KEY_F14,
	TInputDeviceKeyboard::KEY_F15,
	TInputDeviceKeyboard::KEY_NUMPAD0,
	TInputDeviceKeyboard::KEY_NUMPAD1,
	TInputDeviceKeyboard::KEY_NUMPAD2,
	TInputDeviceKeyboard::KEY_NUMPAD3,
	TInputDeviceKeyboard::KEY_NUMPAD4,
	TInputDeviceKeyboard::KEY_NUMPAD5,
	TInputDeviceKeyboard::KEY_NUMPAD6,
	TInputDeviceKeyboard::KEY_NUMPAD7,
	TInputDeviceKeyboard::KEY_NUMPAD8,
	TInputDeviceKeyboard::KEY_NUMPAD9,
	TInputDeviceKeyboard::KEY_ESCAPE,
	TInputDeviceKeyboard::KEY_MINUS,
	TInputDeviceKeyboard::KEY_BACK,
	TInputDeviceKeyboard::KEY_TAB,
	TInputDeviceKeyboard::KEY_RETURN,
	TInputDeviceKeyboard::KEY_LCONTROL,
	TInputDeviceKeyboard::KEY_LSHIFT,
	TInputDeviceKeyboard::KEY_COMMA,
	TInputDeviceKeyboard::KEY_SLASH,
	TInputDeviceKeyboard::KEY_BACKSLASH,
	TInputDeviceKeyboard::KEY_PERIOD,
	TInputDeviceKeyboard::KEY_RSHIFT,
	TInputDeviceKeyboard::KEY_MULTIPLY,
	TInputDeviceKeyboard::KEY_LMENU,
	TInputDeviceKeyboard::KEY_SPACE,
	TInputDeviceKeyboard::KEY_CAPITAL,
	TInputDeviceKeyboard::KEY_NUMLOCK,
	TInputDeviceKeyboard::KEY_SCROLL,
	TInputDeviceKeyboard::KEY_SUBTRACT,
	TInputDeviceKeyboard::KEY_ADD,
	TInputDeviceKeyboard::KEY_DECIMAL,
	TInputDeviceKeyboard::KEY_KANA,
	TInputDeviceKeyboard::KEY_CONVERT,
	TInputDeviceKeyboard::KEY_NOCONVERT,
	TInputDeviceKeyboard::KEY_NUMPADEQUALS,
	TInputDeviceKeyboard::KEY_KANJI,
	TInputDeviceKeyboard::KEY_RCONTROL,
	TInputDeviceKeyboard::KEY_DIVIDE,
	TInputDeviceKeyboard::KEY_RMENU,
	TInputDeviceKeyboard::KEY_HOME,
	TInputDeviceKeyboard::KEY_PAUSE,
	TInputDeviceKeyboard::KEY_UP,
	TInputDeviceKeyboard::KEY_PRIOR,
	TInputDeviceKeyboard::KEY_LEFT,
	TInputDeviceKeyboard::KEY_RIGHT,
	TInputDeviceKeyboard::KEY_END,
	TInputDeviceKeyboard::KEY_DOWN,
	TInputDeviceKeyboard::KEY_NEXT,
	TInputDeviceKeyboard::KEY_INSERT,
	TInputDeviceKeyboard::KEY_DELETE,
	TInputDeviceKeyboard::KEY_LWIN,
	TInputDeviceKeyboard::KEY_RWIN,
	TInputDeviceKeyboard::KEY_APPS,
	TInputDeviceKeyboard::KEY_SLEEP,
	TInputDeviceKeyboard::KEY_GRAVE,
	TInputDeviceKeyboard::KEY_EQUALS,
	TInputDeviceKeyboard::KEY_SEMICOLON,
	TInputDeviceKeyboard::KEY_APOSTROPHE,
	TInputDeviceKeyboard::KEY_LBRACKET,
	TInputDeviceKeyboard::KEY_RBRACKET,
};

// The cheats, as ACheatActivator::CHEAT
static const TCHAR* s_aCheatNames[ ACheatActivator::CHEAT_NUMOF ] = {
	"Unlock Mud Jump",
	"Unlock Barn Pool",
	"Unlock Bike Race",
	"Unlock Chasing Chicks",
	"Unlock Veggie Patch",
	"Unlock Cow Tipping",
	"Unlock Barntender",
	"Money",
	"Items",
	"Unlock Bonus Items",
	"Unlock Antics",
};

// The variables mirroring a state of the game: their storage is set from the game every frame, a change from the
// console or the menu goes to the game through the callback
static TINT   s_iOverlay;
static TINT   s_iDayPhase;
static TINT   s_iWeather;
static TFLOAT s_fTimeScale = 1.0f;

static PCVar s_oOverlayVar;
static PCVar s_oDayPhaseVar;
static PCVar s_oWeatherVar;
static PCVar s_oTimeScaleVar;

// Settings read by the game when it needs them (AGameLoader)
static PCVar s_oFrontEndAnticsVar;
static PCVar s_oMiniGameVariantVar;
static PCVar s_oMiniGamePlayersVar;

// player.pos (x, y, z, facing in degrees) and player.teleport, which holds the position while the terrain streams in
static constexpr TFLOAT TELEPORT_HOLD_TIME = 3.0f;

static TFLOAT   s_afPlayerPos[ 4 ];
static PCVar    s_oPlayerPosVar;
static PCFunc   s_oTeleportFunc;
static PCFunc   s_oPosCopyFunc;
static TVector4 s_vTeleportPos;
static TVector4 s_vTeleportDir;
static TFLOAT   s_fTeleportHold;

#	ifdef AWORLDDEBUGDRAW
static TINT  s_aiLayers[ AWorldDebugDraw::LAYER_NUMOF ];
static PCVar s_aLayerVars[ AWorldDebugDraw::LAYER_NUMOF ];

static const TCHAR* s_aLayerVarNames[ AWorldDebugDraw::LAYER_NUMOF ][ 2 ] = {
	{ "world.draw.networks", "The interior networks: the edges of the areas, their normals and the goals" },
	{ "world.draw.steering", "The steering of the units: their radius, heading and path" },
	{ "world.draw.locators", "The terrain locators and their names" },
};
#	endif

// Screenshots: "screenshot [path]" and dev.screenshot_times (seconds since start) -> <dir>/<tag>_<seconds>.png
static constexpr TINT MAX_SHOT_TIMES = 64;

static PCVar  s_oShotDirVar;
static PCVar  s_oShotTagVar;
static PCVar  s_oShotTimesVar;
static PCFunc s_oScreenshotFunc;

static TFLOAT           s_afShotTimes[ MAX_SHOT_TIMES ];
static TBOOL            s_abShotTaken[ MAX_SHOT_TIMES ];
static TINT             s_iNumShotTimes;
static TCHAR            s_szQueuedShot[ MAX_PATH ];
static TRenderCapture*  s_pShotCapture;
static TCHAR            s_szShotFile[ MAX_PATH ];
static TINT             s_iShotFrames;
static TGenericListener s_oEndSceneListener;

static PCFunc s_oMoneyAddFunc;
static PCFunc s_oMoneySetFunc;
static PCFunc s_oCheatFunc;
static PCFunc s_oNetworkEditorFunc;
static PCFunc s_oMenuCloseFunc;

static LARGE_INTEGER s_oFrequency;
static LARGE_INTEGER s_oFrameStart;
static LARGE_INTEGER s_oLastFrameEnd;
static TFLOAT        s_fCpuTime;

static PCVar s_oNetworkSavePathVar;
#	ifdef BARNYARD_LATEST
static PCFunc s_oXUILoadFunc;
#	endif

#  endif // PRIMEDEBUG_DEV

// The settings read while the game creates its systems, registered before them (CreateConsole)
static PCVar s_oFPSLimitVar;
static PCVar s_oTextureOverridesVar;
static PCVar s_oPlayerFloatsVar;
#  ifdef TRENDERINTERFACE_DX8
static PCVar s_oLowEndSkinVar;
#  endif

static PCVar* s_apSettingVars[] = {
	&s_oFPSLimitVar,
	&s_oTextureOverridesVar,
	&s_oPlayerFloatsVar,
#  ifdef TRENDERINTERFACE_DX8
	&s_oLowEndSkinVar,
#  endif
};

#  ifdef TRENDERINTERFACE_DX8
// The skin shader is created again for the new path
static void OnLowEndSkinChanged( PCVar* a_pVar )
{
	if ( !ASkinShader::IsSingletonCreated() || !ASkinShader::GetSingleton()->IsValidated() )
		return;

	ASkinShader* pShader  = ASkinShader::GetSingleton();
	const TBOOL  bHighEnd = !ASkinShader::ms_iForceLowEndSkinning && pShader->IsCapableHighEndSkinning();

	if ( pShader->IsHighEndSkinning() != bHighEnd )
	{
		pShader->Invalidate();
		pShader->Validate();
	}
}
#  endif

void APrimeDebug::RegisterSettings()
{
	s_oFPSLimitVar.CreateFloatRefRange( "game.fps_limit", "The frame rate cap (0: none, like retail)", PCVar::FLAGS_VALIDATERANGE, &AApplication::ms_fFPSLimit, TNULL, TNULL, 0.0f, 1000.0f, 10.0f );
	s_oTextureOverridesVar.CreateIntRefRange( "render.texture_overrides", "Textures in Data\\Textures replace the ones of the material libraries loaded next", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, &AMaterialLibrary::ms_iTextureOverrides, TNULL, TNULL, 0, 1, 1 );
	s_oPlayerFloatsVar.CreateIntRefRange( "minigame.player_floats", "The minigames started next show the P1-P4/Com markers above the players (dead code in retail)", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, &AMiniGameState::ms_iShowPlayerFloats, TNULL, TNULL, 0, 1, 1 );
#  ifdef TRENDERINTERFACE_DX8
	s_oLowEndSkinVar.CreateIntRefRange( "render.low_end_skin", "The skin shader uses its low end path (CPU skinning, fixed function), as without vertex shader 1.1", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, &ASkinShader::ms_iForceLowEndSkinning, OnLowEndSkinChanged, TNULL, 0, 1, 1 );
#  endif
}

void APrimeDebug::UnregisterSettings()
{
	for ( TINT i = 0; i < TARRAYSIZE( s_apSettingVars ); i++ )
		s_apSettingVars[ i ]->Remove();
}

// A line setting one of the settings: "set <name> <value>" or "<name> <value>"
static TBOOL IsSettingLine( const TCHAR* a_szLine )
{
	TCHAR szLine[ PConsole::MAX_LINE ];
	PString::Copy( szLine, sizeof( szLine ), a_szLine );

	if ( TCHAR* pComment = strchr( szLine, '#' ) )
		*pComment = '\0';

	TCHAR* apArgs[ PConsole::MAX_ARGS ];
	TINT   iNumArgs = 0;

	if ( !PConsole::ParseArguments( szLine, apArgs, PConsole::MAX_ARGS, &iNumArgs ) )
		return TFALSE;

	const TCHAR* szName = TNULL;

	if ( iNumArgs >= 3 && strcmp( apArgs[ 0 ], "set" ) == 0 )
		szName = apArgs[ 1 ];
	else if ( iNumArgs >= 2 )
		szName = apArgs[ 0 ];

	PCVar* pVar = szName ? PConsole::FindVariable( szName ) : TNULL;

	for ( TINT i = 0; pVar && i < TARRAYSIZE( s_apSettingVars ); i++ )
	{
		if ( pVar == s_apSettingVars[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

#  ifdef PRIMEDEBUG_DEV
// Save\OpenBarnyard_dev.cfg, next to savedata.by (SaveLoadSKU)
static const TCHAR* GetDevConfigPath()
{
	static TCHAR s_szPath[ MAX_PATH ];

	if ( s_szPath[ 0 ] == '\0' )
		TStringManager::String8Format( s_szPath, sizeof( s_szPath ), "%s\\OpenBarnyard_dev.cfg", SaveLoadSKU::GetSaveDirectory() );

	return s_szPath;
}
#  endif

// Splits the command line into its arguments (quotes kept around the ones that have them) and runs the console
// lines starting with '+'
void APrimeDebug::RunCommandLine( TBOOL ( *a_fnFilter )( const TCHAR* a_szLine ) )
{
	const TCHAR* pChar = GetCommandLineA();

	TCHAR szLine[ PConsole::MAX_LINE ];
	TINT  iLength = -1; // -1: no line

	auto RunLine = [ & ]() {
		if ( iLength > 0 )
			szLine[ iLength ] = '\0';

		if ( iLength > 0 && ( !a_fnFilter || a_fnFilter( szLine ) ) )
		{
			TINFO( "[primedebug] command line%s: %s\n", a_fnFilter ? " (early)" : "", szLine );

			if ( !PConsole::Execute( szLine ) )
				TWARN( "[primedebug] '%s' failed\n", szLine );
		}

		iLength = -1;
	};

	while ( *pChar != '\0' )
	{
		while ( *pChar == ' ' || *pChar == '\t' )
			pChar++;

		if ( *pChar == '\0' )
			break;

		// The argument, with its quotes
		const TCHAR* pStart  = pChar;
		TBOOL        bQuoted = TFALSE;

		while ( *pChar != '\0' && ( bQuoted || ( *pChar != ' ' && *pChar != '\t' ) ) )
		{
			if ( *pChar == '"' )
				bQuoted = !bQuoted;

			pChar++;
		}

		TINT iArgLength = TINT( pChar - pStart );

		if ( pStart[ 0 ] == '+' )
		{
			RunLine();
			pStart++;
			iArgLength--;
			iLength = 0;
		}
		else if ( pStart[ 0 ] == '-' && isalpha( TUINT8( pStart[ 1 ] ) ) )
		{
			// A switch of the game ends the line
			RunLine();
			continue;
		}
		else if ( iLength < 0 )
		{
			continue;
		}

		if ( iLength > 0 && iLength < PConsole::MAX_LINE - 1 )
			szLine[ iLength++ ] = ' ';

		if ( iLength + iArgLength < PConsole::MAX_LINE - 1 )
		{
			memcpy( szLine + iLength, pStart, iArgLength );
			iLength += iArgLength;
		}
	}

	RunLine();
}

#  ifdef PRIMEDEBUG_DEV

static void OnOverlayChanged( PCVar* a_pVar )
{
#	ifdef ADEBUGOVERLAY
	if ( ADebugOverlay::IsSingletonCreated() )
		ADebugOverlay::GetSingleton()->SetEnabled( a_pVar->GetBool() );
#	endif
}

#	ifdef AWORLDDEBUGDRAW
static void OnLayerChanged( PCVar* a_pVar )
{
	if ( AWorldDebugDraw::IsSingletonCreated() )
		AWorldDebugDraw::GetSingleton()->SetLayerEnabled( AWorldDebugDraw::LAYER( TINT( TUINTPTR( a_pVar->GetUserData() ) ) ), a_pVar->GetBool() );
}
#	endif

// A world variable set by the command line / the dev config is set again once the player is in the world
static TBOOL DeferWorldVar( PCVar* a_pVar )
{
	if ( !APrimeDebugFSM::IsRunningCommandLine() )
		return TFALSE;

	TCHAR szValue[ 16 ];
	_snprintf( szValue, sizeof( szValue ), "%d", a_pVar->GetInt() );
	szValue[ sizeof( szValue ) - 1 ] = '\0';

	TCHAR* apArgs[] = { const_cast<TCHAR*>( "set" ), const_cast<TCHAR*>( a_pVar->GetName() ), szValue };
	APrimeDebugFSM::QueueCommand( TARRAYSIZE( apArgs ), apArgs, TTRUE );
	return TTRUE;
}

static void OnDayPhaseChanged( PCVar* a_pVar )
{
	if ( DeferWorldVar( a_pVar ) )
		return;

	if ( AGameTimeManager::IsSingletonCreated() )
	{
		TINFO( "[primedebug] day phase %d\n", a_pVar->GetInt() );
		AGameTimeManager::GetSingleton()->ChangePhaseOfDay( AGameTimeManager::DAYPHASE( a_pVar->GetInt() ), TTRUE, TTRUE );
	}
}

static void OnWeatherChanged( PCVar* a_pVar )
{
	if ( DeferWorldVar( a_pVar ) )
		return;

	if ( AWeatherManager::IsSingletonCreated() )
	{
		TINFO( "[primedebug] weather %d\n", a_pVar->GetInt() );
		AWeatherManager::GetSingleton()->SetCurrentWeather( AWeatherManager::WEATHERTYPE( a_pVar->GetInt() ) );

		// The weather only shows through the time of day FX (sky, light, fog, rain, the ambient effects), which take
		// it when the day phase changes: they're set again for the current phase, from where it is
		if ( AGameTimeFXManager::IsSingletonCreated() && AGameTimeManager::IsSingletonCreated() )
		{
			AGameTimeManager*          pGameTime = AGameTimeManager::GetSingleton();
			AGameTimeManager::DAYPHASE eDayPhase = pGameTime->GetCurrentDayPhase();
			TFLOAT                     fElapsed  = pGameTime->GetDayTime() - AGameTimeManager::GetDayPhaseStartTime( eDayPhase );

			AGameTimeFXManager::GetSingleton()->SetDayPhase( eDayPhase, TFALSE, TMath::Max( fElapsed, 0.0f ) );
		}
	}
}

static void OnTimeScaleChanged( PCVar* a_pVar )
{
	ASimState::Dev_SetTimeScale( a_pVar->GetFloat() );
}

static TBOOL MoneyAddCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( !APlayerProfileManager::IsSingletonCreated() )
		return TFALSE;

	TINT iValue = atoi( a_ppArgv[ 1 ] );
	TINFO( "[primedebug] +%d pick ups\n", iValue );
	APlayerProfileManager::GetSingleton()->GetMainProfile()->AddPickups( iValue );
	return TTRUE;
}

static TBOOL MoneySetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( !APlayerProfileManager::IsSingletonCreated() )
		return TFALSE;

	TINT iValue = atoi( a_ppArgv[ 1 ] );
	TINFO( "[primedebug] %d pick ups\n", iValue );
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetPickups( iValue, TFALSE, TTRUE );
	return TTRUE;
}

// The cheat is entered in the current state, under the menu if it's open (the menu closes first). From the command
// line it waits for the world
static TBOOL CheatCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( APrimeDebugFSM::IsRunningCommandLine() )
		return APrimeDebugFSM::QueueCommand( a_iArgc, a_ppArgv, TTRUE );

	// A word: its letters are typed as cheat keys
	if ( !isdigit( TUINT8( a_ppArgv[ 1 ][ 0 ] ) ) )
	{
		if ( ADebugMenuState::GetOpenMenu() )
			return APrimeDebugFSM::QueueCommand( a_iArgc, a_ppArgv, TFALSE );

		ACheatActivator::Dev_EnterCheatCode( a_ppArgv[ 1 ] );
		return TTRUE;
	}

	TINT iCheat = atoi( a_ppArgv[ 1 ] );

	if ( iCheat < 0 || iCheat >= ACheatActivator::CHEAT_NUMOF )
		return TFALSE;

	if ( !ADebugMenuState::RequestAction( ADebugMenuState::ACTION_CHEAT, iCheat ) )
		ACheatActivator::Dev_TriggerCheat( ACheatActivator::CHEAT( iCheat ) );

	return TTRUE;
}

static AUnit* GetPlayerUnit()
{
	return APlayerManager::IsSingletonCreated() ? APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) : TNULL;
}

// player.teleport x y z [facing] / player.teleport @<terrain locator>
static TBOOL TeleportCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( APrimeDebugFSM::IsRunningCommandLine() )
		return APrimeDebugFSM::QueueCommand( a_iArgc, a_ppArgv, TTRUE );

	AUnit* pPlayer = GetPlayerUnit();

	if ( !pPlayer || !pPlayer->GetSteer() )
	{
		TWARN( "[primedebug] player.teleport: no player\n" );
		return TFALSE;
	}

	TVector4 vPos = *pPlayer->GetSteer()->GetPosition();
	TVector4 vDir = *pPlayer->GetSteer()->GetDirection();

	if ( a_iArgc == 2 && a_ppArgv[ 1 ][ 0 ] == '@' )
	{
		const TCHAR* szLocator = a_ppArgv[ 1 ] + 1;

		if ( !ATerrainInterface::IsSingletonCreated() || !ATerrainInterface::GetSingleton()->GetLocatorList() ||
		     !ATerrainInterface::GetSingleton()->GetLocatorPosAndDir( szLocator, vPos, vDir ) )
		{
			TWARN( "[primedebug] player.teleport: no locator %s\n", szLocator );
			return TFALSE;
		}
	}
	else if ( a_iArgc == 4 || a_iArgc == 5 )
	{
		vPos.x = TFLOAT( atof( a_ppArgv[ 1 ] ) );
		vPos.y = TFLOAT( atof( a_ppArgv[ 2 ] ) );
		vPos.z = TFLOAT( atof( a_ppArgv[ 3 ] ) );

		if ( a_iArgc == 5 )
		{
			TFLOAT fAngle = TFLOAT( atof( a_ppArgv[ 4 ] ) ) * TMath::PI / 180.0f;
			vDir.x        = TMath::Sin( fAngle );
			vDir.z        = TMath::Cos( fAngle );
		}
	}
	else
	{
		TWARN( "[primedebug] usage: player.teleport <x> <y> <z> [facing] | player.teleport @<locator>\n" );
		return TFALSE;
	}

	vPos.w = 1.0f;
	vDir.y = 0.0f;
	vDir.w = 0.0f;

	if ( vDir.x * vDir.x + vDir.z * vDir.z < 0.0001f )
		vDir = TVector4( 0.0f, 0.0f, 1.0f, 0.0f );

	vDir.Normalise();

	s_vTeleportPos  = vPos;
	s_vTeleportDir  = vDir;
	s_fTeleportHold = TELEPORT_HOLD_TIME;

	pPlayer->GetSteer()->SetPosition( vPos );
	pPlayer->GetSteer()->SetDirection( vDir );

	TINFO( "[primedebug] player.teleport %.2f %.2f %.2f\n", vPos.x, vPos.y, vPos.z );
	return TTRUE;
}

// Logs the player.teleport line of the player's position and copies it to the clipboard
static TBOOL PosCopyCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( !GetPlayerUnit() )
		return TFALSE;

	TCHAR szLine[ 128 ];
	_snprintf( szLine, sizeof( szLine ), "player.teleport %.2f %.2f %.2f %.0f", s_afPlayerPos[ 0 ], s_afPlayerPos[ 1 ], s_afPlayerPos[ 2 ], s_afPlayerPos[ 3 ] );
	szLine[ sizeof( szLine ) - 1 ] = '\0';
	TINFO( "[primedebug] %s\n", szLine );

	if ( OpenClipboard( NULL ) )
	{
		EmptyClipboard();

		if ( HGLOBAL hMemory = GlobalAlloc( GMEM_MOVEABLE, strlen( szLine ) + 1 ) )
		{
			strcpy( TSTATICCAST( TCHAR, GlobalLock( hMemory ) ), szLine );
			GlobalUnlock( hMemory );

			if ( !SetClipboardData( CF_TEXT, hMemory ) )
				GlobalFree( hMemory );
		}

		CloseClipboard();
	}

	return TTRUE;
}

// Holds the teleported player in place for a moment
static void UpdateTeleport( TFLOAT a_fDeltaTime )
{
	if ( s_fTeleportHold <= 0.0f )
		return;

	AUnit* pPlayer = GetPlayerUnit();

	if ( !pPlayer || !pPlayer->GetSteer() )
	{
		s_fTeleportHold = 0.0f;
		return;
	}

	pPlayer->GetSteer()->SetPosition( s_vTeleportPos );
	pPlayer->GetSteer()->SetDirection( s_vTeleportDir );

	if ( !ADebugMenuState::GetOpenMenu() )
		s_fTeleportHold -= a_fDeltaTime;
}

static TBOOL NetworkEditorCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	// The editor takes the place of the menu
	if ( !ADebugMenuState::GetOpenMenu() && !ADebugMenuState::Open() )
		return TFALSE;

	return ADebugMenuState::RequestAction( ADebugMenuState::ACTION_NETWORK_EDITOR, 0 );
}

#	ifdef BARNYARD_LATEST
static TBOOL XUILoadCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	PString::Copy( AGUI2::ms_szDevXUIScene, sizeof( AGUI2::ms_szDevXUIScene ), a_ppArgv[ 1 ] );
	return TTRUE;
}
#	endif

const TCHAR* APrimeDebug::GetNetworkSavePath()
{
	return s_oNetworkSavePathVar.GetType() == PCVar::TYPE_STRING ? s_oNetworkSavePathVar.GetString() : TNULL;
}

static TBOOL MenuCloseCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( !ADebugMenuState::RequestAction( ADebugMenuState::ACTION_CLOSE, 0 ) )
		PDebugMenu::SetVisible( TFALSE );

	return TTRUE;
}

static TFLOAT GetTimeSinceStart()
{
	FILETIME oCreation, oExit, oKernel, oUser, oNow;
	GetProcessTimes( GetCurrentProcess(), &oCreation, &oExit, &oKernel, &oUser );
	GetSystemTimeAsFileTime( &oNow );

	ULARGE_INTEGER uiCreation, uiNow;
	uiCreation.LowPart  = oCreation.dwLowDateTime;
	uiCreation.HighPart = oCreation.dwHighDateTime;
	uiNow.LowPart       = oNow.dwLowDateTime;
	uiNow.HighPart      = oNow.dwHighDateTime;

	return TFLOAT( double( uiNow.QuadPart - uiCreation.QuadPart ) / 10000000.0 );
}

// String variables have no change callback: dev.screenshot_times is parsed again when it changes
static void UpdateShotTimes()
{
	static TCHAR s_szParsed[ 256 ];
	const TCHAR* szTimes = s_oShotTimesVar.GetString();

	if ( !szTimes || strcmp( szTimes, s_szParsed ) == 0 )
		return;

	PString::Copy( s_szParsed, sizeof( s_szParsed ), szTimes );

	s_iNumShotTimes      = 0;
	const TCHAR* pCursor = szTimes;

	while ( pCursor && *pCursor != '\0' && s_iNumShotTimes < MAX_SHOT_TIMES )
	{
		TCHAR* pEnd;
		TFLOAT fTime = TFLOAT( strtod( pCursor, &pEnd ) );

		if ( pEnd == pCursor )
			break;

		s_afShotTimes[ s_iNumShotTimes ] = fTime;
		s_abShotTaken[ s_iNumShotTimes ] = TFALSE;
		s_iNumShotTimes++;

		pCursor = ( *pEnd == ',' ) ? pEnd + 1 : pEnd;
	}

	TINFO( "[screenshot] %d shot times\n", s_iNumShotTimes );
}

static void GetShotFileName( TCHAR* a_szOut, TINT a_iSize, const TCHAR* a_szSuffix )
{
	const TCHAR* szDir = s_oShotDirVar.GetString();
	const TCHAR* szTag = s_oShotTagVar.GetString();

	if ( !szDir || szDir[ 0 ] == '\0' ) szDir = "Screenshots";
	if ( !szTag || szTag[ 0 ] == '\0' ) szTag = "screenshot";

	CreateDirectoryA( szDir, NULL );
	PString::Format( a_szOut, a_iSize, "%s/%s_%s.png", szDir, szTag, a_szSuffix );
}

static TBOOL ScreenshotCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc > 2 )
		return TFALSE;

	if ( a_iArgc == 2 )
	{
		PString::Copy( s_szQueuedShot, sizeof( s_szQueuedShot ), a_ppArgv[ 1 ] );
		return TTRUE;
	}

	TCHAR szSuffix[ 32 ];
	PString::Format( szSuffix, sizeof( szSuffix ), "%d", TINT( GetTimeSinceStart() ) );
	GetShotFileName( s_szQueuedShot, sizeof( s_szQueuedShot ), szSuffix );

	for ( TINT i = 2; GetFileAttributesA( s_szQueuedShot ) != INVALID_FILE_ATTRIBUTES && i < 100; i++ )
	{
		PString::Format( szSuffix, sizeof( szSuffix ), "%d_%d", TINT( GetTimeSinceStart() ), i );
		GetShotFileName( s_szQueuedShot, sizeof( s_szQueuedShot ), szSuffix );
	}

	return TTRUE;
}

static void DestroyShotCapture()
{
	if ( s_pShotCapture )
	{
		TRenderInterface::GetSingleton()->DestroyCapture( s_pShotCapture );
		s_pShotCapture = TNULL;
	}
}

static TBOOL SaveShot( TRenderCapture* a_pCapture, TINT a_iWidth, TINT a_iHeight, const TCHAR* a_szFileName )
{
	TUINT8* pPixels = TSTATICCAST( TUINT8, a_pCapture->ObtainBuffer() );

	if ( !pPixels )
		return TFALSE;

	for ( TINT i = 0; i < a_iWidth * a_iHeight; i++ )
	{
		pPixels[ i * 3 + 0 ] = pPixels[ i * 4 + 0 ];
		pPixels[ i * 3 + 1 ] = pPixels[ i * 4 + 1 ];
		pPixels[ i * 3 + 2 ] = pPixels[ i * 4 + 2 ];
	}

	return stbi_write_png( a_szFileName, a_iWidth, a_iHeight, 3, pPixels, a_iWidth * 3 ) != 0;
}

// Every frame (loading screens too), before the render interface captures the requested frame and presents it
static TBOOL __stdcall OnEndScene( void* a_pCaller, void* a_pOwner, void* a_pData )
{
	TRenderInterface* pRender = TRenderInterface::GetSingleton();

	if ( s_pShotCapture )
	{
		if ( s_pShotCapture->Poll() )
		{
			const TINT iWidth  = TINT( pRender->GetCurrentDisplayParams()->uiWidth );
			const TINT iHeight = TINT( pRender->GetCurrentDisplayParams()->uiHeight );

			if ( SaveShot( s_pShotCapture, iWidth, iHeight, s_szShotFile ) )
				TINFO( "[screenshot] %.1fs: saved %s\n", GetTimeSinceStart(), s_szShotFile );
			else
				TWARN( "[screenshot] %.1fs: couldn't save %s\n", GetTimeSinceStart(), s_szShotFile );

			fflush( stdout );
			DestroyShotCapture();
		}
		else if ( ++s_iShotFrames > 30 )
		{
			TWARN( "[screenshot] %s: the capture didn't come\n", s_szShotFile );
			DestroyShotCapture();
		}

		return TTRUE;
	}

	if ( s_szQueuedShot[ 0 ] != '\0' )
	{
		PString::Copy( s_szShotFile, sizeof( s_szShotFile ), s_szQueuedShot );
		s_szQueuedShot[ 0 ] = '\0';
	}
	else
	{
		UpdateShotTimes();

		if ( s_iNumShotTimes == 0 )
			return TTRUE;

		const TFLOAT fTime = GetTimeSinceStart();
		TINT         iShot = -1;

		for ( TINT i = 0; i < s_iNumShotTimes; i++ )
		{
			if ( !s_abShotTaken[ i ] && s_afShotTimes[ i ] <= fTime && ( iShot == -1 || s_afShotTimes[ i ] < s_afShotTimes[ iShot ] ) )
				iShot = i;
		}

		if ( iShot == -1 )
			return TTRUE;

		s_abShotTaken[ iShot ] = TTRUE;

		TCHAR        szSuffix[ 32 ];
		const TFLOAT fShotTime = s_afShotTimes[ iShot ];

		if ( fShotTime == TFLOAT( TINT( fShotTime ) ) )
			PString::Format( szSuffix, sizeof( szSuffix ), "%d", TINT( fShotTime ) );
		else
			PString::Format( szSuffix, sizeof( szSuffix ), "%g", fShotTime );

		GetShotFileName( s_szShotFile, sizeof( s_szShotFile ), szSuffix );
	}

	s_pShotCapture = pRender->CreateCapture();
	s_iShotFrames  = 0;
	s_pShotCapture->Create( TRenderCapture::FORMAT_RGBA32, TINT( pRender->GetCurrentDisplayParams()->uiWidth ), TINT( pRender->GetCurrentDisplayParams()->uiHeight ) );

	if ( !s_pShotCapture->Request() )
	{
		TWARN( "[screenshot] %s: the capture failed\n", s_szShotFile );
		DestroyShotCapture();
	}

	return TTRUE;
}

// The cheat labels of the menu
static void OnCheatLabel( TINT a_iId, void* a_pUserData )
{
	TINT iCheat = TINT( TUINTPTR( a_pUserData ) );
	TINFO( "[primedebug] cheat %s\n", s_aCheatNames[ iCheat ] );
	ADebugMenuState::RequestAction( ADebugMenuState::ACTION_CHEAT, iCheat );
}

void APrimeDebug::RegisterGame()
{
	// Variables
	s_oOverlayVar.CreateIntRefRange( "debug.overlay", "The debug overlay: the lines of the systems, the unit labels, the frame time", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, &s_iOverlay, OnOverlayChanged, TNULL, 0, 1, 1 );
	s_oDayPhaseVar.CreateIntRefRange( "game.day_phase", "The phase of the day (0=Dawn,1=Morning,2=Afternoon,3=Dusk,4=Night)", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_ENUM, &s_iDayPhase, OnDayPhaseChanged, TNULL, 0, AGameTimeManager::DAYPHASE_NUMOF - 1, 1 );
	s_oWeatherVar.CreateIntRefRange( "game.weather", "The weather (0=Normal,1=Heatwave,2=Light Rain,3=Heavy Rain,4=Rainbow,5=Lightning Storm)", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_ENUM, &s_iWeather, OnWeatherChanged, TNULL, 0, AWeatherManager::WEATHERTYPE_NUMOF - 1, 1 );
	s_oTimeScaleVar.CreateFloatRefRange( "game.time_scale", "The speed of the game time in the sim", PCVar::FLAGS_VALIDATERANGE, &s_fTimeScale, OnTimeScaleChanged, TNULL, 1.0f, 60.0f, 1.0f );

	s_oFrontEndAnticsVar.CreateIntRefRange( "frontend.antics", "What the Antics button of the main menu opens (0=Normal,1=Debug Menu,2=Players Select)", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_ENUM, &AGameLoader::ms_iDevFrontEndAntics, TNULL, TNULL, 0, AGameLoader::DEVANTICS_NUMOF - 1, 1 );
	s_oMiniGameVariantVar.CreateIntRefRange( "minigame.variant", "The variant of the minigame started with -minigame (-1: its default one)", PCVar::FLAGS_VALIDATERANGE, &AGameLoader::ms_iDevMiniGameVariant, TNULL, TNULL, -1, 9, 1 );
	s_oMiniGamePlayersVar.CreateIntRefRange( "minigame.players", "The human players of the minigame started with -minigame, sharing the controllers (0: one)", PCVar::FLAGS_VALIDATERANGE, &AGameLoader::ms_iDevMiniGamePlayers, TNULL, TNULL, 0, 4, 1 );

	PCVarValue oNoVector;
	memset( &oNoVector, 0, sizeof( oNoVector ) );
	s_oPlayerPosVar.Create( "player.pos", "The position of the player and their facing in degrees (read only, see player.pos.copy)", PCVar::FLAGS_READONLY, PCVar::TYPE_VECTOR4, &oNoVector, s_afPlayerPos, TNULL, TNULL );

#	ifdef AWORLDDEBUGDRAW
	for ( TINT i = 0; i < AWorldDebugDraw::LAYER_NUMOF; i++ )
		s_aLayerVars[ i ].CreateIntRefRange( s_aLayerVarNames[ i ][ 0 ], s_aLayerVarNames[ i ][ 1 ], PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, &s_aiLayers[ i ], OnLayerChanged, (void*)TUINTPTR( i ), 0, 1, 1 );
#	endif

	PCVarValue oNoString;
	memset( &oNoString, 0, sizeof( oNoString ) );

	// Screenshots
	s_oShotDirVar.Create( "dev.screenshot_dir", "The directory of the screenshots (Screenshots if empty)", PCVar::FLAGS_NONE, PCVar::TYPE_STRING, &oNoString, TNULL, TNULL, TNULL );
	s_oShotTagVar.Create( "dev.screenshot_tag", "The start of the file names of the screenshots (screenshot if empty)", PCVar::FLAGS_NONE, PCVar::TYPE_STRING, &oNoString, TNULL, TNULL, TNULL );
	s_oShotTimesVar.Create( "dev.screenshot_times", "Seconds after the start at which screenshots are saved as <dir>/<tag>_<seconds>.png, comma separated", PCVar::FLAGS_NONE, PCVar::TYPE_STRING, &oNoString, TNULL, TNULL, TNULL );
	s_oNetworkSavePathVar.Create( "editor.network_save_path", "The file the Save of the network editor writes its text dump to (nothing if empty)", PCVar::FLAGS_NONE, PCVar::TYPE_STRING, &oNoString, TNULL, TNULL, TNULL );

	SyncVars();

	// Commands
	s_oMoneyAddFunc.Create( "money.add", "Adds pick ups to the player's money.", "money.add <count>", MoneyAddCommand, 1, TNULL );
	s_oMoneySetFunc.Create( "money.set", "Sets the player's money.", "money.set <count>", MoneySetCommand, 1, TNULL );
	s_oCheatFunc.Create( "game.cheat", "Enters a cheat (0 Mud Jump, 1 Barn Pool, 2 Bike Race, 3 Chasing Chicks, 4 Veggie Patch, 5 Cow Tipping, 6 Barntender, 7 Money, 8 Items, 9 Bonus Items, 10 Antics) or types its code word. From the command line it waits for the world.", "game.cheat <index|WORD>", CheatCommand, 1, TNULL );
	s_oTeleportFunc.Create( "player.teleport", "Puts the player at a position (facing in degrees, 0 = +Z) or at a terrain locator. From the command line it waits for the world.", "player.teleport <x> <y> <z> {facing} | player.teleport @<locator>", TeleportCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	s_oPosCopyFunc.Create( "player.pos.copy", "Logs the player.teleport line of the player's position and copies it to the clipboard.", "player.pos.copy", PosCopyCommand, 0, TNULL );
	s_oNetworkEditorFunc.Create( "tools.network_editor", "Opens the interior network editor (Escape quits it).", "tools.network_editor", NetworkEditorCommand, 0, TNULL );
	s_oMenuCloseFunc.Create( "debug.menu.close", "Closes the debug menu.", "debug.menu.close", MenuCloseCommand, 0, TNULL );
	s_oScreenshotFunc.Create( "screenshot", "Saves the next frame as a PNG file (dev.screenshot_dir/dev.screenshot_tag_<seconds>.png by default).", "screenshot {path}", ScreenshotCommand, PCFunc::ANY_NUM_ARGS, TNULL );
#	ifdef BARNYARD_LATEST
	s_oXUILoadFunc.Create( "xui.load", "Loads Data/XUI/<language>/<scene>.trb and shows its first scene, logging its elements ([xui]); 5 s after the start at the earliest.", "xui.load <scene>", XUILoadCommand, 1, TNULL );
#	endif

	s_oEndSceneListener.Connect( TRenderInterface::GetSingleton()->GetEndSceneEmitter(), TNULL, OnEndScene, 0 );

	// The menu
	PDebugMenu::AddVar( "./debug", "debug.stats" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Display" );
	PDebugMenu::AddVar( "./Display", "debug.overlay" );
	PDebugMenu::AddVar( "./Display", "world.draw.*" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Tools" );
	PDebugMenu::AddCommandLine( "./Tools", "Network Editor", "tools.network_editor" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Render" );
	PDebugMenu::AddVar( "./Render", "game.fps_limit" );
	PDebugMenu::AddVar( "./Render", "render.*" );

	APrimeDebugQuest::Register();

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Cheats" );

	for ( TINT i = 0; i < ACheatActivator::CHEAT_NUMOF; i++ )
		PDebugMenu::AddLabel( "./Cheats", s_aCheatNames[ i ], OnCheatLabel, (void*)TUINTPTR( i ) );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Money" );
	PDebugMenu::AddCommandLine( "./Money", "Add 1000", "money.add 1000" );
	PDebugMenu::AddCommandLine( "./Money", "Set to 9999", "money.set 9999" );
	PDebugMenu::AddCommandLine( "./Money", "Set to 0", "money.set 0" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Time" );
	PDebugMenu::AddVar( "./Time", "game.day_phase" );
	PDebugMenu::AddVar( "./Time", "game.time_scale" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Weather" );
	PDebugMenu::AddVar( "./Weather", "game.weather" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Player" );
	PDebugMenu::AddVar( "./Player", "player.pos" );
	PDebugMenu::AddCommandLine( "./Player", "Copy position", "player.pos.copy" );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Front End" );
	PDebugMenu::AddVar( "./Front End", "frontend.antics" );
	PDebugMenu::AddVar( "./Front End", "minigame.variant" );
	PDebugMenu::AddVar( "./Front End", "minigame.player_floats" );
	PDebugMenu::AddVar( "./Front End", "minigame.players" );

	APrimeDebugFSM::Register();
	PDebugMenu::AddCommandLine( PDebugMenu::ROOT, "Close", "debug.menu.close" );
}

void APrimeDebug::UnregisterGame()
{
	APrimeDebugFSM::Unregister();
	APrimeDebugQuest::Unregister();
	s_oEndSceneListener.Disconnect();
	DestroyShotCapture();

#	ifdef BARNYARD_LATEST
	s_oXUILoadFunc.Remove();
#	endif
	s_oScreenshotFunc.Remove();
	s_oPosCopyFunc.Remove();
	s_oTeleportFunc.Remove();
	s_oMenuCloseFunc.Remove();
	s_oNetworkEditorFunc.Remove();
	s_oCheatFunc.Remove();
	s_oMoneySetFunc.Remove();
	s_oMoneyAddFunc.Remove();

#	ifdef AWORLDDEBUGDRAW
	for ( TINT i = 0; i < AWorldDebugDraw::LAYER_NUMOF; i++ )
		s_aLayerVars[ i ].Remove();
#	endif

	s_oNetworkSavePathVar.Remove();
	s_oShotTimesVar.Remove();
	s_oShotTagVar.Remove();
	s_oShotDirVar.Remove();

	s_oPlayerPosVar.Remove();
	s_oMiniGamePlayersVar.Remove();
	s_oMiniGameVariantVar.Remove();
	s_oFrontEndAnticsVar.Remove();
	s_oTimeScaleVar.Remove();
	s_oWeatherVar.Remove();
	s_oDayPhaseVar.Remove();
	s_oOverlayVar.Remove();
}

// The variables mirroring the game get its state
void APrimeDebug::SyncVars()
{
#	ifdef ADEBUGOVERLAY
	if ( ADebugOverlay::IsSingletonCreated() )
		s_iOverlay = ADebugOverlay::GetSingleton()->IsEnabled();
#	endif

#	ifdef AWORLDDEBUGDRAW
	if ( AWorldDebugDraw::IsSingletonCreated() )
	{
		for ( TINT i = 0; i < AWorldDebugDraw::LAYER_NUMOF; i++ )
			s_aiLayers[ i ] = AWorldDebugDraw::GetSingleton()->IsLayerEnabled( AWorldDebugDraw::LAYER( i ) );
	}
#	endif

	if ( AGameTimeManager::IsSingletonCreated() )
		s_iDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();

	if ( AWeatherManager::IsSingletonCreated() )
		s_iWeather = AWeatherManager::GetSingleton()->GetCurrentWeather();

	s_fTimeScale = ASimState::Dev_GetTimeScale();

	AUnit* pPlayer = GetPlayerUnit();

	if ( pPlayer && pPlayer->GetSteer() )
	{
		const TVector4& rcPos = *pPlayer->GetSteer()->GetPosition();
		const TVector4& rcDir = *pPlayer->GetSteer()->GetDirection();
		s_afPlayerPos[ 0 ]    = rcPos.x;
		s_afPlayerPos[ 1 ]    = rcPos.y;
		s_afPlayerPos[ 2 ]    = rcPos.z;
		s_afPlayerPos[ 3 ]    = TFLOAT( atan2( rcDir.x, rcDir.z ) ) * 180.0f / TMath::PI;
	}
}

void APrimeDebug::ReadKeyboard()
{
	TInputDeviceKeyboard* pKeyboard = AInputHandler::IsSingletonCreated() ? AInputHandler::GetSingleton()->GetKeyboardDevice() : TNULL;

	for ( TINT i = 0; i < PKeyboard::KEY_NUMOF; i++ )
		PKeyboard::SetKeyDown( PKeyboard::KEY( i ), pKeyboard && pKeyboard->IsDown( s_aKeyDoodads[ i ] ) );
}

void APrimeDebug::Update( TFLOAT a_fDeltaTime )
{
	QueryPerformanceCounter( &s_oFrameStart );

	ReadKeyboard();
	SyncVars();

	PDebugSystem::Update( a_fDeltaTime );
	APrimeDebugFSM::Update( a_fDeltaTime );
	APrimeDebugQuest::Update( a_fDeltaTime );
	UpdateTeleport( a_fDeltaTime );

	// Home shows the menu: the game state behind it opens too (or the menu hides again where it can't)
	if ( PDebugMenu::IsVisible() && !ADebugMenuState::GetOpenMenu() && !ADebugMenuState::Open() )
		PDebugMenu::SetVisible( TFALSE );
}

void APrimeDebug::Render3D()
{
	PDebugSystem::Render3D();
}

void APrimeDebug::Render2D()
{
	// The frame stats: the CPU time of the last frame (from its update to here) and the time between two frames
	LARGE_INTEGER oNow;
	QueryPerformanceCounter( &oNow );

	if ( s_oLastFrameEnd.QuadPart != 0 && s_oFrequency.QuadPart != 0 )
	{
		TFLOAT fFrameTime = TFLOAT( double( oNow.QuadPart - s_oLastFrameEnd.QuadPart ) / double( s_oFrequency.QuadPart ) );
		s_fCpuTime        = TFLOAT( double( oNow.QuadPart - s_oFrameStart.QuadPart ) / double( s_oFrequency.QuadPart ) );
		PDebugSystem::UpdateStats( s_fCpuTime, fFrameTime );
	}

	s_oLastFrameEnd = oNow;

	if ( !s_pRenderer || !s_pRenderer->Begin2D() )
	{
		// Nothing can be drawn yet: the queued text goes
		PDebugText::Flush();
		return;
	}

	ACamera* pCamera = ACameraManager::IsSingletonCreated() ? ACameraManager::GetSingleton()->GetCurrentCamera() : TNULL;
	TVector4 vEye    = pCamera ? pCamera->GetMatrix().GetTranslation() : TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	PDebugSystem::Render2D( pCamera ? &vEye : TNULL );
	s_pRenderer->End2D();
}

TBOOL APrimeDebug::GetFrameStats( TFLOAT& a_rfMilliseconds, TFLOAT& a_rfFPS )
{
	const PStats& rcStats = PStats::GetFrameStats();

	if ( rcStats.GetAverage() <= 0.0f )
		return TFALSE;

	a_rfMilliseconds = rcStats.GetAverage() * 1000.0f;
	a_rfFPS          = rcStats.GetRate();
	return TTRUE;
}

#  endif // PRIMEDEBUG_DEV

void APrimeDebug::CreateConsole()
{
	PConsole::Startup();
	RegisterSettings();

#  ifdef PRIMEDEBUG_DEV
	PConsole::ExecuteFile( GetDevConfigPath(), IsSettingLine );
#  endif

	RunCommandLine( IsSettingLine );
}

void APrimeDebug::Create()
{
#  ifdef PRIMEDEBUG_DEV
	QueryPerformanceFrequency( &s_oFrequency );
	s_pRenderer = new APrimeDebugRenderer();
	PDebugSystem::Startup( s_pRenderer );
	RegisterGame();

	const TCHAR* szPath = GetDevConfigPath();

	if ( PConsole::ExecuteFile( szPath ) )
		TINFO( "[primedebug] ran %s\n", szPath );
#  else
	PDebugSystem::Startup( TNULL );
#  endif

	RunCommandLine( TNULL );
}

void APrimeDebug::Destroy()
{
	UnregisterSettings();

#  ifdef PRIMEDEBUG_DEV
	UnregisterGame();
#  endif

	PDebugSystem::Shutdown();

#  ifdef PRIMEDEBUG_DEV
	delete s_pRenderer;
	s_pRenderer = TNULL;
#  endif
}

#endif // PRIMEDEBUG
