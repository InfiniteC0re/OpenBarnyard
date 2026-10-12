#include "pch.h"
#include "AWhackARaccMiniGameState.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Tasks/ARootTask.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2AnythingPopup.h"
#include "GUI/AGUI2LifeHud.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Input/AInputHandler.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitManager.h"
#include "Units/ASSeqController.h"

#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>
#include <Math/TMatrix44.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Render/TModel.h>
#include <Render/TRenderInterface.h>
#include <Render/TSkeleton.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( AWhackARaccMiniGameState );
TPSTRING8_DECLARE( down );
TPSTRING8_DECLARE( hit );
TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( idle_1 );
TPSTRING8_DECLARE( impact );
TPSTRING8_DECLARE( impactbp );
TPSTRING8_DECLARE( reset );
TPSTRING8_DECLARE( up );

// NOTE: GetClass 004d8a30, CreateObject 004dbc70, CreateObjectInPlace 004dbcd0 (the constructor is inlined in both)
TDEFINE_CLASS( AWhackARaccMiniGameState );

// Sounds
static constexpr ASoundId SOUND_MISS         = 305;
static constexpr ASoundId SOUND_HIT          = 306;
static constexpr ASoundId SOUND_LOOP         = 313;
static constexpr ASoundId SOUND_INTRO        = 314;
static constexpr ASoundId SOUND_DIAL         = 315;
static constexpr ASoundId SOUND_HIT_COYOTE   = 316;
static constexpr ASoundId SOUND_HIT_PECK     = 317;
static constexpr ASoundId SOUND_HIT_EGG      = 318;
static constexpr ASoundId SOUND_FREDDY_UP    = 319;
static constexpr ASoundId SOUND_FREDDY_HIT_3 = 321;
static constexpr ASoundId SOUND_FREDDY_HIT_1 = 322;
static constexpr ASoundId SOUND_FREDDY_HIT_2 = 323;
static constexpr ASoundId SOUND_CROWD        = 324;

static constexpr TFLOAT STICK_DEADZONE = 0.2f;  // 00773b64
static constexpr TFLOAT BUCK_SPIN      = 10.5f; // 00773b68
static constexpr TFLOAT MAX_PICK_DIST  = 2500.0f;

static const TVector4 s_vEffectDir( 0.0f, -1.0f, 0.0f, 1.0f ); // 0077d024
static const TVector4 s_vMoleDir( -1.0f, 0.0f, 0.0f, 1.0f );   // 0077d014
static const TVector4 s_vArrowDir( 0.0f, 0.0f, 1.0f, 1.0f );   // 0077d004
static const TVector4 s_vDialDir( 0.0f, 0.0f, -1.0f, 1.0f );   // 0077d034

// The level the game starts at (never written)
static TINT s_iStartLevel = 0; // 00783268

// Whether the stick/keys hold the ring off the middle row/column
static TBOOL s_bRowHeld = TFALSE; // 0078329c
static TBOOL s_bColHeld = TFALSE; // 0078329d

// Squared distance of a_rcPoint to the line through a_rcLineStart and a_rcLineEnd
// $Barnyard: FUNCTION 006b8ba0
static TFLOAT DistancePointToLineSq( const TVector4& a_rcPoint, const TVector4& a_rcLineStart, const TVector4& a_rcLineEnd )
{
	TFLOAT fT;
	return TMath::DistancePointToLineSq( a_rcPoint, a_rcLineStart, a_rcLineEnd, fT );
}

// NOTE: same as the static helper in ABikeSteer.cpp (004083c0)
static TUINT32 PackColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return ( ( ( TUINT32( a_fGreen * 255.0f ) | ( ( TUINT32( a_fRed * 255.0f ) | ( TUINT32( a_fAlpha * 255.0f ) << 8 ) ) << 8 ) ) << 8 ) | TUINT32( a_fBlue * 255.0f ) );
}

// Builds the transform of the flat objects (rings, dials, the arrow): a_rcPosition, rotated by a_fRotX around X
static void MakeFlatTransform( TMatrix44& a_rOutMatrix, const TVector4& a_rcPosition, TFLOAT a_fRotX )
{
	a_rOutMatrix = TMatrix44::IDENTITY;
	TMatrix44::TransformVector( a_rOutMatrix.GetTranslation(), a_rOutMatrix, a_rcPosition );
	a_rOutMatrix.RotateX( a_fRotX );
}

// $Barnyard: FUNCTION 004da280
AWhackARaccMiniGameState::AWhackARaccMiniGameState()
{
	m_bBadHit         = TFALSE;
	m_iLoopCue        = -1;
	m_iIntroCue       = -1;
	m_iUnkCue         = -1;
	m_iFloatIndex     = 0;
	m_fMessageTimer   = -1.0f;
	m_fShakeTimer     = -1.0f;
	m_fShakeTimeLeft  = -1.0f;
	m_iCurrentPlayer  = 0;
	m_pMessage        = TNULL;
	m_pLifeHud        = TNULL;
	m_bTurnEnded      = TFALSE;
	m_bRestartPending = TFALSE;
	m_iFreddyCue      = -1;
	m_iCrowdCue       = -1;
	m_bGameEnding     = TFALSE;

	TStringManager::String16Format( m_wszLevelText, 31, L"" );

	m_pInputMap                   = AInputMapManager::GetSingleton()->FindMap( TPS8( AWhackARaccMiniGameState ) );
	m_aEffects[ EFFECT_MISS ]     = AParticleEffectManager::FindEffect( "whackmiss" );
	m_aEffects[ EFFECT_PUFF ]     = AParticleEffectManager::FindEffect( "whackpuff" );
	m_aEffects[ EFFECT_DUST ]     = AParticleEffectManager::FindEffect( "whackdust" );
	m_aEffects[ EFFECT_BAD ]      = AParticleEffectManager::FindEffect( "whackbad" );
	m_aEffects[ EFFECT_EGGSPLAT ] = AParticleEffectManager::FindEffect( "eggsplat" );
	m_aEffects[ EFFECT_EGGSHELL ] = AParticleEffectManager::FindEffect( "eggshell" );
	m_aEffects[ EFFECT_EGGYOLK ]  = AParticleEffectManager::FindEffect( "eggyolk" );
}

// $Barnyard: FUNCTION 004dbd30
// $Barnyard: FUNCTION 004dbd10
AWhackARaccMiniGameState::~AWhackARaccMiniGameState()
{
}

// $Barnyard: FUNCTION 004d8a40
void AWhackARaccMiniGameState::LoadData()
{
	m_aInts[ INT_RACCOON_SCORE ] = 1;
	m_aInts[ INT_COYOTE_SCORE ]  = 1;
	m_aInts[ INT_PECK_SCORE ]    = 1;
	m_aInts[ INT_EGG_SCORE ]     = 1;
	m_aInts[ INT_FREDDY_SCORE ]  = 1;
	m_aInts[ INT_LIVES ]         = 3;

	m_aFloats[ FLOAT_BAD_RING_TIME ]    = 1.0f;
	m_aFloats[ FLOAT_MESSAGE_TIME ]     = 1.0f;
	m_aFloats[ FLOAT_SHAKE_AMOUNT ]     = 1.0f;
	m_aFloats[ FLOAT_SHAKE_INTERVAL ]   = 0.1f;
	m_aFloats[ FLOAT_SHAKE_DURATION ]   = 0.5f;
	m_aFloats[ FLOAT_GAME_OVER_UPTIME ] = 2.0f;

	TTRB oTRB;
	oTRB.Load( "Data\\MinigameData\\WhackARac\\whackarac.trb" );

	m_iNumLevels = *oTRB.CastSymbol<TINT>( "LevelCount" );

	TCHAR szSymbol[ 1024 ];

	for ( TINT i = 1; i - 1 < m_iNumLevels; i++ )
	{
		Level& rLevel = m_aLevels[ i - 1 ];

		TStringManager::String8Format( szSymbol, sizeof( szSymbol ), "Level%i", i );
		TUINT8* pLevelData = oTRB.CastSymbol<TUINT8>( szSymbol );
		TUtil::MemCopy( &rLevel, pLevelData, 0x20 );

		TStringManager::String8Format( szSymbol, sizeof( szSymbol ), "LevelFormCount%i", i );
		TINT* pFormCount = oTRB.CastSymbol<TINT>( szSymbol );
		rLevel.iNumForms = *pFormCount;
		rLevel.pForms    = TSTATICCAST( Form, TMalloc( sizeof( Form ) * *pFormCount ) );
		TUtil::MemCopy( rLevel.pForms, pLevelData + 0x20, sizeof( Form ) * *pFormCount );
	}

	TINT*      pNumFormations = oTRB.CastSymbol<TINT>( "NumFormations" );
	Formation* pFormations    = oTRB.CastSymbol<Formation>( "Formations" );

	for ( TINT i = 0; i < *pNumFormations; i++ )
		TUtil::MemCopy( &m_aFormations[ i ], &pFormations[ i ], sizeof( Formation ) );

	TUtil::MemCopy( m_aInts, oTRB.GetSymbolAddress( "Ints" ), sizeof( m_aInts ) );
	TUtil::MemCopy( m_aFloats, oTRB.GetSymbolAddress( "Floats" ), sizeof( m_aFloats ) );

	oTRB.Close();
}

// $Barnyard: FUNCTION 004d8ca0
void AWhackARaccMiniGameState::RenderRing( TModelInstance* a_pInstance )
{
	TRenderInterface* pRender = TRenderInterface::GetSingleton();
	TMatrix44         matOldLightColour;

	// The ring turns red while Freddy's hit is shown
	if ( m_bBadHit == TTRUE )
	{
		matOldLightColour = pRender->GetLightColour();

		TMatrix44 matRed;
		TUtil::MemSet( &matRed, 0, sizeof( matRed ) );
		matRed.AsBasisVector4( BASISVECTOR_RIGHT ) = TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
		pRender->SetLightColourMatrix( matRed );
	}

	TSkeletonInstance* pSkeletonInstance = a_pInstance->GetSkeletonInstance();
	pSkeletonInstance->UpdateState( TTRUE );
	pRender->GetCurrentContext()->SetSkeletonInstance( pSkeletonInstance );

	TModelLOD& rLOD = a_pInstance->GetModel()->GetLOD( a_pInstance->GetLOD() );

	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
		rLOD.ppMeshes[ i ]->Render();

	if ( m_bBadHit == TTRUE )
		pRender->SetLightColourMatrix( matOldLightColour );
}

// $Barnyard: FUNCTION 004d8dd0
void AWhackARaccMiniGameState::RenderBadRing( TModelInstance* a_pInstance )
{
	// Only visible while Freddy's hit is shown
	if ( !m_bBadHit )
		return;

	TRenderInterface* pRender           = TRenderInterface::GetSingleton();
	TMatrix44         matOldLightColour = pRender->GetLightColour();

	TMatrix44 matRed;
	TUtil::MemSet( &matRed, 0, sizeof( matRed ) );
	matRed.AsBasisVector4( BASISVECTOR_RIGHT ) = TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	pRender->SetLightColourMatrix( matRed );

	TSkeletonInstance* pSkeletonInstance = a_pInstance->GetSkeletonInstance();
	pSkeletonInstance->UpdateState( TTRUE );
	pRender->GetCurrentContext()->SetSkeletonInstance( pSkeletonInstance );

	TModelLOD& rLOD = a_pInstance->GetModel()->GetLOD( a_pInstance->GetLOD() );

	for ( TINT i = 0; i < rLOD.iNumMeshes; i++ )
		rLOD.ppMeshes[ i ]->Render();

	pRender->SetLightColourMatrix( matOldLightColour );
}

// $Barnyard: FUNCTION 004d8ef0
void AWhackARaccMiniGameState::RenderBadRingCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	TSTATICCAST( AWhackARaccMiniGameState, a_pUserData )->RenderBadRing( a_pInstance );
}

// $Barnyard: FUNCTION 004d8f10
void AWhackARaccMiniGameState::RenderRingCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	TSTATICCAST( AWhackARaccMiniGameState, a_pUserData )->RenderRing( a_pInstance );
}

// $Barnyard: FUNCTION 004d8f30
ASoundId AWhackARaccMiniGameState::GetSound()
{
	return 35;
}

// $Barnyard: FUNCTION 004d8f40
void AWhackARaccMiniGameState::ShowMessage( const TWCHAR* a_wszText )
{
	m_pMessage->m_eMode       = AGUI2AnythingPopup::MODE_FADE;
	m_pMessage->m_fPulseSpeed = 1.0f;
	m_pMessage->m_fStartScale = 1.0f;
	m_pMessage->m_fScale      = 1.2f;
	m_pMessage->m_fDuration   = m_aFloats[ FLOAT_MESSAGE_TIME ];
	m_pMessage->SetText( a_wszText, 0xFF77FFFF, &m_GUIElement );
}

// $Barnyard: FUNCTION 004d8fb0
void AWhackARaccMiniGameState::ResetLevel( TBOOL a_bNewGame )
{
	for ( TINT i = 0; i < NUM_BUCKS; i++ )
	{
		m_aBucks[ i ].eState = MOLESTATE_HIDDEN;
		m_aBucks[ i ].vPos   = TVector4::VEC_ZERO;
	}

	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
	{
		m_aRaccoons[ i ].eState = MOLESTATE_HIDDEN;
		m_aRaccoons[ i ].vPos   = TVector4::VEC_ZERO;
	}

	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
	{
		m_aSpecials[ i ].eState = MOLESTATE_HIDDEN;
		m_aSpecials[ i ].vPos   = TVector4::VEC_ZERO;
	}

	for ( TINT i = 0; i < NUM_HOLES; i++ )
	{
		m_aHoles[ i ].bOccupied = TFALSE;
		m_aHoles[ i ].iObject   = -1;
		m_aHoles[ i ].eKind     = KIND_RACCOON;
		m_aHoles[ i ].fCooldown = 0.0f;
	}

	m_fWhackCooldown = 0.0f;
	m_fTimeLimit     = m_aLevels[ m_iLevel ].fTime;
	m_iVoiceCue      = -1;
	m_iFreddyCue     = -1;

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		Player& rPlayer      = m_aPlayers[ i ];
		rPlayer.iRow         = 1;
		rPlayer.iCol         = 1;
		rPlayer.bPerfect     = TTRUE;
		rPlayer.iHole        = 4;
		rPlayer.fRingPulse   = 0.0f;
		rPlayer.eMalletState = MALLETSTATE_IDLE;

		if ( a_bNewGame == TTRUE )
		{
			rPlayer.iLives          = m_aInts[ INT_LIVES ];
			rPlayer.iNumFreddyHits  = 0;
			rPlayer.iLevelScore     = 0;
			rPlayer.iRaccoonScore   = 0;
			rPlayer.iEggScore       = 0;
			rPlayer.iPeckScore      = 0;
			rPlayer.iCoyoteScore    = 0;
			rPlayer.iNumMisses      = 0;
			rPlayer.iScore          = 0;
			rPlayer.iNumMisses2     = 0;
			rPlayer.iPrevLevelScore = 0;
			rPlayer.iLevel          = s_iStartLevel;
			rPlayer.bFailed         = TFALSE;
		}
	}

	if ( a_bNewGame == TTRUE )
	{
		for ( TINT i = 0; i < NUM_DIALS; i++ )
			m_aDials[ i ].fAngle = 0.0f;
	}
}

// $Barnyard: FUNCTION 004d9330
TBOOL AWhackARaccMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( !m_oStartHudElement.IsForceHidden() || m_fMessageTimer > 0.0f || IsGameOver() || GetCurrentPlayer().iLives == 0 )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	TBOOL   bHandled = TFALSE;
	Player& rPlayer  = GetCurrentPlayer();

	switch ( a_eInputCommand )
	{
		case AInputCommand_Move:
			if ( a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				// Moving the mouse switches to mouse control
				if ( 0.0f != a_pInputEvent->GetMagnitudeFloat( 0 ) && 0.0f != a_pInputEvent->GetMagnitudeFloat( 1 ) )
				{
					m_bMouseControl = TTRUE;
					m_bKeyboardMove = TFALSE;
				}
			}
			else
			{
				bHandled = TTRUE;

				TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
				TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

				if ( TMath::Abs( fX ) > 0.001f || TMath::Abs( fY ) > 0.001f || !m_bMouseControl )
				{
					if ( fX < -STICK_DEADZONE )
						rPlayer.iCol = 0;
					else if ( fX > STICK_DEADZONE )
						rPlayer.iCol = 2;
					else if ( !s_bColHeld )
						rPlayer.iCol = 1;

					if ( fY < -STICK_DEADZONE )
						rPlayer.iRow = 2;
					else if ( fY > STICK_DEADZONE )
						rPlayer.iRow = 0;
					else if ( !s_bRowHeld )
						rPlayer.iRow = 1;

					m_bMouseControl = TFALSE;
					m_bKeyboardMove = TFALSE;
				}
			}
			break;
		case AInputCommand_LeftWithUp:
			// The ring goes back to the middle when the key is released
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				s_bColHeld = TFALSE;
				rPlayer.iCol += 1;
			}
			else
			{
				s_bColHeld = TTRUE;
				rPlayer.iCol -= 1;
			}

			TMath::Clip( rPlayer.iCol, 0, 2 );
			bHandled        = TTRUE;
			m_bMouseControl = TFALSE;
			m_bKeyboardMove = TTRUE;
			break;
		case AInputCommand_RightWithUp:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				s_bColHeld = TFALSE;
				rPlayer.iCol -= 1;
			}
			else
			{
				s_bColHeld = TTRUE;
				rPlayer.iCol += 1;
			}

			TMath::Clip( rPlayer.iCol, 0, 2 );
			bHandled        = TTRUE;
			m_bMouseControl = TFALSE;
			m_bKeyboardMove = TTRUE;
			break;
		case AInputCommand_ForwardWithUp:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				s_bRowHeld = TFALSE;
				rPlayer.iRow -= 1;
			}
			else
			{
				s_bRowHeld = TTRUE;
				rPlayer.iRow += 1;
			}

			TMath::Clip( rPlayer.iRow, 0, 2 );
			bHandled        = TTRUE;
			m_bMouseControl = TFALSE;
			break;
		case AInputCommand_BackwardWithUp:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				s_bRowHeld = TFALSE;
				rPlayer.iRow += 1;
			}
			else
			{
				s_bRowHeld = TTRUE;
				rPlayer.iRow -= 1;
			}

			TMath::Clip( rPlayer.iRow, 0, 2 );
			bHandled        = TTRUE;
			m_bMouseControl = TFALSE;
			m_bKeyboardMove = TTRUE;
			break;
		case AInputCommand_Shoot:
			if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || m_bBadHit )
				break;

			bHandled = TTRUE;

			if ( m_aHoles[ rPlayer.iHole ].fCooldown <= 0.0f && m_fWhackCooldown <= 0.0f )
			{
				// Swing the mallet at the hole under the ring
				m_pMallet->FinishAnimation( m_aHoleRecAnims[ rPlayer.iMalletHole ] );
				m_pMallet->FinishAnimation( m_aHoleImpAnims[ rPlayer.iMalletHole ] );
				m_pMallet->FinishAnimation( m_aHoleRstAnims[ rPlayer.iMalletHole ] );
				m_pMallet->FinishAnimation( TPS8( reset ) );
				m_pMallet->FinishAnimation( TPS8( idle ) );
				m_pMallet->FinishAnimation( TPS8( impact ) );

				rPlayer.eMalletState                = MALLETSTATE_SWING;
				rPlayer.iMalletHole                 = rPlayer.iHole;
				m_aHoles[ rPlayer.iHole ].fCooldown = m_aFloats[ FLOAT_HOLE_COOLDOWN ];
				m_fWhackCooldown                    = m_aFloats[ FLOAT_WHACK_COOLDOWN ];
			}
			break;
	}

	rPlayer.iHole = rPlayer.iCol + rPlayer.iRow * NUM_ROWS;

	if ( !bHandled )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	return bHandled;
}

// $Barnyard: FUNCTION 004d98c0
void AWhackARaccMiniGameState::OnIntroCueReset( TINT a_iType, TINT a_iCueIndex, void* a_pUserData )
{
	TSTATICCAST( AWhackARaccMiniGameState, a_pUserData )->m_iIntroCue = -1;
}

// $Barnyard: FUNCTION 004d98e0
void AWhackARaccMiniGameState::OnRemoval()
{
	m_pLifeHud->Destroy();

	if ( m_pMessage )
		delete m_pMessage;

	if ( m_pLifeHud )
		delete m_pLifeHud;

	m_oPortrait.Destroy();
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	for ( TINT i = 0; i < m_iNumLevels; i++ )
		TFree( m_aLevels[ i ].pForms );

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	pAnimObjManager->DestroyAnimatableObject( m_pMallet );
	pAnimObjManager->DestroyAnimatableObject( m_pRing );
	pAnimObjManager->DestroyAnimatableObject( m_pBadRing );
	pAnimObjManager->DestroyAnimatableObject( m_pArrow );

	for ( TINT i = 0; i < NUM_BUCKS; i++ )
		pAnimObjManager->DestroyAnimatableObject( m_aBucks[ i ].pObject );

	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
		pAnimObjManager->DestroyAnimatableObject( m_aRaccoons[ i ].pObject );

	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
		pAnimObjManager->DestroyAnimatableObject( m_aSpecials[ i ].pObject );

	for ( TINT i = 0; i < NUM_DIALS; i++ )
		pAnimObjManager->DestroyAnimatableObject( m_aDials[ i ].pObject );

	m_oScoreFloats.Destroy();
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AGameLoader::DestroyLevel();
	AUnitManager::GetSingleton()->RemoveAllUnits();
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	ASoundManager::GetSingleton()->RemoveCueCallback( m_iIntroCue, 1, OnIntroCueReset );

	// Turn the sky and the sun flare back on
	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	AMiniGameState::OnRemoval();
}

// NOTE: inlined in the original (00004d9ab0, 004da480, 004dd470, 004ddc00, 004ddf50)
void AWhackARaccMiniGameState::HideHud()
{
	m_pMessage->SetText( TNULL, 0x55000000, TNULL );
	m_oTimeLeftHudElement.SetForceHide( TTRUE );
	m_pLifeHud->SetVisible( TFALSE );
	m_oPortrait.Hide();
	m_oScoreFloats.Hide();
	m_oPickUpHudElement.SetForceHide( TTRUE );
}

// NOTE: inlined in the original (004dd470)
void AWhackARaccMiniGameState::ShowHud()
{
	m_oTimeLeftHudElement.SetForceHide( TFALSE );
	m_pLifeHud->SetVisible( TTRUE );
	m_oPortrait.Show();
	m_oScoreFloats.Show();
	m_oPickUpHudElement.SetForceHide( TFALSE );
}

// NOTE: inlined in the original (004da480, 004ddc00, 004ddf50)
void AWhackARaccMiniGameState::ResetDials()
{
	for ( TINT i = 0; i < NUM_DIALS; i++ )
	{
		m_aDials[ i ].fAngle = 0.0f;
		m_aDials[ i ].pObject->GetModelInstance()->SetTranslation( m_aDials[ i ].vPos.AsVector3() );
		m_aDials[ i ].pObject->GetModelInstance()->SetDirection( s_vDialDir );
	}
}

// $Barnyard: FUNCTION 004d9ab0
void AWhackARaccMiniGameState::EndGame()
{
	m_bGameEnding = TFALSE;
	m_bWon        = TFALSE;

	ASoundManager::GetSingleton()->StopCue( m_iUnkCue );
	m_iUnkCue = -1;
	ASoundManager::GetSingleton()->StopCue( m_iLoopCue );
	m_iLoopCue = -1;

	AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );
	m_bTurnEnded      = TTRUE;
	m_bRestartPending = TTRUE;

	m_oTimeLeftHudElement.SetForceHide( TTRUE );
	m_oPickUpHudElement.SetNumber( 0 );
	HideHud();
}

// $Barnyard: FUNCTION 004d9bd0
void AWhackARaccMiniGameState::RemoveBuck( Mole* a_pBuck )
{
	a_pBuck->pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	a_pBuck->eBuckState = BUCKSTATE_NONE;
	a_pBuck->eState     = MOLESTATE_HIDDEN;

	if ( a_pBuck->iHole != -1 )
	{
		m_aHoles[ a_pBuck->iHole ].bOccupied = TFALSE;
		m_aHoles[ a_pBuck->iHole ].iObject   = -1;
	}
}

// $Barnyard: FUNCTION 004d9c20
void AWhackARaccMiniGameState::OnActivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );
	m_pMessage->ShowIfHasText();

	GetCurrentPlayer().iRow = 1;
	GetCurrentPlayer().iCol = 1;

	m_oLevelText.Show();
	AMiniGameState::OnActivate();
}

// $Barnyard: FUNCTION 004d9c70
void AWhackARaccMiniGameState::OnDeactivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	m_pMessage->Hide();
	m_oLevelText.Hide();
	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 004d9ca0
TBOOL AWhackARaccMiniGameState::IsGameOver()
{
	AGUI2Timer& rTimer = m_oTimeLeftHudElement.GetTimer();

	// Wait for the last tick of the clock
	if ( !rTimer.IsTickComplete() && rTimer.IsTimerFull() )
		return TFALSE;

	return m_bGameEnding || m_bTurnEnded || ( GetCurrentPlayer().iLives == 0 && !ASoundManager::GetSingleton()->IsCueUsed( m_iVoiceCue ) );
}

// $Barnyard: FUNCTION 004d9d20
void AWhackARaccMiniGameState::OnPause()
{
	m_iIntroCue = ASoundManager::GetSingleton()->PlayCue( SOUND_INTRO );
	m_iCrowdCue = ASoundManager::GetSingleton()->PlayCue( SOUND_CROWD );
	ASoundManager::GetSingleton()->AddCueCallback( m_iIntroCue, 1, OnIntroCueReset, this );

	m_pMallet->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pRing->SetSkeletonUpdating( TTRUE, TTRUE );

	ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->GetMatrix().GetTranslation() = ATerrainInterface::GetSingleton()->GetLocatorPos( "camera" );
	pCamera->LookAtPoint( ATerrainInterface::GetSingleton()->GetLocatorPos( "camera_target" ) );

	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );
	AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );
	AGameState::OnPause();
}

// $Barnyard: FUNCTION 004d9e10
void AWhackARaccMiniGameState::PlayLoseSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sswar_lose", "sswar_lose", "SSwar_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d9ea0
void AWhackARaccMiniGameState::PlayWinSequence()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sswar_win", "sswar_win", "SSwar_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d9f30
void AWhackARaccMiniGameState::Pause()
{
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "sswar_intro", "sswar_intro", "SSwar_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 004d9fc0
void AWhackARaccMiniGameState::OnPreStart()
{
	m_pMallet->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pRing->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pBadRing->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < NUM_BUCKS; i++ )
		m_aBucks[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
		m_aRaccoons[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
		m_aSpecials[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
}

// $Barnyard: FUNCTION 004da070
void AWhackARaccMiniGameState::OnObjectCollected( AAnimatableObject* a_pObject )
{
	// A barn buck has flown into the pick up HUD
	Mole* pBuck = TNULL;

	for ( TINT i = 0; i < NUM_BUCKS; i++ )
	{
		if ( m_aBucks[ i ].pObject == a_pObject )
		{
			pBuck = &m_aBucks[ i ];
			break;
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original doesn't check if the object is a barn buck
	if ( !pBuck )
		return;
#endif

	RemoveBuck( pBuck );
}

// $Barnyard: FUNCTION 004da0e0
AMiniGameState::S1* AWhackARaccMiniGameState::GetResults()
{
	TINT iNumPlayers = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
		m_oResults.oPlayerList[ i ].bUnk1 = TFALSE;

	if ( iNumPlayers <= 1 )
	{
		m_oResults.oPlayerList[ 0 ].bUnk1 = TTRUE;
	}
	else
	{
		for ( TINT i = 0; i < iNumPlayers; i++ )
			m_oResults.oPlayerList[ i ].bUnk1 = TTRUE;
	}

	m_oResults.bWon = m_bWon;

	TINT        iPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();
	PlayerInfo& rInfo   = m_oResults.oPlayerList[ iPlayer ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	Player& rPlayer = m_aPlayers[ iPlayer ];
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_RACCOONS ), rPlayer.iRaccoonScore } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_COYOTES ), rPlayer.iCoyoteScore } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_EGGS ), rPlayer.iEggScore } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_PECKS ), rPlayer.iPeckScore } );
	m_oResults.ComputeTotal( iPlayer );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 0 )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE, TFLOAT( m_oResults.oPlayerList[ 0 ].iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 004da480
void AWhackARaccMiniGameState::StartTurn( TBOOL a_bNewGame )
{
	for ( TINT i = 0; i < NUM_BUCKS; i++ )
		m_aBucks[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
		m_aRaccoons[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
		m_aSpecials[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	if ( a_bNewGame == TTRUE )
		ResetDials();

	for ( TINT i = 0; i < m_iNumLevels; i++ )
	{
		for ( TINT k = 0; k < m_aLevels[ i ].iNumForms; k++ )
			m_aLevels[ i ].pForms[ k ].bTriggered = TFALSE;
	}

	ResetLevel( a_bNewGame );

	if ( m_fMessageTimer < 0.0f )
	{
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		m_oStartHudElement.Setup( pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount(), m_iCurrentPlayer );
		HideHud();
		m_bShowLevelText = TTRUE;
	}

	m_oTimeLeftHudElement.Setup( m_aLevels[ GetCurrentPlayer().iLevel ].fTime );
	m_pLifeHud->SetLives( GetCurrentPlayer().iLives );
	m_bSubmitHighScore = TTRUE;
}

// $Barnyard: FUNCTION 004da6f0
void AWhackARaccMiniGameState::UpdateSpecials( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
	{
		Mole& rSpecial = m_aSpecials[ i ];
		TBOOL bFree    = TFALSE;

		switch ( rSpecial.eState )
		{
			case MOLESTATE_HIT:
				if ( !rSpecial.pObject->HasAnimation( TPS8( hit ) ) || rSpecial.pObject->IsAnimationFinished( TPS8( hit ) ) == TTRUE )
					rSpecial.eState = MOLESTATE_FREE;
				break;
			case MOLESTATE_FREE:
				bFree = TTRUE;
				break;
			case MOLESTATE_RISING:
				if ( !rSpecial.pObject->HasAnimation( TPS8( up ) ) || rSpecial.pObject->IsAnimationFinished( TPS8( up ) ) == TTRUE )
				{
					rSpecial.eState = MOLESTATE_UP;

					if ( rSpecial.pObject->HasAnimation( TPS8( idle_1 ) ) )
						rSpecial.pObject->PlayAnimation( TPS8( idle_1 ) );

					if ( i == SPECIAL_FREDDY )
						m_iFreddyCue = ASoundManager::GetSingleton()->PlayCue( SOUND_FREDDY_UP );
				}
				break;
			case MOLESTATE_UP:
				rSpecial.fTimer -= a_fDeltaTime;

				if ( rSpecial.fTimer <= 0.0f )
				{
					rSpecial.eState = MOLESTATE_HIDING;

					if ( rSpecial.pObject->HasAnimation( TPS8( down ) ) )
						rSpecial.pObject->PlayAnimation( TPS8( down ) );
				}
				break;
			case MOLESTATE_HIDING:
				if ( rSpecial.pObject->HasAnimation( TPS8( down ) ) && rSpecial.pObject->IsAnimationFinished( TPS8( down ) ) != TTRUE )
					break;

				// It got away, no perfect level if it's worth points
				switch ( i )
				{
					case SPECIAL_FREDDY:
						if ( m_aInts[ INT_FREDDY_SCORE ] > 0 ) GetCurrentPlayer().bPerfect = TFALSE;
						break;
					case SPECIAL_PECK:
						if ( m_aInts[ INT_PECK_SCORE ] > 0 ) GetCurrentPlayer().bPerfect = TFALSE;
						break;
					case SPECIAL_EGG:
						if ( m_aInts[ INT_EGG_SCORE ] > 0 ) GetCurrentPlayer().bPerfect = TFALSE;
						break;
					case SPECIAL_COYOTE:
						if ( m_aInts[ INT_COYOTE_SCORE ] > 0 ) GetCurrentPlayer().bPerfect = TFALSE;
						break;
				}

				bFree = TTRUE;
				break;
		}

		if ( bFree )
		{
			rSpecial.eState                      = MOLESTATE_HIDDEN;
			m_aHoles[ rSpecial.iHole ].bOccupied = TFALSE;
			m_aHoles[ rSpecial.iHole ].iObject   = -1;
		}

		rSpecial.pObject->GetModelInstance()->SetTranslation( rSpecial.vPos.AsVector3() );
		rSpecial.pObject->SetSkeletonUpdating( rSpecial.eState != MOLESTATE_HIDDEN, TTRUE );
	}
}

// $Barnyard: FUNCTION 004da930
void AWhackARaccMiniGameState::Whack( TINT a_iPlayer )
{
	Player& rPlayer = m_aPlayers[ a_iPlayer ];
	Hole&   rHole   = m_aHoles[ rPlayer.iMalletHole ];

	if ( !rHole.bOccupied )
	{
		// Missed
		rPlayer.iNumMisses2++;
		rPlayer.iNumMisses++;
		ASoundManager::GetSingleton()->PlayCue( SOUND_MISS );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_MISS ], rHole.vPos, s_vEffectDir );
		return;
	}

	TFLOAT fRumbleStrength = 0.5f;
	TINT   iScore          = 0;

	auto IsUp = []( const Mole& a_rMole ) {
		return a_rMole.eState != MOLESTATE_HIDDEN && a_rMole.eState != MOLESTATE_HIT && a_rMole.eState != MOLESTATE_FREE;
	};

	if ( rHole.eKind == KIND_RACCOON && IsUp( m_aRaccoons[ rHole.iObject ] ) )
	{
		Mole& rRaccoon = m_aRaccoons[ rHole.iObject ];

		ASoundManager::GetSingleton()->PlayCue( SOUND_HIT );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_PUFF ], rHole.vPos, s_vEffectDir );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_DUST ], rHole.vPos, s_vEffectDir );

		rPlayer.iLevelScore += m_aInts[ INT_RACCOON_SCORE ];
		rPlayer.iScore += m_aInts[ INT_RACCOON_SCORE ];
		rPlayer.iRaccoonScore += m_aInts[ INT_RACCOON_SCORE ];
		rRaccoon.eState = MOLESTATE_HIT;

		if ( rRaccoon.pObject->IsAnimPlaying( TPS8( idle_1 ), 0.0f ) == TTRUE )
			rRaccoon.pObject->FinishAnimation( TPS8( idle_1 ) );

		rRaccoon.pObject->PlayAnimation( TPS8( hit ) );
	}
	else if ( rHole.eKind == KIND_BUCK && IsUp( m_aBucks[ rHole.iObject ] ) )
	{
		Mole& rBuck = m_aBucks[ rHole.iObject ];

		ASoundManager::GetSingleton()->PlayCue( SOUND_HIT );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_PUFF ], rHole.vPos, s_vEffectDir );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_DUST ], rHole.vPos, s_vEffectDir );

		// The buck flies into the pick up HUD (see OnObjectCollected)
		rBuck.eState                      = MOLESTATE_HIT;
		m_aHoles[ rBuck.iHole ].bOccupied = TFALSE;
		m_aHoles[ rBuck.iHole ].iObject   = -1;
		StartCollectEffect( rBuck.pObject, 1, 30.0f );
	}
	else if ( rHole.eKind == KIND_SPECIAL && IsUp( m_aSpecials[ rHole.iObject ] ) )
	{
		m_aSpecials[ rHole.iObject ].eState = MOLESTATE_HIT;
		TINT iOldLevelScore                 = rPlayer.iLevelScore;

		switch ( rHole.iObject )
		{
			case SPECIAL_FREDDY:
			{
				ASoundManager::GetSingleton()->PlayCue( SOUND_HIT );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_PUFF ], rHole.vPos, s_vEffectDir );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_BAD ], rHole.vPos, s_vEffectDir );

				ASoundId eVoice = -1;

				switch ( rPlayer.iNumFreddyHits++ )
				{
					case 0: eVoice = SOUND_FREDDY_HIT_1; break;
					case 1: eVoice = SOUND_FREDDY_HIT_2; break;
					case 2: eVoice = SOUND_FREDDY_HIT_3; break;
				}

				ASoundManager::GetSingleton()->StopCue( m_iFreddyCue );
				m_iVoiceCue = ASoundManager::GetSingleton()->PlayCue( eVoice );

				// Lose a life, shake the camera and show the red ring
				rPlayer.iLives--;
				m_pLifeHud->AddLives( -1 );
				m_fShakeTimeLeft = m_aFloats[ FLOAT_SHAKE_DURATION ];
				m_fShakeTimer    = m_aFloats[ FLOAT_SHAKE_INTERVAL ];
				m_fBadRingTime   = m_aFloats[ FLOAT_BAD_RING_TIME ];
				m_bBadHit        = TTRUE;

				rPlayer.iRow        = 1;
				rPlayer.iCol        = 1;
				rPlayer.fRingPulse  = 0.0f;
				rPlayer.vBadRingPos = rHole.vPos;
				fRumbleStrength     = 2.0f;
				rPlayer.vBadRingPos.y -= 0.05f;

				m_pBadRing->SetSkeletonUpdating( TTRUE, TTRUE );
				m_pBadRing->SetOpacity( 1.0f, TTRUE );

				TMatrix44 matTransform;
				MakeFlatTransform( matTransform, GetCurrentPlayer().vBadRingPos, TMath::HALF_PI );
				m_pBadRing->GetModelInstance()->SetTransform( matTransform );
				break;
			}
			case SPECIAL_PECK:
				ASoundManager::GetSingleton()->PlayCue( SOUND_HIT_PECK );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_PUFF ], rHole.vPos, s_vEffectDir );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_BAD ], rHole.vPos, s_vEffectDir );

				rPlayer.iLevelScore += m_aInts[ INT_PECK_SCORE ];
				rPlayer.iScore += m_aInts[ INT_PECK_SCORE ];
				rPlayer.iPeckScore += m_aInts[ INT_PECK_SCORE ];
				iScore = m_aInts[ INT_PECK_SCORE ];
				break;
			case SPECIAL_EGG:
				ASoundManager::GetSingleton()->PlayCue( SOUND_HIT_EGG );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_EGGSPLAT ], rHole.vPos, s_vEffectDir );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_EGGSHELL ], rHole.vPos, s_vEffectDir );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_EGGYOLK ], rHole.vPos, s_vEffectDir );

				rPlayer.iLevelScore += m_aInts[ INT_EGG_SCORE ];
				rPlayer.iScore += m_aInts[ INT_EGG_SCORE ];
				rPlayer.iEggScore += m_aInts[ INT_EGG_SCORE ];
				iScore = m_aInts[ INT_EGG_SCORE ];
				break;
			case SPECIAL_COYOTE:
				ASoundManager::GetSingleton()->PlayCue( SOUND_HIT_COYOTE );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_PUFF ], rHole.vPos, s_vEffectDir );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_DUST ], rHole.vPos, s_vEffectDir );

				rPlayer.iLevelScore += m_aInts[ INT_COYOTE_SCORE ];
				rPlayer.iScore += m_aInts[ INT_COYOTE_SCORE ];
				rPlayer.iCoyoteScore += m_aInts[ INT_COYOTE_SCORE ];
				iScore = m_aInts[ INT_COYOTE_SCORE ];
				break;
		}

		if ( rPlayer.iLevelScore < iOldLevelScore )
			rPlayer.bPerfect = TFALSE;

		if ( iScore != 0 )
		{
			// Show the points rising from the ring
			TUINT32 uiColour = PackColour( 1.0f, 0.0f, 0.0f, 0.8f );
			TWCHAR* wszFloat = m_awszFloats[ m_iFloatIndex ];

			if ( iScore > 0 )
			{
				TStringManager::String16Format( wszFloat, 128, L"+%i", iScore );
				uiColour = PackColour( 0.13333334f, 0.33333334f, 0.86666667f, 0.8f );
			}
			else
			{
				TStringManager::String16Format( wszFloat, 128, L"%i", iScore );
			}

			TVector4 vRingPos;
			m_pRing->GetModelInstance()->GetPosition( vRingPos );

			// NOTE: the original passes a 7th argument (0) that AddTimedFloat doesn't use
			m_oScoreFloats.AddTimedFloat( wszFloat, uiColour, vRingPos, 1.0f, -0.1f, 1.0f );
			m_iFloatIndex = ( m_iFloatIndex + 1 ) % NUM_FLOATS;
		}

		if ( rPlayer.iLevelScore < 0 )
		{
			rPlayer.iScore      = 0;
			rPlayer.iLevelScore = 0;
		}
	}
	else
	{
		// The hole's object isn't up
		ASoundManager::GetSingleton()->PlayCue( SOUND_MISS );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_aEffects[ EFFECT_MISS ], rHole.vPos, s_vEffectDir );
		return;
	}

	rPlayer.fRingPulse = TMath::HALF_PI;

	AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, fRumbleStrength, 0.0f, 1.0f, 1.0f }, 2 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

// $Barnyard: FUNCTION 004db0b0
void AWhackARaccMiniGameState::UpdateDials( TFLOAT a_fDeltaTime )
{
	m_oPortrait.SetScore( GetCurrentPlayer().iScore );

	TINT iDivisor = 10;

	for ( TINT i = 0; i < NUM_DIALS; i++ )
	{
		Dial& rDial = m_aDials[ i ];

		// The dials show the digits of the level score
		TINT   iDigit       = GetCurrentPlayer().iLevelScore % iDivisor / ( iDivisor / 10 );
		TFLOAT fTargetAngle = TFLOAT( iDigit ) * TMath::PI * 0.2f;

		if ( iDigit == 0 && rDial.fAngle > 0.0f )
		{
			// From 9 to 0
			rDial.fAngle = TMath::PI * -0.2f;
		}
		else if ( rDial.fAngle < fTargetAngle )
		{
			rDial.fAngle += ( 4.0f - TFLOAT( i ) ) * a_fDeltaTime;

			if ( rDial.fAngle > fTargetAngle )
			{
				rDial.fAngle = fTargetAngle;
				ASoundManager::GetSingleton()->PlayCue( SOUND_DIAL );
			}
		}
		else if ( rDial.fAngle > fTargetAngle )
		{
			rDial.fAngle -= ( 4.0f - TFLOAT( i ) ) * a_fDeltaTime;

			if ( rDial.fAngle < fTargetAngle )
			{
				rDial.fAngle = fTargetAngle;
				ASoundManager::GetSingleton()->PlayCue( SOUND_DIAL );
			}
		}

		TMatrix44 matTransform;
		MakeFlatTransform( matTransform, rDial.vPos, TMath::HALF_PI );
		matTransform.RotateY( -rDial.fAngle );
		rDial.pObject->GetModelInstance()->SetTransform( matTransform );

		iDivisor *= 10;
	}
}

// $Barnyard: FUNCTION 004db2b0
void AWhackARaccMiniGameState::UpdateMallet( TFLOAT a_fDeltaTime )
{
	Player& rPlayer = GetCurrentPlayer();

	TPString8 strImpact  = m_aHoleImpAnims[ rPlayer.iMalletHole ];
	TPString8 strRecover = m_aHoleRecAnims[ rPlayer.iMalletHole ];
	TPString8 strReset   = m_aHoleRstAnims[ rPlayer.iMalletHole ];

	switch ( rPlayer.eMalletState )
	{
		case MALLETSTATE_IDLE:
			if ( !m_pMallet->IsAnimPlaying( TPS8( idle ), 0.0f ) )
				m_pMallet->PlayAnimation( TPS8( idle ) );
			break;
		case MALLETSTATE_SWING:
			if ( m_pMallet->IsAnimPlaying( TPS8( idle ), 0.0f ) == TTRUE )
				m_pMallet->FinishAnimation( TPS8( idle ) );

			m_pMallet->PlayAnimation( TPS8( impact ) );
			m_pMallet->PlayAnimation( strImpact );
			rPlayer.eMalletState = MALLETSTATE_IMPACT;
			break;
		case MALLETSTATE_IMPACT:
			if ( m_pMallet->IsAnimationFinished( strImpact ) == TTRUE )
				rPlayer.eMalletState = MALLETSTATE_IMPACT_DONE;
			break;
		case MALLETSTATE_IMPACT_DONE:
			rPlayer.eMalletState = MALLETSTATE_RECOVER;
			break;
		case MALLETSTATE_RECOVER:
			if ( m_pMallet->IsAnimationFinished( strRecover ) == TTRUE )
				rPlayer.eMalletState = MALLETSTATE_RECOVER_DONE;
			break;
		case MALLETSTATE_RECOVER_DONE:
			rPlayer.eMalletState = MALLETSTATE_RESET;
			break;
		case MALLETSTATE_RESET:
			if ( m_pMallet->IsAnimationFinished( strReset ) == TTRUE )
				rPlayer.eMalletState = MALLETSTATE_IDLE;
			break;
	}

	// The ring over the hole under the cursor pulses after a hit
	TFLOAT   fPulse = TMath::Sin( rPlayer.fRingPulse );
	TVector4 vPos   = m_aHoles[ rPlayer.iHole ].vPos;
	vPos.y -= 0.01f;

	TMatrix44 matTransform;
	MakeFlatTransform( matTransform, vPos, TMath::HALF_PI );

	TFLOAT fScale = fPulse * 0.2f + 1.0f;
	matTransform.Scale( fScale, fScale, fScale );
	m_pRing->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 004db600
void AWhackARaccMiniGameState::UpdateRaccoons( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
	{
		Mole& rRaccoon = m_aRaccoons[ i ];

		switch ( rRaccoon.eState )
		{
			case MOLESTATE_HIT:
				if ( rRaccoon.pObject->IsAnimationFinished( TPS8( hit ) ) == TTRUE )
					rRaccoon.eState = MOLESTATE_FREE;
				break;
			case MOLESTATE_FREE:
				rRaccoon.eState                      = MOLESTATE_HIDDEN;
				m_aHoles[ rRaccoon.iHole ].bOccupied = TFALSE;
				m_aHoles[ rRaccoon.iHole ].iObject   = -1;
				break;
			case MOLESTATE_RISING:
				if ( rRaccoon.pObject->GetModelInstance()->IsUpdatingSkeleton() && rRaccoon.pObject->IsAnimationFinished( TPS8( up ) ) == TTRUE )
				{
					rRaccoon.eState = MOLESTATE_UP;
					rRaccoon.pObject->PlayAnimation( TPS8( idle_1 ) );
				}
				break;
			case MOLESTATE_UP:
				rRaccoon.fTimer -= a_fDeltaTime;

				if ( rRaccoon.fTimer <= 0.0f )
				{
					rRaccoon.eState = MOLESTATE_HIDING;
					rRaccoon.pObject->PlayAnimation( TPS8( down ) );
				}
				break;
			case MOLESTATE_HIDING:
				if ( rRaccoon.pObject->IsAnimationFinished( TPS8( down ) ) == TTRUE )
				{
					// It got away
					rRaccoon.eState                      = MOLESTATE_HIDDEN;
					m_aHoles[ rRaccoon.iHole ].bOccupied = TFALSE;
					m_aHoles[ rRaccoon.iHole ].iObject   = -1;
					GetCurrentPlayer().bPerfect          = TFALSE;
				}
				break;
		}

		rRaccoon.pObject->GetModelInstance()->SetTranslation( rRaccoon.vPos.AsVector3() );
		rRaccoon.pObject->SetSkeletonUpdating( rRaccoon.eState != MOLESTATE_HIDDEN, TTRUE );
	}
}

// $Barnyard: FUNCTION 004db830
void AWhackARaccMiniGameState::LaunchBuck( Mole* a_pBuck )
{
	a_pBuck->pObject->SetSkeletonUpdating( TTRUE, TTRUE );

	// NOTE: the original gets the local transform and then overwrites it
	TMatrix44 matTransform;
	a_pBuck->pObject->GetModelInstance()->GetLocalTransform( matTransform );

	matTransform = TMatrix44::IDENTITY;
	matTransform.GetTranslation().Set( a_pBuck->vPos.x, a_pBuck->vPos.y + 0.5f, a_pBuck->vPos.z, a_pBuck->vPos.w );
	matTransform.RotateX( TMath::HALF_PI );
	a_pBuck->pObject->GetModelInstance()->SetTransform( matTransform );

	a_pBuck->vGround = a_pBuck->vPos;
	a_pBuck->vGround.y -= 0.15f;
	a_pBuck->fVelocity  = -2.8f;
	a_pBuck->fFallTime  = 0.0f;
	a_pBuck->eBuckState = BUCKSTATE_FLYING;
	a_pBuck->eState     = MOLESTATE_UP;
	a_pBuck->bLanded    = TFALSE;
	a_pBuck->pObject->GetModelInstance()->SetScale( TVector4( 0.5f, 0.5f, 0.5f, 1.0f ) );
}

// $Barnyard: FUNCTION 004db9e0
void AWhackARaccMiniGameState::UpdateBucks( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_BUCKS; i++ )
	{
		Mole&           rBuck          = m_aBucks[ i ];
		AModelInstance* pModelInstance = rBuck.pObject->GetModelInstance();

		if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() || rBuck.eBuckState == 4 || rBuck.eState == MOLESTATE_FREE )
			continue;

		TMatrix44 matTransform;
		pModelInstance->GetLocalTransform( matTransform );

		// Bounce on the ground (Y goes down)
		TVector4 vTranslation = matTransform.GetTranslation();
		TFLOAT   fY           = a_fDeltaTime * rBuck.fVelocity + vTranslation.y;
		rBuck.fFallTime += a_fDeltaTime;
		rBuck.fVelocity += rBuck.fFallTime * a_fDeltaTime * 20.0f;
		rBuck.bLanded = rBuck.bLanded || rBuck.fVelocity >= -0.001f;

		if ( fY > rBuck.vGround.y && rBuck.fVelocity > 0.0f && rBuck.eBuckState != BUCKSTATE_FALLING )
		{
			fY              = rBuck.vGround.y;
			rBuck.fVelocity = rBuck.fVelocity * ( ( rBuck.eBuckState == BUCKSTATE_HOPPING ) ? -0.5f : -0.25f );
			rBuck.fFallTime = 0.0f;
		}

		if ( rBuck.eBuckState == BUCKSTATE_FALLING && rBuck.vGround.y + 0.5f < fY )
			RemoveBuck( &rBuck );

		vTranslation.y                = fY;
		matTransform.GetTranslation() = vTranslation;
		matTransform.RotateZ( BUCK_SPIN * a_fDeltaTime );
		pModelInstance->SetTransform( matTransform );

		rBuck.fTimer -= a_fDeltaTime;

		if ( rBuck.fTimer < 0.0f && rBuck.bLanded )
		{
			if ( rBuck.fTimer >= -0.1f )
			{
				// One more hop before it falls through the ground
				if ( rBuck.eBuckState != BUCKSTATE_HOPPING )
				{
					rBuck.eBuckState = BUCKSTATE_HOPPING;
					rBuck.fVelocity  = -1.0f;
				}
			}
			else
			{
				rBuck.eBuckState = BUCKSTATE_FALLING;
			}
		}
	}
}

// $Barnyard: FUNCTION 004dbe00
void AWhackARaccMiniGameState::UpdateForms( TFLOAT a_fDeltaTime )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
	Level&   rLevel  = m_aLevels[ m_iLevel ];

	for ( TINT i = 0; i < rLevel.iNumForms; i++ )
	{
		Form& rForm = rLevel.pForms[ i ];

		if ( m_fElapsedTime < rForm.fTime || rForm.bTriggered )
			continue;

		rForm.bTriggered = TTRUE;

		TINT iCount     = rForm.iType;
		TINT iFormation = -1;
		TINT eKind      = KIND_SPECIAL;
		TINT iSpecial   = 0;

		if ( rForm.iType < 0 && rForm.iType == Form::TYPE_FREDDY )
		{
			iSpecial = SPECIAL_FREDDY;
			iCount   = 1;
		}
		else if ( rForm.iType == Form::TYPE_EGG )
		{
			iSpecial = SPECIAL_EGG;
			iCount   = 1;
		}
		else if ( rForm.iType == Form::TYPE_PECK )
		{
			iSpecial = SPECIAL_PECK;
			iCount   = 1;
		}
		else if ( rForm.iType == Form::TYPE_COYOTE )
		{
			iSpecial = SPECIAL_COYOTE;
			iCount   = 1;
		}
		else
		{
			if ( rForm.iType < 0 )
			{
				// A formation
				iFormation = -1 - rForm.iType;
				iCount     = m_aFormations[ iFormation ].iCount;
			}

			eKind = rForm.eKind;
		}

		for ( TINT k = 0; k < iCount; k++ )
		{
			// Find what pops out
			Mole* pMole   = TNULL;
			TINT  iObject = -1;

			if ( eKind == KIND_SPECIAL )
			{
				pMole = &m_aSpecials[ iSpecial ];
			}
			else if ( eKind == KIND_BUCK )
			{
				for ( TINT j = 0; j < NUM_BUCKS; j++ )
				{
					AModelInstance* pModelInstance = m_aBucks[ j ].pObject->GetModelInstance();

					if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() )
					{
						iObject = j;
						break;
					}
				}

				// NOTE: retail uses m_aBucks[ -1 ] if all of them are in use
				pMole = &m_aBucks[ iObject ];
			}
			else
			{
				for ( TINT j = 0; j < NUM_RACCOONS; j++ )
				{
					if ( m_aRaccoons[ j ].eState == MOLESTATE_HIDDEN )
					{
						iObject = j;
						break;
					}
				}

				// NOTE: retail uses m_aRaccoons[ -1 ] if all of them are in use
				pMole = &m_aRaccoons[ iObject ];
			}

#ifdef BARNYARD_COMMUNITY_PATCH
			if ( eKind != KIND_SPECIAL && iObject == -1 )
				continue;
#endif

			if ( 0.0f == rForm.fStayTime )
				pMole->fTimer = rRandom.GetFloat() * ( rLevel.fMaxStayTime - 0.05f ) + 0.05f;
			else
				pMole->fTimer = rForm.fStayTime;

			// Find the hole
			TINT iHole = -1;

			if ( rForm.iType >= 0 || ( eKind != KIND_RACCOON && eKind != KIND_BUCK ) )
			{
				// A random free hole
				TINT j = 0;

				for ( ; j < NUM_HOLES; j++ )
				{
					if ( !m_aHoles[ j ].bOccupied )
						break;
				}

				if ( j != NUM_HOLES )
				{
					do iHole = rRandom.GetInt( NUM_HOLES );
					while ( m_aHoles[ iHole ].bOccupied == TTRUE );
				}

#ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: retail uses m_aHoles[ -1 ] if all the holes are occupied
				if ( iHole == -1 )
					continue;
#endif
			}
			else
			{
				iHole = m_aFormations[ iFormation ].aHoles[ k ];

				if ( m_aHoles[ iHole ].bOccupied )
					continue;
			}

			Hole& rHole     = m_aHoles[ iHole ];
			rHole.bOccupied = TTRUE;

			if ( eKind == KIND_SPECIAL )
			{
				rHole.iObject = iSpecial;
				rHole.eKind   = KIND_SPECIAL;
				pMole->vPos   = rHole.vPos;
				pMole->vPos.y -= 0.05f;
			}
			else
			{
				rHole.iObject = iObject;
				rHole.eKind   = ( eKind == KIND_BUCK ) ? KIND_BUCK : KIND_RACCOON;
				pMole->vPos   = rHole.vPos;
			}

			pMole->eState = MOLESTATE_RISING;
			pMole->iHole  = iHole;

			if ( eKind == KIND_BUCK )
				LaunchBuck( pMole );
			else if ( pMole->pObject->HasAnimation( TPS8( up ) ) )
				pMole->pObject->PlayAnimation( TPS8( up ) );
		}
	}
}

// $Barnyard: FUNCTION 004dc1b0
TBOOL __stdcall AWhackARaccMiniGameState::OnMalletImpact( AWhackARaccMiniGameState* a_pThis, void* a_pOwner, void* a_pData )
{
	a_pThis->Whack( a_pThis->m_iCurrentPlayer );
	return TTRUE;
}

// $Barnyard: FUNCTION 004dc1d0
void AWhackARaccMiniGameState::StartGame()
{
	m_iLevel       = s_iStartLevel;
	m_fElapsedTime = 0.0f;
	APlayerManager::GetSingleton()->ResetPlayerIterator();
	m_iCurrentPlayer = 0;

	m_pMallet->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pMallet->SetVisible( TTRUE );

	if ( m_oImpactListener.IsLinked() )
		m_oImpactListener.Disconnect();

	m_oImpactListener.Connect( m_pMallet->GetAnimationEmitter( TPS8( impactbp ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnMalletImpact ), 0 );

	ResetLevel( TTRUE );

	// NOTE: the same as the new game part of ResetLevel
	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		Player& rPlayer         = m_aPlayers[ i ];
		rPlayer.iRow            = 1;
		rPlayer.iCol            = 1;
		rPlayer.iHole           = 4;
		rPlayer.fRingPulse      = 0.0f;
		rPlayer.eMalletState    = MALLETSTATE_IDLE;
		rPlayer.iLevelScore     = 0;
		rPlayer.iRaccoonScore   = 0;
		rPlayer.iEggScore       = 0;
		rPlayer.iPeckScore      = 0;
		rPlayer.iCoyoteScore    = 0;
		rPlayer.iNumMisses      = 0;
		rPlayer.iScore          = 0;
		rPlayer.iNumMisses2     = 0;
		rPlayer.iPrevLevelScore = 0;
		rPlayer.bFailed         = TFALSE;
		rPlayer.iLevel          = s_iStartLevel;
		rPlayer.iLives          = m_aInts[ INT_LIVES ];
		rPlayer.bPerfect        = TTRUE;
		rPlayer.iNumFreddyHits  = 0;
	}

	StartTurn( TTRUE );

	m_oTimeLeftHudElement.SetForceHide( TTRUE );
	m_oPickUpHudElement.SetNumber( 0 );
	m_bWon = TFALSE;
}

// $Barnyard: FUNCTION 004dc310
void AWhackARaccMiniGameState::OnInsertion()
{
	m_bShowPlayerFloats = TFALSE;
	AMiniGameState::OnInsertion();
	LoadData();

	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvWacARac, TFALSE, TTRUE, 0, 0, 0, 0 );
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	// Animations of the mallet for each hole
	TString8 strName;

	for ( TINT i = 0; i < NUM_HOLES; i++ )
	{
		strName.Format( "hole_%i_imp", i + 1 );
		m_aHoleImpAnims[ i ] = TPString8( strName );

		strName.Format( "hole_%i_rec", i + 1 );
		m_aHoleRecAnims[ i ] = TPString8( strName );

		strName.Format( "hole_%i_rst", i + 1 );
		m_aHoleRstAnims[ i ] = TPString8( strName );
	}

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	ATerrainInterface*        pTerrain        = ATerrainInterface::GetSingleton();

	for ( TINT i = 0; i < NUM_HOLES; i++ )
	{
		strName.Format( "hole_%i", i + 1 );
		m_aHoles[ i ].vPos      = pTerrain->GetLocatorPos( strName );
		m_aHoles[ i ].bOccupied = TFALSE;
		m_aHoles[ i ].fCooldown = 0.0f;
	}

	m_fWhackCooldown = 0.0f;

	// Raccoons
	AAnimatableObjectType* pType = pAnimObjManager->FindType( TPS8D( "racforwaca" ) );

	for ( TINT i = 0; i < NUM_RACCOONS; i++ )
	{
		Mole& rRaccoon   = m_aRaccoons[ i ];
		rRaccoon.pObject = pAnimObjManager->CreateAnimatableObject( pType, TNULL, 0 );
		rRaccoon.pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		rRaccoon.pObject->SetVisible( TTRUE );
		rRaccoon.pObject->GetModelInstance()->SetTranslation( TVector4::VEC_ZERO.AsVector3() );
		rRaccoon.pObject->GetModelInstance()->SetDirection( s_vMoleDir );
		rRaccoon.pObject->PlayAnimation( TPS8( idle_1 ) );
		rRaccoon.pObject->SetAnimationTime( TPS8( idle_1 ), 0.0f );
		rRaccoon.pObject->FinishAnimation( TPS8( idle_1 ) );
		rRaccoon.eState = MOLESTATE_HIDDEN;
		rRaccoon.vPos   = TVector4::VEC_ZERO;
	}

	// Barn bucks
	pType = pAnimObjManager->FindType( TPS8D( "barnbuck" ) );

	for ( TINT i = 0; i < NUM_BUCKS; i++ )
	{
		Mole& rBuck   = m_aBucks[ i ];
		rBuck.pObject = pAnimObjManager->CreateAnimatableObject( pType, TNULL, 0 );
		rBuck.pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		rBuck.pObject->SetVisible( TTRUE );
		rBuck.pObject->GetModelInstance()->SetTranslation( TVector4::VEC_ZERO.AsVector3() );
		rBuck.pObject->GetModelInstance()->SetDirection( s_vMoleDir );
		rBuck.pObject->GetModelInstance()->SetScale( TVector4( 0.5f, 0.5f, 0.5f, 1.0f ) );
		rBuck.eState = MOLESTATE_HIDDEN;
		rBuck.vPos   = TVector4::VEC_ZERO;
	}

	// The special characters
	m_aSpecials[ SPECIAL_FREDDY ].pObject = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "freddyforwaca" ) ), TNULL, 0 );
	m_aSpecials[ SPECIAL_PECK ].pObject   = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "peckforwaca" ) ), TNULL, 0 );
	m_aSpecials[ SPECIAL_EGG ].pObject    = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "eggforwaca" ) ), TNULL, 0 );
	m_aSpecials[ SPECIAL_COYOTE ].pObject = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "coyoteforwaca" ) ), TNULL, 0 );

	for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
	{
		Mole& rSpecial  = m_aSpecials[ i ];
		rSpecial.eState = MOLESTATE_HIDDEN;
		rSpecial.vPos   = TVector4::VEC_ZERO;
		rSpecial.pObject->SetVisible( TTRUE );
		rSpecial.pObject->GetModelInstance()->SetTranslation( TVector4::VEC_ZERO.AsVector3() );
		rSpecial.pObject->GetModelInstance()->SetDirection( s_vMoleDir );
	}

	// The clock hand
	m_pArrow = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "wacaarrow" ) ), TNULL, 0 );
	m_pArrow->GetModelInstance()->SetSkeletonUpdating( TTRUE );
	m_pArrow->SetVisible( TTRUE );
	m_vArrowPos = pTerrain->GetLocatorPos( "dial" );
	m_vArrowPos.x -= 0.1f;
	m_pArrow->GetModelInstance()->SetTranslation( m_vArrowPos.AsVector3() );
	m_pArrow->GetModelInstance()->SetDirection( s_vArrowDir );

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
	{
		m_aPlayers[ i ].iMalletHole = 0;
		m_aPlayers[ i ].iHole       = 4;
		m_aPlayers[ i ].vUnk2       = s_vDialDir;
		m_aPlayers[ i ].vUnk1       = s_vDialDir;
	}

	// The rings, drawn with their own render callbacks so they can turn red
	AAnimatableObjectType* pRingType = pAnimObjManager->FindType( TPS8D( "wackring" ) );

	m_pRing = pAnimObjManager->CreateAnimatableObject( pRingType, TNULL, 0 );
	m_pRing->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( RenderRingCallback, this );
	m_pRing->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pRing->SetVisible( TTRUE );

	m_pBadRing = pAnimObjManager->CreateAnimatableObject( pRingType, TNULL, 0 );
	m_pBadRing->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( RenderBadRingCallback, this );
	m_pBadRing->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pBadRing->SetVisible( TTRUE );

	m_pRing->GetModelInstance()->SetUnlit();
	m_pBadRing->GetModelInstance()->SetUnlit();

	// The mallet
	m_pMallet = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8D( "wacmallet" ) ), TNULL, 0 );
	m_pMallet->GetModelInstance()->SetTransform( TVector4::VEC_ZERO, s_vDialDir );
	m_pMallet->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pMallet->SetVisible( TFALSE );
	m_pMallet->GetModelInstance()->DisableDrawWithDefaultMaterials();
	m_pMallet->GetModelInstance()->SetUnlit();

	// The score dials
	AAnimatableObjectType* pDialType = pAnimObjManager->FindType( TPS8D( "wacadial" ) );

	for ( TINT i = 0; i < NUM_DIALS; i++ )
	{
		Dial& rDial = m_aDials[ i ];

		strName.Format( "score_%i", i + 1 );
		rDial.vPos    = pTerrain->GetLocatorPos( strName );
		rDial.fAngle  = 0.0f;
		rDial.pObject = pAnimObjManager->CreateAnimatableObject( pDialType, TNULL, 0 );
		rDial.pObject->GetModelInstance()->SetTranslation( rDial.vPos.AsVector3() );
		rDial.pObject->GetModelInstance()->SetDirection( s_vDialDir );
		rDial.pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		rDial.pObject->SetVisible( TTRUE );
	}

	// HUD
	m_oPortrait.Create( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), AGUI2Portrait::FLAGS_LEFT, &m_GUIElement );
	m_GUIElement.AddChildTail( m_oPortrait );
	m_oPortrait.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oScoreFloats.Create( &m_GUIElement, "rekord26" );

	// NOTE: the original sets a copy of the effect (AGameTimeFX copy constructor 00492d30)
	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "Whack" ) );

	m_oTimeLeftHudElement.GetTimer().SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTimeLeftHudElement.SetForceHide( TTRUE );

	m_pMessage = new AGUI2AnythingPopup();
	m_pLifeHud = new AGUI2LifeHud();
	m_pLifeHud->Create( &m_GUIElement, TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT, 1.0f, TFALSE );
	m_pLifeHud->SetVisible( TTRUE );

	StartGame();
	ASSeqController::GetSingleton()->LoadScript( "sswar_intro.tso", TVector4::VEC_ZERO );
	m_bMouseControl = TFALSE;

	// No sky and no sun flare in the barn
	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	m_bShowLevelText = TTRUE;
	m_oLevelText.Create( T2GUIFontManager::FindFont( "Rekord26" ), &m_GUIElement, T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oPickUpHudElement.SetNumber( 0 );
}

// $Barnyard: FUNCTION 004dd470
void AWhackARaccMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
	UpdateBucks( a_fDeltaTime );

	if ( IsGameOver() || m_fElapsedTime > m_fTimeLimit )
		return;

	if ( ARootTask::GetSingleton()->IsPaused() )
	{
		GetCurrentPlayer().iRow = 1;
		GetCurrentPlayer().iCol = 1;
		return;
	}

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return;

	for ( TINT i = 0; i < NUM_HOLES; i++ )
		m_aHoles[ i ].fCooldown -= a_fDeltaTime;

	m_fWhackCooldown -= a_fDeltaTime;

	if ( m_fMessageTimer > 0.0f )
	{
		// The game waits while the message is shown
		TMatrix44 matTransform;
		MakeFlatTransform( matTransform, m_vArrowPos, TMath::HALF_PI );
		m_pArrow->GetModelInstance()->SetTransform( matTransform );

		m_fMessageTimer -= a_fDeltaTime;
		m_pMessage->Update( a_fDeltaTime );

		if ( m_fMessageTimer >= 0.0f )
			return;

		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		m_oStartHudElement.Setup( pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount(), m_iCurrentPlayer );
		m_bShowLevelText = TTRUE;
		HideHud();
	}

	if ( !m_oStartHudElement.IsForceHidden() )
	{
		m_fElapsedTime = 0.0f;
		return;
	}

	if ( m_bShowLevelText == TTRUE )
	{
		ALocale::FormatTags( m_wszLevelText, TARRAYSIZE( m_wszLevelText ), LOCALE_LEVEL, PPropertyValue( m_iLevel + 1 ) );
		m_oLevelText.SetText( m_wszLevelText, 0x55000000 );
		m_bShowLevelText = TFALSE;
		ShowHud();
	}

	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );

	if ( m_bMouseControl )
	{
		// Move the ring to the hole under the mouse
		TVector4 vRayPos;
		TVector4 vRayDir;
		ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), AGUI2::GetMouseCursor().GetCursorPos(), vRayPos, vRayDir );

		TVector4 vRayEnd;
		vRayEnd.Multiply( vRayDir, 100.0f );
		vRayEnd.Add( vRayPos, vRayEnd );

		TINT   iClosestHole = -1;
		TFLOAT fClosestDist = MAX_PICK_DIST;

		for ( TINT i = 0; i < NUM_HOLES; i++ )
		{
			TFLOAT fDist = DistancePointToLineSq( m_aHoles[ i ].vPos, vRayPos, vRayEnd );

			if ( fDist < fClosestDist )
			{
				fClosestDist = fDist;
				iClosestHole = i;
			}
		}

		if ( iClosestHole != -1 )
		{
			GetCurrentPlayer().iCol = iClosestHole % NUM_ROWS;
			GetCurrentPlayer().iRow = iClosestHole / NUM_ROWS;
		}
	}

	if ( m_iLoopCue == -1 )
		m_iLoopCue = ASoundManager::GetSingleton()->PlayCue( SOUND_LOOP );

	if ( m_fShakeTimeLeft > 0.0f )
	{
		// Shake the camera after Freddy was hit
		m_fShakeTimeLeft -= a_fDeltaTime;
		m_fShakeTimer -= a_fDeltaTime;

		if ( m_fShakeTimer >= 0.0f || m_fShakeTimeLeft <= 0.0f )
		{
			m_vShake = TVector2::VEC_ZERO;
		}
		else
		{
			TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
			m_vShake.x       = rRandom.GetFloatMinMax( -m_aFloats[ FLOAT_SHAKE_AMOUNT ], m_aFloats[ FLOAT_SHAKE_AMOUNT ] );
			m_vShake.y       = rRandom.GetFloatMinMax( -m_aFloats[ FLOAT_SHAKE_AMOUNT ], m_aFloats[ FLOAT_SHAKE_AMOUNT ] );
			m_fShakeTimer    = m_aFloats[ FLOAT_SHAKE_INTERVAL ];
		}

		ACamera*   pCamera       = ACameraManager::GetSingleton()->GetCurrentCamera();
		TMatrix44& rMatrix       = pCamera->GetMatrix();
		rMatrix.GetTranslation() = ATerrainInterface::GetSingleton()->GetLocatorPos( "camera" );
		pCamera->LookAtPoint( ATerrainInterface::GetSingleton()->GetLocatorPos( "camera_target" ) );

		// NOTE: 0045b350 inlined, the negated up vector
		const TVector4& rUp = rMatrix.AsBasisVector4( BASISVECTOR_UP );
		TVector4        vDown( -rUp.x, -rUp.y, -rUp.z, 1.0f );

		const TVector4& rRight = rMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
		TVector4        vRight( rRight.x, rRight.y, rRight.z, 1.0f );

		TVector4& rTranslation = rMatrix.GetTranslation();
		rTranslation.x += vRight.x * m_vShake.x + vDown.x * m_vShake.y;
		rTranslation.y += vRight.y * m_vShake.x + vDown.y * m_vShake.y;
		rTranslation.z += vRight.z * m_vShake.x + vDown.z * m_vShake.y;
	}

	if ( m_bBadHit == TTRUE )
	{
		// The red ring grows and fades out
		TMatrix44 matTransform;
		MakeFlatTransform( matTransform, GetCurrentPlayer().vBadRingPos, TMath::HALF_PI );

		TFLOAT fScale = 2.0f - m_fBadRingTime / m_aFloats[ FLOAT_BAD_RING_TIME ];
		matTransform.Scale( fScale, fScale, fScale );
		m_pBadRing->GetModelInstance()->SetTransform( matTransform );
		m_pBadRing->SetOpacity( m_fBadRingTime / m_aFloats[ FLOAT_BAD_RING_TIME ], TTRUE );

		m_fBadRingTime -= a_fDeltaTime;

		if ( m_fBadRingTime <= 0.0f )
		{
			m_bBadHit = TFALSE;
			m_pBadRing->SetSkeletonUpdating( TFALSE, TTRUE );
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	UpdateSpecials( a_fDeltaTime );
	UpdateRaccoons( a_fDeltaTime );
	UpdateDials( a_fDeltaTime );
	UpdateMallet( a_fDeltaTime );
	UpdateForms( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004ddc00
void AWhackARaccMiniGameState::UpdateAlways( TFLOAT a_fDeltaTime )
{
	m_oLevelText.Update( a_fDeltaTime );

	if ( m_bRestartPending && !FindChildState( &TGetClass( AAnticGameOverState ) ) )
	{
		// The results are gone, start a new game
		m_bRestartPending = TFALSE;
		StartGame();
		m_bTurnEnded = TFALSE;
		m_iLevel     = GetCurrentPlayer().iLevel;

		ResetDials();

		for ( TINT i = 0; i < NUM_DIALS; i++ )
			m_aDials[ i ].fAngle = 0.0f;

		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		m_oStartHudElement.Setup( pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount(), m_iCurrentPlayer );
		HideHud();
		m_bShowLevelText = TTRUE;
	}

	if ( IsGameOver() || FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return;

	m_pRing->SetSkeletonUpdating( m_oStartHudElement.IsForceHidden(), TTRUE );

	if ( m_fMessageTimer > 0.0f )
		return;

	// The clock hand
	AGUI2Timer& rTimer       = m_oTimeLeftHudElement.GetTimer();
	TFLOAT      fElapsedTime = m_fElapsedTime;

	if ( !m_oStartHudElement.IsForceHidden() || m_fMessageTimer > 0.0f || ( !rTimer.IsTickComplete() && rTimer.IsTimerFull() ) )
		fElapsedTime = 0.0f;

	TFLOAT    fAngle = TMath::PI / m_fTimeLimit * fElapsedTime;
	TMatrix44 matTransform;
	MakeFlatTransform( matTransform, m_vArrowPos, fAngle + fAngle + TMath::HALF_PI );
	m_pArrow->GetModelInstance()->SetTransform( matTransform );

	if ( m_oStartHudElement.IsForceHidden() != TTRUE )
	{
		m_oTimeLeftHudElement.SetTime( m_fTimeLimit - m_fElapsedTime );
		GetCurrentPlayer().fRingPulse += a_fDeltaTime * 4.0f;
	}
}

// $Barnyard: FUNCTION 004ddf50
void AWhackARaccMiniGameState::UpdateGameOver( TFLOAT a_fDeltaTime )
{
	AGUI2Timer& rTimer = m_oTimeLeftHudElement.GetTimer();

	if ( !rTimer.IsTickComplete() && rTimer.IsTimerFull() )
	{
		TMatrix44 matTransform;
		MakeFlatTransform( matTransform, m_vArrowPos, TMath::HALF_PI );
		m_pArrow->GetModelInstance()->SetTransform( matTransform );
		return;
	}

	if ( m_bGameEnding == TTRUE )
	{
		// Wait for the raccoons and the message before showing the results
		UpdateRaccoons( a_fDeltaTime );
		m_pMessage->Update( a_fDeltaTime );

		if ( m_pMessage->m_wszPopupText || m_aRaccoons[ 0 ].eState != MOLESTATE_HIDDEN )
			return;

		EndGame();
		return;
	}

	Player& rPlayer = GetCurrentPlayer();
	rPlayer.bFailed = rPlayer.iLevelScore < m_aLevels[ m_iLevel ].iTargetScore || rPlayer.iLives == 0;

	TINT iNumPlayers = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount();

	if ( m_bTurnEnded == TTRUE )
	{
		if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
			return;

		if ( m_iCurrentPlayer == iNumPlayers - 1 )
		{
			StartGame();
			m_bTurnEnded = TFALSE;
			m_iLevel     = GetCurrentPlayer().iLevel;

			ResetDials();

			for ( TINT i = 0; i < NUM_DIALS; i++ )
				m_aDials[ i ].fAngle = 0.0f;

			m_oStartHudElement.Setup( iNumPlayers, m_iCurrentPlayer );
			HideHud();
			m_bShowLevelText = TTRUE;
			return;
		}
	}
	else if ( GetCurrentPlayer().bFailed == TTRUE && !FindChildState( &TGetClass( AAnticGameOverState ) ) )
	{
		if ( GetCurrentPlayer().iLives == 0 )
		{
			EndGame();
			return;
		}

		// Out of time: the game is lost, all the raccoons come out
		m_bGameEnding = TTRUE;

		for ( TINT i = 0; i < SPECIAL_NUMOF; i++ )
		{
			Mole& rSpecial = m_aSpecials[ i ];

			if ( rSpecial.eState != MOLESTATE_HIDDEN && rSpecial.eState != MOLESTATE_HIDING && !rSpecial.pObject->IsAnimPlaying( TPS8( down ), 0.0f ) )
			{
				rSpecial.pObject->PlayAnimation( TPS8( down ) );
				rSpecial.eState = MOLESTATE_HIDING;
			}
		}

		ShowMessage( ALocaleManager::Interface()->GetString( LOCALE_GAME_OVER ) );

		for ( TINT i = 0; i < NUM_RACCOONS; i++ )
		{
			Hole& rHole     = m_aHoles[ i ];
			Mole& rRaccoon  = m_aRaccoons[ i ];
			rHole.bOccupied = TTRUE;
			rHole.eKind     = KIND_RACCOON;
			rHole.iObject   = i;
			rRaccoon.vPos   = rHole.vPos;
			rRaccoon.eState = MOLESTATE_RISING;
			rRaccoon.iHole  = i;
			rRaccoon.fTimer = m_aFloats[ FLOAT_GAME_OVER_UPTIME ];

			if ( rRaccoon.pObject->HasAnimation( TPS8( up ) ) )
				rRaccoon.pObject->PlayAnimation( TPS8( up ) );
		}

		return;
	}

	Player& rCurPlayer = GetCurrentPlayer();

	if ( !rCurPlayer.bFailed && !m_bWon )
	{
		// Level passed
		m_bWon = TTRUE;
		rCurPlayer.iLevel++;
		m_oTimeLeftHudElement.SetTime( 0.0f );

		if ( rCurPlayer.iLevel != m_iNumLevels )
			return;

		// All the levels are done
		ASoundManager::GetSingleton()->StopCue( m_iUnkCue );
		m_iUnkCue = -1;
		ASoundManager::GetSingleton()->StopCue( m_iLoopCue );
		m_iLoopCue = -1;

		AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );
		m_bTurnEnded      = TTRUE;
		m_bRestartPending = TTRUE;
		HideHud();
		return;
	}

	TBOOL bAllDone = TTRUE;

	for ( TINT i = 0; i < iNumPlayers; i++ )
	{
		if ( !m_aPlayers[ i ].bFailed && m_aPlayers[ i ].iLevel < m_iNumLevels )
			bAllDone = TFALSE;
	}

	m_fElapsedTime = 0.0f;

	if ( bAllDone == TTRUE && !rCurPlayer.bFailed )
	{
		m_bWon = TTRUE;
		ASoundManager::GetSingleton()->StopCue( m_iUnkCue );
		m_iUnkCue = -1;

		AddChildState( new AAnticGameOverState( GetResults(), TFALSE ) );
		m_bTurnEnded      = TTRUE;
		m_bRestartPending = TTRUE;
		HideHud();
		return;
	}

	// Next level or next player
	m_bWon = TFALSE;
	ASoundManager::GetSingleton()->StopCue( m_iUnkCue );
	m_iUnkCue = -1;
	ASoundManager::GetSingleton()->StopCue( m_iLoopCue );
	m_iLoopCue = -1;
	rTimer.Reset();

	GetCurrentPlayer().iPrevLevelScore = GetCurrentPlayer().iLevelScore;
	GetCurrentPlayer().iLevelScore     = 0;

	if ( iNumPlayers > 1 )
	{
		m_pRing->SetSkeletonUpdating( TFALSE, TTRUE );
		m_pMallet->SetSkeletonUpdating( TFALSE, TTRUE );

		m_iCurrentPlayer = APlayerManager::GetSingleton()->GetNextPlayer();

		while ( m_aPlayers[ m_iCurrentPlayer ].bFailed == TTRUE )
			m_iCurrentPlayer = APlayerManager::GetSingleton()->GetNextPlayer();

		m_pMallet->SetSkeletonUpdating( TTRUE, TTRUE );

		if ( m_oImpactListener.IsLinked() )
			m_oImpactListener.Disconnect();

		m_oImpactListener.Connect( m_pMallet->GetAnimationEmitter( TPS8( impactbp ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnMalletImpact ), 0 );
	}

	if ( GetCurrentPlayer().bPerfect == TTRUE )
	{
		// Nothing got away
		m_fMessageTimer = m_aFloats[ FLOAT_MESSAGE_TIME ];
		ShowMessage( ALocaleManager::Interface()->GetString( LOCALE_PERFECT ) );
		m_oTimeLeftHudElement.SetForceHide( TTRUE );
	}
	else
	{
		m_fMessageTimer = -1.0f;
	}

	m_iLevel = GetCurrentPlayer().iLevel;
	StartTurn( TFALSE );
	m_bTurnEnded = TFALSE;

	ResetDials();

	for ( TINT i = 0; i < NUM_DIALS; i++ )
		m_aDials[ i ].fAngle = 0.0f;
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs move the ring to whatever is up
// and swing the mallet (not at Freddy).
void AWhackARaccMiniGameState::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fTimer = 0.0f;
	s_fTimer += a_fDeltaTime;

	if ( s_fTimer < 0.3f )
		return;

	for ( TINT i = 0; i < NUM_HOLES; i++ )
	{
		const Hole& rHole = m_aHoles[ i ];

		if ( !rHole.bOccupied || rHole.iObject < 0 )
			continue;

		const Mole* pMole = TNULL;

		if ( rHole.eKind == KIND_RACCOON )
			pMole = &m_aRaccoons[ rHole.iObject ];
		else if ( rHole.eKind == KIND_BUCK )
			pMole = &m_aBucks[ rHole.iObject ];
		else if ( rHole.iObject != SPECIAL_FREDDY )
			pMole = &m_aSpecials[ rHole.iObject ];

		if ( !pMole || ( pMole->eState != MOLESTATE_RISING && pMole->eState != MOLESTATE_UP ) )
			continue;

		Player& rPlayer = GetCurrentPlayer();
		rPlayer.iRow    = i / NUM_ROWS;
		rPlayer.iCol    = i % NUM_ROWS;

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		TBOOL                       bStopEvents = TFALSE;
		ProcessCommand( AInputCommand_Shoot, &oEvent, bStopEvents );

		TINFO( "[whackauto] whack hole %d (kind %d) score %d level %d lives %d\n", i, rHole.eKind, rPlayer.iScore, rPlayer.iLevel, rPlayer.iLives );
		s_fTimer = 0.0f;
		break;
	}
}

#endif
