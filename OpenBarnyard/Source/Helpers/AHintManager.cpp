#include "pch.h"
#include "AHintManager.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/AGateManager.h"
#include "Units/AInventoryComponentInterface.h"
#include "Units/AInventoryItem.h"
#include "Units/AGopherOpenRequirement.h"
#include "Units/ASpecialEventRequirement.h"
#include "Quests/AQuestManager.h"
#include "Quests/AQuestInstance.h"
#include "Quests/AQuest.h"
#include "AGameLoader.h"
#include "AGameTimeManager.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Assets/AAssetLoader.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Render/ARenderer.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AGameStateController.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitType.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitHash.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "GameInterface/ASimState.h"
#include "Helpers/ALadderBeansGame.h"

#include <GUI/T2GUIFontManager.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TSceneObject.h>
#include <Toshi/T2String8.h>

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/ARegrowthManager.h"
#include "Helpers/AReplayManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( honey_collecting_replay );

// NOTE: GetClass 0041ce50, CreateObject 0041f2d0, CreateObjectInPlace 0041f330
TDEFINE_CLASS_INIT( AHintManager );

TPSTRING8_DECLARE( rock );
TPSTRING8_DECLARE( coinspawnerstump );
TPSTRING8_DECLARE( egg );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( goldrush_token );
TPSTRING8_DECLARE( milkweed_seed );
TPSTRING8_DECLARE( tomatolaunch_battery );
TPSTRING8_DECLARE( tomatolaunch_cog );
TPSTRING8_DECLARE( tomatolaunch_pogospring );
TPSTRING8_DECLARE( tomatolaunch_steerwheel );
TPSTRING8_DECLARE( tomatolaunch_rope );
TPSTRING8_DECLARE( jp_fan );
TPSTRING8_DECLARE( jp_gramophone );
TPSTRING8_DECLARE( jp_umbrella );
TPSTRING8_DECLARE( jp_wateringcan );
TPSTRING8_DECLARE( jp_wheel );
TPSTRING8_DECLARE( barn_gate_key );
TPSTRING8_DECLARE( southpath_gate_key );
TPSTRING8_DECLARE( woods_gate_key );
TPSTRING8_DECLARE( golf_gate_key );
TPSTRING8_DECLARE( honey_gate_key );
TPSTRING8_DECLARE( junkyard_gate_key );
TPSTRING8_DECLARE( oldmill_gate_key );
TPSTRING8_DECLARE( gulch_gate_key );
TPSTRING8_DECLARE( nightbarn_gate_key );
TPSTRING8_DECLARE( ASSeqState );
TPSTRING8_DECLARE( ASimState );

// DAT_00780b10
AFunctionTable AHintManager::ms_oFunctionTable( &TGetClass( AHintManager ), TNULL );

// The keys of the gates, MARKER_GATE points at the gate of the key the player carries (0x007786d0)
static const TPString8* s_apGateKeys[] = {
	&TPS8( barn_gate_key ),
	&TPS8( southpath_gate_key ),
	&TPS8( woods_gate_key ),
	&TPS8( golf_gate_key ),
	&TPS8( honey_gate_key ),
	&TPS8( junkyard_gate_key ),
	&TPS8( oldmill_gate_key ),
	&TPS8( gulch_gate_key ),
	&TPS8( nightbarn_gate_key ),
};

// The markers are shown on top of everything (never set in retail)
static TBOOL s_bRenderMarkersOnTop = TFALSE; // 0x00780b08

static TFLOAT s_fBobAngle         = 0.0f; // 0x00780b04
static TFLOAT s_fHintNoteTimer    = 0.0f; // 0x007727a8
static TFLOAT s_fMarkerUpdateTime = 0.0f; // 0x007727b4

static constexpr TFLOAT MARKER_AMBIENT = 0.5f;  // 0x007727b0
static constexpr TFLOAT MARKER_LIGHT   = 0.75f; // 0x007727ac

// $Barnyard: FUNCTION 0041cdc0
AHintManager::State::State()
{
	iNumHints     = 0;
	iNumCollected = 0;
	iNumRecipes   = 0;

	for ( TINT i = MAX_RECIPES; i != 0; i-- )
		abRecipes[ i - 1 ] = TFALSE;

	for ( TINT i = MAX_HINTS; i != 0; i-- )
	{
		aHints[ i - 1 ].iNumLeft  = 0;
		aHints[ i - 1 ].fHideTime = 0.0f;
	}
}

// $Barnyard: FUNCTION 0041e280
AHintManager::AHintManager()
    : m_pHintNote( TNULL )
    , m_pHintNote2( TNULL )
    , m_iNumHintLocators( 0 )
    , m_iFacingHint( -1 )
    , m_iMaxRecipes( 0 )
    , m_fBob( 0.0f )
    , m_bActive( TFALSE )
    , m_bLoaded( TFALSE )
    , m_pHintPositions( TNULL )
    , m_pHintDirections( TNULL )
    , m_fOutOfMilkTime( 0.0f )
{
	for ( TINT i = EFFECT_NUMOF; i != 0; i-- )
		m_aiEffects[ i - 1 ] = -1;

	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		m_aiMarkerEffectIds[ i - 1 ]  = -1;
		m_abShowMarker[ i - 1 ]       = TFALSE;
		m_apMarkerEffects[ i - 1 ]    = TNULL;
		m_astrMarkerNames[ i - 1 ]    = TPString8();
		m_apMarkerModels[ i - 1 ]     = TNULL;
		m_avMarkerPositions[ i - 1 ]  = TVector4::VEC_ZERO;
		m_afMarkerDistanceSq[ i - 1 ] = 0.0f;
	}

	m_astrMarkerNames[ MARKER_TOMATO_LAUNCHER_PART ] = TPS8D( "Ben Tomato Launcher" );
	m_astrMarkerNames[ MARKER_EGG ]                  = TPS8D( "Hanna Missing Eggs" );
	m_astrMarkerNames[ MARKER_JUNKYARD_PART ]        = TPS8D( "Miles Junkyard Scores" );
	m_astrMarkerNames[ MARKER_FRUIT ]                = TPS8D( "Pig Apple Pie" );
	m_astrMarkerNames[ MARKER_GOLDRUSH ]             = TPS8D( "goldrush_token" );
	m_astrMarkerNames[ MARKER_MILKWEED ]             = TPS8D( "milkweed_seed" );
	m_astrMarkerNames[ MARKER_SAVE_ZONE ]            = TPS8D( "park2.triggers.save" );
	m_astrMarkerNames[ MARKER_SLEEP_ZONE ]           = TPS8D( "park2.triggers.sleep" );
}

// $Barnyard: FUNCTION 0041e750
// $Barnyard: FUNCTION 0041f370
AHintManager::~AHintManager()
{
}

// $Barnyard: FUNCTION 0041d2a0
void AHintManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &GetInstance ) );
}

// $Barnyard: FUNCTION 0041ce60
void AHintManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// FSM fallback: every script uses the single instance
// $Barnyard: FUNCTION 0041d290
AHintManager* __stdcall AHintManager::GetInstance( AFunctionTable* a_pTable, void* a_pCaller )
{
	return ms_pSingleton;
}

// $Barnyard: FUNCTION 0041e740
AFunctionTable* AHintManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0041ce70
TBOOL AHintManager::OnCreate()
{
	m_oOutOfMilkText.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oOutOfMilkText.T2GUIElement::Show();

	return TTRUE;
}

// $Barnyard: FUNCTION 0041f8d0
TBOOL AHintManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Fade the "out of milk" message out
	if ( m_fOutOfMilkTime <= 0.0f )
	{
		m_oOutOfMilkText.RemoveSelf();
	}
	else
	{
		m_oOutOfMilkText.SetAlpha( m_fOutOfMilkTime );
		m_fOutOfMilkTime -= a_fDeltaTime;
	}

	if ( !m_bActive )
		return TTRUE;

	m_vPlayerPos = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
	UpdateHintNotes( a_fDeltaTime );

	// The collected notes come back after a while
	for ( TINT i = m_iNumHintLocators; i != 0; i-- )
	{
		State::Hint& rHint = ms_oState.aHints[ i - 1 ];

		if ( rHint.fHideTime > 0.0f )
		{
			rHint.fHideTime -= a_fDeltaTime;

			if ( rHint.iNumLeft > 0 && rHint.fHideTime <= 0.0f )
				rHint.bAvailable = TTRUE;
		}
	}

	if ( !AGameLoader::IsMainGameLoaded() && !AGameLoader::IsBarnExploreMiniGame() )
		return TTRUE;

	if ( !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASSeqState ) ) )
	{
		UpdateMarkers( a_fDeltaTime );
		return TTRUE;
	}

	// No markers while a sequence is playing
	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		StopMarkerEffect( i - 1 );
		m_abShowMarker[ i - 1 ] = TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0041f390
void AHintManager::OnActivate()
{
	if ( m_bLoaded )
	{
		m_bActive = TTRUE;

		for ( MARKER eMarker = MARKER_SAVE_ZONE; eMarker < MARKER_NUMOF; eMarker++ )
			UpdateZoneMarker( eMarker );
	}
}

// $Barnyard: FUNCTION 0041d2b0
void AHintManager::OnDeactivate()
{
	m_bActive = TFALSE;

	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		m_abShowMarker[ i - 1 ] = TFALSE;

		if ( m_apMarkerEffects[ i - 1 ] )
		{
			m_apMarkerEffects[ i - 1 ]->GetInstance()->Kill();
			m_apMarkerEffects[ i - 1 ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0041e080
// $Barnyard: FUNCTION 0041e180
void AHintManager::Unload()
{
	if ( m_pHintPositions )
		TFree( m_pHintPositions );

	m_pHintPositions = TNULL;

	if ( m_pHintDirections )
		TFree( m_pHintDirections );

	m_pHintDirections = TNULL;

	AModelRepos* pModelRepos = AModelRepos::GetSingleton();

	if ( m_pHintNote )
	{
		pModelRepos->DestroyModelInstance( m_pHintNote );
		m_pHintNote = TNULL;
	}

	if ( m_pHintNote2 )
	{
		pModelRepos->DestroyModelInstance( m_pHintNote2 );
		m_pHintNote2 = TNULL;
	}

	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		if ( m_apMarkerModels[ i - 1 ] )
			pModelRepos->DestroyModelInstance( m_apMarkerModels[ i - 1 ] );

		m_apMarkerModels[ i - 1 ]    = TNULL;
		m_avMarkerPositions[ i - 1 ] = TVector4::VEC_ZERO;
	}

	m_iNumHintLocators = 0;
	m_bActive          = TFALSE;

	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		if ( m_apMarkerEffects[ i - 1 ] )
		{
			m_apMarkerEffects[ i - 1 ]->GetInstance()->Kill();
			m_apMarkerEffects[ i - 1 ] = TNULL;
		}

		m_afMarkerDistanceSq[ i - 1 ] = 0.0f;
	}

	m_oOutOfMilkText.RemoveSelf();
	m_bLoaded = TFALSE;
}

// $Barnyard: FUNCTION 0041e7d0
void AHintManager::Load()
{
	Unload();

	AAssetLoader::GetSymbolAddress( "cooking", "MaxIngredients", AAssetType_AssetPack );
	AAssetLoader::GetSymbolAddress( "cooking", "NumberOfIngredients", AAssetType_AssetPack );
	m_iMaxRecipes = *TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfRecipes", AAssetType_AssetPack ) );

	// Find the hint locators
	m_pHintPositions   = TSTATICCAST( TVector4, TMalloc( sizeof( TVector4 ) * MAX_HINTS ) );
	m_pHintDirections  = TSTATICCAST( TVector4, TMalloc( sizeof( TVector4 ) * MAX_HINTS ) );
	m_iNumHintLocators = 0;

	for ( TINT i = 0; i < MAX_HINTS; i++ )
	{
		TCHAR szLocator[ 64 ];
		T2String8::Format( szLocator, "hint%02d", i + 1 );

		if ( ATerrainInterface::GetSingleton()->GetLocatorPosAndDir( szLocator, m_pHintPositions[ m_iNumHintLocators ], m_pHintDirections[ m_iNumHintLocators ] ) )
			m_iNumHintLocators++;
	}

	for ( TINT i = m_iNumHintLocators; i != 0; i-- )
		ms_oState.aHints[ i - 1 ].bAvailable = TFALSE;

	// The notes shown at the closest hints
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	m_pHintNote              = pModelRepos->InstantiateNewModel( TPS8D( "data\\models\\hint_note" ), TNULL );
	m_pHintNote2             = pModelRepos->InstantiateNewModel( TPS8D( "data\\models\\hint_note" ), TNULL );

	if ( m_pHintNote && m_pHintNote2 )
	{
		m_pHintNote->SetSkeletonUpdating( TFALSE );
		m_pHintNote2->SetSkeletonUpdating( TFALSE );
	}
	else
	{
		m_iNumHintLocators = 0;
	}

	// The icons of the zone markers
	static constexpr const TCHAR* s_aszZoneModels[] = {
		"data\\models\\saveicon",   // MARKER_SAVE_ZONE
		"data\\models\\icon_sleep", // MARKER_SLEEP_ZONE
		"data\\models\\icon_enter", // MARKER_BARN_ENTRANCE
		"data\\models\\icon_food",  // MARKER_KITCHEN_ENTRANCE
		"data\\models\\icon_enter", // MARKER_WALNUT_ENTRANCE
		"data\\models\\icon_drink", // MARKER_MOCKTAIL_ENTRANCE
		"data\\models\\icon_enter", // MARKER_BARN_EXIT
		"data\\models\\icon_bucks", // MARKER_GOPHER_ENTRANCE
	};

	for ( TINT i = 0; i < TARRAYSIZE( s_aszZoneModels ); i++ )
	{
		AModelInstance* pModel                   = pModelRepos->InstantiateNewModel( TPString8( s_aszZoneModels[ i ] ), TNULL );
		m_apMarkerModels[ MARKER_SAVE_ZONE + i ] = pModel;

		// Make it stand up
		TMatrix44 mTransform;
		pModel->GetLocalTransform( mTransform );
		mTransform.RotateX( TMath::HALF_PI );
		pModel->SetTransform( mTransform );
		pModel->SetSkeletonUpdating( TFALSE );
	}

	if ( m_iNumHintLocators > 0 )
		SetNumHints( 1000 );

	m_aiEffects[ EFFECT_SPARKLE_WHITE ] = AParticleEffectManager::FindEffect( "sparklewhite" );
	m_aiEffects[ EFFECT_SPARKLE_GREEN ] = AParticleEffectManager::FindEffect( "sparklegreen" );
	m_aiEffects[ EFFECT_SAVE_ZONE ]     = AParticleEffectManager::FindEffect( "Savezone" );
	m_aiEffects[ EFFECT_SLEEP_ZONE ]    = AParticleEffectManager::FindEffect( "Sleepzone" );
	m_aiEffects[ EFFECT_DOOR_ZONE ]     = AParticleEffectManager::FindEffect( "Doorzone" );

	m_bActive = TFALSE;
	m_bLoaded = TTRUE;
}

// $Barnyard: FUNCTION 0041e190
void AHintManager::ShowOutOfMilkMessage()
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// As wide as the longest message
	TFLOAT fWidth1 = pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 349 ), 1.0f );
	TFLOAT fWidth2 = pFont->GetTextWidth( ALocaleManager::Interface()->GetString( 348 ), 1.0f );
	m_oOutOfMilkText.Create( pFont, TMath::Max( fWidth2, fWidth1 ) );

	m_fOutOfMilkTime = 1.0f;
	m_oOutOfMilkText.SetText( ALocaleManager::Interface()->GetString( 348 ) );
	m_oOutOfMilkText.SetTransform( 0.0f, AGUI2::GetRootElement()->GetHeight() * -0.1f );
	AGUI2::GetRootElement()->AddChildTail( m_oOutOfMilkText );
}

// $Barnyard: FUNCTION 0041d190
void AHintManager::SetNumHints( TINT a_iNumHints )
{
	TINT iNumHints = TMath::Min( a_iNumHints, MAX_HINTS );

	// Spread the new notes over the locators
	for ( TINT iNumNew = iNumHints - ms_oState.iNumHints; iNumNew != 0; )
	{
		iNumNew--;
		ms_oState.aHints[ iNumNew % m_iNumHintLocators ].iNumLeft++;
	}

	for ( TINT i = m_iNumHintLocators; i != 0; i-- )
	{
		State::Hint& rHint = ms_oState.aHints[ i - 1 ];
		rHint.fHideTime    = 0.0f;
		rHint.bAvailable   = TFALSE;

		if ( rHint.iNumLeft > 0 )
			rHint.bAvailable = TTRUE;
	}

	ms_oState.iNumHints = iNumHints;
}

// $Barnyard: FUNCTION 0041cff0
TBOOL AHintManager::IsFacingHint( AUnit* a_pUnit )
{
	if ( !m_bActive )
		return TFALSE;

	const TFLOAT fMaxAngle = TMath::HALF_PI * 0.67f;

	if ( !a_pUnit->GetClass()->IsA( &TGetClass( AUnitPlayer ) ) )
	{
		m_iFacingHint = -1;
		return TFALSE;
	}

	TVector4 vecPosition  = *a_pUnit->GetSteer()->GetPosition();
	TVector4 vecDirection = *a_pUnit->GetSteer()->GetDirection();

	for ( TINT i = m_iNumHintLocators; i != 0; i-- )
	{
		State::Hint& rHint = ms_oState.aHints[ i - 1 ];

		if ( rHint.iNumLeft > 0 && rHint.fHideTime <= 0.0f && TVector4::DistanceSq( vecPosition, m_pHintPositions[ i - 1 ] ) < 9.0f )
		{
			// Is the player looking at the note...
			TVector4 vecToHint = m_pHintPositions[ i - 1 ] - vecPosition;
			vecToHint.y        = 0.0f;

			if ( vecToHint.GetAngleBetweenVector( vecDirection ) < fMaxAngle )
			{
				// ...and is the note facing the player?
				vecToHint.x = -vecToHint.x;
				vecToHint.y = -vecToHint.y;
				vecToHint.z = -vecToHint.z;

				if ( vecToHint.GetAngleBetweenVector( m_pHintDirections[ i - 1 ] ) < fMaxAngle )
				{
					m_iFacingHint = i - 1;
					return TTRUE;
				}
			}
		}
	}

	m_iFacingHint = -1;
	return TFALSE;
}

// $Barnyard: FUNCTION 0041cf30
TINT AHintManager::CollectHint()
{
	if ( m_iFacingHint > m_iNumHintLocators || m_iFacingHint < 0 )
		return -1;

	if ( ms_oState.iNumCollected >= ms_oState.iNumHints )
		return -1;

	State::Hint& rHint = ms_oState.aHints[ m_iFacingHint ];

	if ( rHint.iNumLeft <= 0 )
		return -1;

	rHint.iNumLeft--;
	rHint.fHideTime = 10.0f;

	TINT iIndex                                  = ms_oState.iNumCollected++;
	ms_oState.aHints[ m_iFacingHint ].bAvailable = TFALSE;

	m_pHintNote->SetSkeletonUpdating( TFALSE );
	m_pHintNote2->SetSkeletonUpdating( TFALSE );
	m_iFacingHint = -1;

	return iIndex;
}

// $Barnyard: FUNCTION 0041cfc0
TINT AHintManager::AddRecipe()
{
	TINT iIndex = ms_oState.iNumRecipes;

	if ( iIndex >= m_iMaxRecipes )
		return -1;

	ms_oState.abRecipes[ ms_oState.iNumRecipes++ ] = TTRUE;
	return iIndex;
}

// $Barnyard: FUNCTION 0041d210
TBOOL __stdcall AHintManager::SaveState( void* a_pBuffer, void* a_pUnused )
{
	TUtil::MemCopy( a_pBuffer, &ms_oState, sizeof( ms_oState ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 0041d230
TBOOL __stdcall AHintManager::LoadState( void* a_pBuffer, void* a_pUnused )
{
	TUtil::MemCopy( &ms_oState, a_pBuffer, sizeof( ms_oState ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 0041cef0
void AHintManager::ResetState()
{
	ms_oState = State();
}

// Places the notes at the two closest hints the player can see
// $Barnyard: FUNCTION 0041d2f0
void AHintManager::UpdateHintNotes( TFLOAT a_fDeltaTime )
{
	s_fHintNoteTimer += a_fDeltaTime;

	if ( s_fHintNoteTimer < 0.5f )
		return;

	s_fHintNoteTimer = 0.0f;

	TINT   iClosestVisible       = -1;
	TINT   iSecondClosestVisible = -1;
	TINT   iClosest              = -1;
	TFLOAT fClosestVisible       = 10000.0f;
	TFLOAT fSecondClosestVisible = 10000.0f;
	TFLOAT fClosest              = 10000.0f;

	for ( TINT i = m_iNumHintLocators; i != 0; i-- )
	{
		if ( !ms_oState.aHints[ i - 1 ].bAvailable )
			continue;

		TFLOAT fDistanceSq = TVector4::DistanceSq( m_vPlayerPos, m_pHintPositions[ i - 1 ] );

		if ( fDistanceSq < fClosest )
		{
			fClosest = fDistanceSq;
			iClosest = i - 1;
		}

		// The closest two on the screen
		TVector2 vecScreen;

		if ( fDistanceSq >= fClosestVisible )
		{
			if ( fDistanceSq < fSecondClosestVisible && ARenderer::GetSingleton()->GetScreenPosition( m_pHintPositions[ i - 1 ], vecScreen ) )
			{
				fSecondClosestVisible = fDistanceSq;
				iSecondClosestVisible = i - 1;
			}
		}
		else if ( ARenderer::GetSingleton()->GetScreenPosition( m_pHintPositions[ i - 1 ], vecScreen ) )
		{
			iSecondClosestVisible = iClosestVisible;
			fSecondClosestVisible = fClosestVisible;
			fClosestVisible       = fDistanceSq;
			iClosestVisible       = i - 1;
		}
	}

	// NOTE: AModelInstance::SetDirection turns the model to face the direction (with VEC_NEGY up), the translation is kept
	auto PlaceNote = []( AModelInstance* a_pNote, const TVector4& a_rcPosition, const TVector4& a_rcDirection ) {
		a_pNote->GetTransform().SetTranslate( a_rcPosition.AsVector3() );

		TMatrix44 mTransform;
		AModelInstance::CreateRotationFromDirection( mTransform, a_rcDirection, TVector4::VEC_NEGY );
		mTransform.GetTranslation3() = a_rcPosition.AsVector3();
		a_pNote->GetTransform().SetMatrix( mTransform );
		a_pNote->SetSkeletonUpdating( TTRUE );
	};

	if ( iClosestVisible >= 0 )
		PlaceNote( m_pHintNote, m_pHintPositions[ iClosestVisible ], m_pHintDirections[ iClosestVisible ] );
	else
		m_pHintNote->SetSkeletonUpdating( TFALSE );

	if ( iClosest < 0 || iClosest == iClosestVisible )
	{
		if ( iSecondClosestVisible < 0 )
			m_pHintNote2->SetSkeletonUpdating( TFALSE );
		else
			PlaceNote( m_pHintNote2, m_pHintPositions[ iSecondClosestVisible ], m_pHintDirections[ iSecondClosestVisible ] );
	}
	else
	{
		PlaceNote( m_pHintNote2, m_pHintPositions[ iClosest ], m_pHintDirections[ iClosest ] );
	}
}

// $Barnyard: FUNCTION 0041e270
void AHintManager::Render( TFLOAT a_fDeltaTime )
{
	RenderMarkers( TFALSE );
}

// $Barnyard: FUNCTION 0041d500
void AHintManager::RenderMarkers( TBOOL a_bOnTop )
{
	if ( a_bOnTop && !s_bRenderMarkersOnTop )
		return;

	ACameraManager::GetSingleton()->GetCurrentCamera();

	TRenderInterface* pRender  = TRenderInterface::GetSingleton();
	TRenderContext*   pContext = pRender->GetCurrentContext();

	// Light the markers evenly
	TMatrix44 mOldLightColour = pRender->GetLightColour();
	TVector4  vecOldAmbient   = pContext->GetAmbientColour();
	TMatrix44 mLightColour    = mOldLightColour;

	TVector4 vecAmbient = vecOldAmbient;
	vecAmbient.w        = 0.0f;

	if ( a_bOnTop )
	{
		vecAmbient.x = 1.0f;
		vecAmbient.y = 1.0f;
		vecAmbient.z = 1.0f;

		mLightColour.m_f11 = 0.0f;
		mLightColour.m_f12 = 0.0f;
		mLightColour.m_f13 = 0.0f;
	}
	else
	{
		vecAmbient.x = MARKER_AMBIENT;
		vecAmbient.y = MARKER_AMBIENT;
		vecAmbient.z = MARKER_AMBIENT;

		mLightColour.m_f11 = MARKER_LIGHT;
		mLightColour.m_f12 = MARKER_LIGHT;
		mLightColour.m_f13 = MARKER_LIGHT;
	}

	mLightColour.m_f14 = 1.0f;
	pContext->SetAmbientColour( vecAmbient );
	pRender->SetLightColourMatrix( mLightColour );

	// The light comes from the camera
	TMatrix44 mOldLightDirection = pRender->GetLightDirection();
	TMatrix44 mLightDirection    = mOldLightDirection;

	const TMatrix44& rcCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	TVector4&        rLight   = mLightDirection.AsBasisVector4( BASISVECTOR_RIGHT );
	rLight.x                  = -rcCamera.m_f31 + rcCamera.m_f11 + rcCamera.m_f21;
	rLight.y                  = -rcCamera.m_f32 + rcCamera.m_f12 + rcCamera.m_f22;
	rLight.z                  = -rcCamera.m_f33 + rcCamera.m_f13 + rcCamera.m_f23;
	rLight.Normalise();
	pRender->SetLightDirectionMatrix( mLightDirection );

	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		AModelInstance* pModel = m_apMarkerModels[ i - 1 ];

		if ( pModel && m_abShowMarker[ i - 1 ] )
		{
			TSceneObject* pSceneObject = pModel->GetSceneObject();
			pSceneObject->EnableSkeletonUpdate();
			pSceneObject->RenderIfVisible();
			pSceneObject->DisableSkeletonUpdate();
		}
	}

	pContext->SetAmbientColour( vecOldAmbient );
	pRender->SetLightColourMatrix( mOldLightColour );
	pRender->SetLightDirectionMatrix( mOldLightDirection );
}

// $Barnyard: FUNCTION 0041d760
void AHintManager::PlaceMarker( MARKER a_eMarker, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	TBOOL bNoEffect = TFALSE;
	TINT  iEffect   = -1;

	switch ( a_eMarker )
	{
		case MARKER_GOLDRUSH:
		case MARKER_GATE:
		case MARKER_TOMATO_LAUNCHER_PART:
		case MARKER_EGG:
		case MARKER_JUNKYARD_PART:
		case MARKER_FRUIT:
			iEffect = m_aiEffects[ EFFECT_SPARKLE_WHITE ];
			break;
		case MARKER_MILKWEED:
			iEffect = m_aiEffects[ EFFECT_SPARKLE_GREEN ];
			break;
		case MARKER_SAVE_ZONE:
			iEffect = m_aiEffects[ EFFECT_SAVE_ZONE ];
			break;
		case MARKER_SLEEP_ZONE:
			iEffect = m_aiEffects[ EFFECT_SLEEP_ZONE ];
			break;
		case MARKER_BARN_ENTRANCE:
		case MARKER_KITCHEN_ENTRANCE:
		case MARKER_WALNUT_ENTRANCE:
		case MARKER_MOCKTAIL_ENTRANCE:
		case MARKER_BARN_EXIT:
		case MARKER_GOPHER_ENTRANCE:
			iEffect = m_aiEffects[ EFFECT_DOOR_ZONE ];
			break;
		default:
			bNoEffect = TTRUE;
			break;
	}

	if ( iEffect < 0 )
		bNoEffect = TTRUE;

	if ( m_afMarkerDistanceSq[ a_eMarker ] > 50.0f || bNoEffect )
	{
		StopMarkerEffect( a_eMarker );
		return;
	}

	if ( iEffect != m_aiMarkerEffectIds[ a_eMarker ] )
	{
		m_aiMarkerEffectIds[ a_eMarker ] = iEffect;
		StopMarkerEffect( a_eMarker );
	}

	if ( m_apMarkerEffects[ a_eMarker ] )
	{
		// Move the effect
		TMatrix44& rTransform       = m_apMarkerEffects[ a_eMarker ]->GetInstance()->GetTransform();
		rTransform.GetTranslation() = a_rcPosition;
		rTransform.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );
		rTransform.RotateX( -TMath::HALF_PI );
	}
	else
	{
		m_apMarkerEffects[ a_eMarker ] = AParticleEffectManager::GetSingleton()->PlayEffect( m_aiMarkerEffectIds[ a_eMarker ], a_rcPosition, a_rcDirection );
	}
}

// NOTE: inlined in retail, stops the effect of the marker (AParticleEffectInstance::StopLooping makes it stop emitting)
void AHintManager::StopMarkerEffect( MARKER a_eMarker )
{
	if ( m_apMarkerEffects[ a_eMarker ] )
	{
		m_apMarkerEffects[ a_eMarker ]->GetInstance()->StopLooping();
		m_apMarkerEffects[ a_eMarker ] = TNULL;
	}
}

// $Barnyard: FUNCTION 0041ce90
void AHintManager::BobMarker( MARKER a_eMarker, TFLOAT a_fHeight )
{
	if ( m_apMarkerModels[ a_eMarker ] )
	{
		TVector3 vecPosition = m_avMarkerPositions[ a_eMarker ].AsVector3();
		vecPosition.y += a_fHeight * m_fBob;
		m_apMarkerModels[ a_eMarker ]->GetTransform().SetTranslate( vecPosition );
	}
}

// Finds the closest unit of the type to the player
static AUnit* FindClosestUnitOfType( const TVector4& a_rcPosition, const TPString8& a_rcTypeName )
{
	AUnitManager* pUnitManager = AUnitManager::GetSingleton();
	return pUnitManager->FindClosestUnit( a_rcPosition, pUnitManager->GetUnitFactory()->Find( a_rcTypeName ), 50.0f, TNULL );
}

// $Barnyard: FUNCTION 0041d8e0
void AHintManager::UpdateGoldrushMarker()
{
	if ( AUnit* pStump = FindClosestUnitOfType( m_vPlayerPos, TPS8( coinspawnerstump ) ) )
	{
		TVector4 vecPosition = *pStump->GetSteer()->GetPosition();
		vecPosition.y -= 0.3f;
		PlaceMarker( MARKER_GOLDRUSH, vecPosition, TVector4::VEC_NEGY );
	}
	else
	{
		StopMarkerEffect( MARKER_GOLDRUSH );
	}
}

// $Barnyard: FUNCTION 0041dec0
void AHintManager::UpdateEggMarker()
{
	if ( AUnit* pEgg = FindClosestUnitOfType( m_vPlayerPos, TPS8( egg ) ) )
	{
		TVector4 vecPosition = *pEgg->GetSteer()->GetPosition();
		vecPosition.y -= 0.3f;
		PlaceMarker( MARKER_EGG, vecPosition, TVector4::VEC_NEGY );
	}
	else
	{
		StopMarkerEffect( MARKER_EGG );
	}
}

// Finds the closest unit that is on the screen and is of one of the types (FindUnits within 50 metres)
static AUnit* FindClosestVisibleUnitOfTypes( const TVector4& a_rcPosition, const TPString8** a_ppTypeNames, TINT a_iNumTypes )
{
	AUnit* pClosest       = TNULL;
	TINT   iNumUnits      = 0;
	TFLOAT fClosestDistSq = 99999.992f;

	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( a_rcPosition, 50.0f, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

	for ( TINT i = iNumUnits; i != 0; i-- )
	{
		AUnit*           pUnit     = pResults[ i - 1 ].pUnit;
		const TPString8& rcType    = pUnit->GetType()->GetName();
		TBOOL            bIsOfType = TFALSE;

		for ( TINT k = 0; k < a_iNumTypes; k++ )
		{
			if ( *a_ppTypeNames[ k ] == rcType )
			{
				bIsOfType = TTRUE;
				break;
			}
		}

		if ( !bIsOfType )
			continue;

		const TVector4* pPosition = pUnit->GetSteer()->GetPosition();
		TFLOAT          fZ        = pPosition->z - a_rcPosition.z;
		TFLOAT          fX        = pPosition->x - a_rcPosition.x;

		if ( fZ * fZ + fX * fX < fClosestDistSq )
		{
			TVector2 vecScreen;

			if ( ARenderer::GetSingleton()->GetScreenPosition( *pUnit->GetSteer()->GetPosition(), vecScreen ) )
			{
				fClosestDistSq = fZ * fZ + fX * fX;
				pClosest       = pUnit;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 0041daf0
void AHintManager::UpdateTomatoLauncherMarker()
{
	static const TPString8* s_apParts[] = { &TPS8( tomatolaunch_battery ), &TPS8( tomatolaunch_cog ), &TPS8( tomatolaunch_pogospring ), &TPS8( tomatolaunch_steerwheel ), &TPS8( tomatolaunch_rope ) };

	if ( AUnit* pPart = FindClosestVisibleUnitOfTypes( m_vPlayerPos, s_apParts, TARRAYSIZE( s_apParts ) ) )
	{
		TVector4 vecPosition = *pPart->GetSteer()->GetPosition();
		vecPosition.y -= 0.3f;
		PlaceMarker( MARKER_TOMATO_LAUNCHER_PART, vecPosition, TVector4::VEC_NEGY );
	}
	else
	{
		StopMarkerEffect( MARKER_TOMATO_LAUNCHER_PART );
	}
}

// $Barnyard: FUNCTION 0041dc60
void AHintManager::UpdateJunkyardMarker()
{
	static const TPString8* s_apParts[] = { &TPS8( jp_fan ), &TPS8( jp_gramophone ), &TPS8( jp_umbrella ), &TPS8( jp_wateringcan ), &TPS8( jp_wheel ) };

	if ( AUnit* pPart = FindClosestVisibleUnitOfTypes( m_vPlayerPos, s_apParts, TARRAYSIZE( s_apParts ) ) )
	{
		TVector4 vecPosition = *pPart->GetSteer()->GetPosition();
		vecPosition.y -= 0.3f;

		// NOTE: inlined PlaceMarker in retail
		PlaceMarker( MARKER_JUNKYARD_PART, vecPosition, TVector4::VEC_NEGY );
	}
	else
	{
		StopMarkerEffect( MARKER_JUNKYARD_PART );
	}
}

// $Barnyard: FUNCTION 0041d990
void AHintManager::UpdateFruitMarker()
{
	// The closest rock (ARegrowthManager)
	ARegrowthManager::GrowthObject* pRock = ARegrowthManager::GetSingleton()->FindClosestVisible( m_vPlayerPos, ARegrowthManager::TypeMask::FromName( TPS8( rock ) ), 50.0f );

	if ( pRock )
	{
		TVector4 vecPosition = pRock->vecPosition;
		vecPosition.y -= 0.3f;

		// NOTE: inlined PlaceMarker in retail
		PlaceMarker( MARKER_FRUIT, vecPosition, TVector4::VEC_NEGY );
		return;
	}

	StopMarkerEffect( MARKER_FRUIT );
}

// $Barnyard: FUNCTION 0041df70
void AHintManager::UpdateGateMarker()
{
	// While riding the bike the cow that rides it is the player
	AUnit* pPlayer = ABikeManager::GetSingleton()->IsPlayerOnBike() ? ABikeManager::GetSingleton()->GetRider() : APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pPlayer );

	if ( !pInventory )
		return;

	// The closest closed padlock of the gates the player has a key for
	AGateManager::Gate* pClosest         = TNULL;
	TFLOAT              fClosestDistance = 10000.0f;

	for ( TINT i = TARRAYSIZE( s_apGateKeys ) - 1; i >= 0; i-- )
	{
		AInventoryItem* pItem = pInventory->FindItemByType( *s_apGateKeys[ i ] );

		if ( pItem )
		{
			AUnit* pKeyUnit = pItem->GetUnit();

			if ( pKeyUnit && pKeyUnit->GetType() )
				pClosest = AGateManager::GetSingleton()->FindClosestPadlock( AGateManager::GATETYPE( i ), m_vPlayerPos, fClosestDistance, TTRUE );
		}
	}

	if ( pClosest )
	{
		const TVector3& rcGatePos = pClosest->GetPosition();
		PlaceMarker( MARKER_GATE, TVector4( rcGatePos.x, rcGatePos.y - 0.3f, rcGatePos.z, 1.0f ), TVector4::VEC_NEGY );
		return;
	}

	StopMarkerEffect( MARKER_GATE );
}

// $Barnyard: FUNCTION 0041ee80
void AHintManager::UpdateMilkweedMarker()
{
	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

	if ( !pSimState )
	{
		m_abShowMarker[ MARKER_MILKWEED ] = TFALSE;
		return;
	}

	ALadderBeansGame* pLadderBeansGame = pSimState->GetLadderBeansGame();
	AInteriorNetwork* pNetwork         = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	if ( !pLadderBeansGame || !pNetwork )
		return;

	// The closest patch where nothing is planted yet
	TFLOAT   fClosestDistanceSq = 4000.0f;
	TVector4 vecClosest;

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		if ( pLadderBeansGame->IsPlanted( i ) )
			continue;

		TCHAR szArea[ 16 ];
		T2String8::Format( szArea, "mweed%02d", ALadderBeansGame::GetLadderId( i ) );

		const TINT iArea = pNetwork->FindAreaIndex( TPString8( szArea ) );

		if ( iArea >= 0 )
		{
			TVector4 vecPosition;
			pNetwork->GetArea( iArea )->GetPosition( vecPosition );

			const TFLOAT fDistanceSq = ( vecPosition.z - m_vPlayerPos.z ) * ( vecPosition.z - m_vPlayerPos.z ) + ( vecPosition.x - m_vPlayerPos.x ) * ( vecPosition.x - m_vPlayerPos.x );

			if ( fDistanceSq < fClosestDistanceSq )
			{
				fClosestDistanceSq = fDistanceSq;
				vecClosest         = vecPosition;
			}
		}
	}

	if ( fClosestDistanceSq >= 3500.0f )
	{
		StopMarkerEffect( MARKER_MILKWEED );
	}
	else
	{
		vecClosest.y = ATerrainInterface::GetSingleton()->GetGroundHeightTop( vecClosest.x, vecClosest.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - 0.3f;
		PlaceMarker( MARKER_MILKWEED, vecClosest, TVector4::VEC_NEGY );
	}
}

// $Barnyard: FUNCTION 0041f0a0
void AHintManager::UpdateZoneMarker( MARKER a_eMarker )
{
	m_abShowMarker[ a_eMarker ] = TFALSE;

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	if ( !pNetwork )
		return;

	const TCHAR* szGoal = TNULL;

	switch ( a_eMarker )
	{
		case MARKER_SAVE_ZONE:
		case MARKER_SLEEP_ZONE:
			// Not before the story got going
			if ( AQuestManager::GetSingleton()->GetCurAbsQuestTier() <= 1 )
				return;

			szGoal = ( a_eMarker == MARKER_SAVE_ZONE ) ? "hint_save_zone" : "hint_sleep_zone";
			break;
		case MARKER_BARN_ENTRANCE:
			if ( AGameTimeManager::GetSingleton()->GetCurrentDayPhase() == AGameTimeManager::DAYPHASE_NIGHT )
				return;

			szGoal = "hint_barn_entrance";
			break;
		case MARKER_KITCHEN_ENTRANCE:
			if ( !ASpecialEventRequirement::CanEnterKitchen() )
				return;

			szGoal = "hint_kitchen_entrance";
			break;
		case MARKER_WALNUT_ENTRANCE:
			if ( !AReplayManager::GetSingleton()->IsReplayUnlocked( TPS8( honey_collecting_replay ), TTRUE ) )
				return;

			szGoal = "hint_walnut_entrance";
			break;
		case MARKER_MOCKTAIL_ENTRANCE:
			szGoal = "hint_mocktail_entrance";
			break;
		case MARKER_BARN_EXIT:
			szGoal = "hint_barn_exit";
			break;
		case MARKER_GOPHER_ENTRANCE:
			if ( !AGopherOpenRequirement::IsGopherShopOpen() )
				return;

			szGoal = "hint_gopher_entrance";
			break;
		default:
			return;
	}

	AInteriorGoal* pGoal = pNetwork->FindClosestGoal( szGoal, m_vPlayerPos );

	if ( pGoal && m_apMarkerModels[ a_eMarker ] )
	{
		TVector4 vecGoal;
		pGoal->GetPosition( vecGoal );

		TFLOAT fX                         = m_vPlayerPos.x - vecGoal.x;
		TFLOAT fZ                         = m_vPlayerPos.z - vecGoal.z;
		m_afMarkerDistanceSq[ a_eMarker ] = fZ * fZ + fX * fX;

		TVector4 vecEffect = vecGoal;
		vecEffect.y -= 0.1f;
		PlaceMarker( a_eMarker, vecEffect, TVector4::VEC_NEGY );

		// The icon floats above the goal
		vecGoal.y -= 2.5f;

		if ( !m_avMarkerPositions[ a_eMarker ].IsEqual( vecGoal ) )
		{
			m_avMarkerPositions[ a_eMarker ] = vecGoal;
			m_apMarkerModels[ a_eMarker ]->GetTransform().SetTranslate( vecGoal.AsVector3() );
		}

		m_abShowMarker[ a_eMarker ] = TTRUE;
	}
}

// $Barnyard: FUNCTION 0041f3f0
void AHintManager::UpdateMarkers( TFLOAT a_fDeltaTime )
{
	// Bob and spin the icons
	s_fBobAngle += TMath::HALF_PI * a_fDeltaTime;

	if ( s_fBobAngle > TMath::TWO_PI )
		s_fBobAngle = fmod( s_fBobAngle, TMath::TWO_PI );

	m_fBob = TMath::Abs( TMath::Sin( s_fBobAngle ) );

	TFLOAT fSpin = TMath::PI * a_fDeltaTime;

	for ( MARKER eMarker = MARKER_SAVE_ZONE; eMarker < MARKER_NUMOF; eMarker++ )
	{
		if ( AModelInstance* pModel = m_apMarkerModels[ eMarker ] )
		{
			TMatrix44 mTransform;
			pModel->GetLocalTransform( mTransform );
			mTransform.RotateZ( fSpin );
			pModel->SetTransform( mTransform );
		}
	}

	BobMarker( MARKER_BARN_ENTRANCE, 0.36f );
	BobMarker( MARKER_WALNUT_ENTRANCE, 0.36f );
	BobMarker( MARKER_BARN_EXIT, 0.36f );

	s_fMarkerUpdateTime += a_fDeltaTime;

	if ( s_fMarkerUpdateTime < 1.5f )
		return;

	s_fMarkerUpdateTime = 0.0f;

	// While riding the bike the cow that rides it is the player
	AUnit* pPlayer = ABikeManager::GetSingleton()->IsPlayerOnBike() ? ABikeManager::GetSingleton()->GetRider() : APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	m_abShowMarker[ MARKER_GOLDRUSH ]             = TFALSE;
	m_abShowMarker[ MARKER_MILKWEED ]             = TFALSE;
	m_abShowMarker[ MARKER_GATE ]                 = TFALSE;
	m_abShowMarker[ MARKER_TOMATO_LAUNCHER_PART ] = TFALSE;
	m_abShowMarker[ MARKER_EGG ]                  = TFALSE;
	m_abShowMarker[ MARKER_JUNKYARD_PART ]        = TFALSE;
	m_abShowMarker[ MARKER_FRUIT ]                = TFALSE;

	// The markers of the items of the last assigned quest that has one (4-7)
	for ( TINT iQuest = AQuestManager::GetSingleton()->GetNumAssignedQuests(); iQuest != 0; iQuest-- )
	{
		AQuestInstance* pInstance = AQuestManager::GetSingleton()->GetAssignedQuest( iQuest - 1 );
		TINT            iMarker   = MARKER_NUMOF;
		TBOOL           bFound    = TFALSE;

		while ( iMarker != 0 )
		{
			iMarker--;

			if ( ( iMarker == 4 || iMarker == 5 || iMarker == 6 || iMarker == 7 ) && pInstance->GetQuest()->GetName() == m_astrMarkerNames[ iMarker ] )
			{
				bFound = TTRUE;
				break;
			}
		}

		if ( bFound )
		{
			m_abShowMarker[ iMarker ] = TTRUE;
			break;
		}
	}

	if ( m_abShowMarker[ MARKER_FRUIT ] )
		UpdateFruitMarker();

	if ( m_abShowMarker[ MARKER_TOMATO_LAUNCHER_PART ] )
		UpdateTomatoLauncherMarker();

	if ( m_abShowMarker[ MARKER_EGG ] )
		UpdateEggMarker();

	if ( m_abShowMarker[ MARKER_JUNKYARD_PART ] )
		UpdateJunkyardMarker();

	// The item the player carries
	if ( AUnit* pCarried = TSTATICCAST( AAnimatedUnit, pPlayer )->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) )
	{
		const TPString8& rcType = pCarried->GetType()->GetName();

		if ( rcType == TPS8( goldrush_token ) )
		{
			m_abShowMarker[ MARKER_GOLDRUSH ] = TTRUE;
			UpdateGoldrushMarker();
		}

		if ( rcType == TPS8( milkweed_seed ) )
		{
			m_abShowMarker[ MARKER_MILKWEED ] = TTRUE;
			UpdateMilkweedMarker();
		}

		for ( TINT i = TARRAYSIZE( s_apGateKeys ); i != 0; i-- )
		{
			if ( rcType == *s_apGateKeys[ i - 1 ] )
			{
				m_abShowMarker[ MARKER_GATE ] = TTRUE;
				UpdateGateMarker();
				break;
			}
		}
	}

	// The items in the inventory
	if ( AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pPlayer ) )
	{
		if ( !m_abShowMarker[ MARKER_GATE ] && AGateManager::IsSingletonCreated() )
		{
			for ( TINT i = TARRAYSIZE( s_apGateKeys ); i != 0; i-- )
			{
				if ( pInventory->FindItemByType( *s_apGateKeys[ i - 1 ] ) )
				{
					m_abShowMarker[ MARKER_GATE ] = TTRUE;
					UpdateGateMarker();
					break;
				}
			}
		}

		if ( !m_abShowMarker[ MARKER_GOLDRUSH ] && pInventory->FindItemByType( m_astrMarkerNames[ MARKER_GOLDRUSH ] ) )
		{
			m_abShowMarker[ MARKER_GOLDRUSH ] = TTRUE;
			UpdateGoldrushMarker();
		}

		if ( !m_abShowMarker[ MARKER_MILKWEED ] && pInventory->FindItemByType( m_astrMarkerNames[ MARKER_MILKWEED ] ) )
		{
			m_abShowMarker[ MARKER_MILKWEED ] = TTRUE;
			UpdateMilkweedMarker();
		}
	}

	for ( MARKER eMarker = MARKER_SAVE_ZONE; eMarker < MARKER_NUMOF; eMarker++ )
		UpdateZoneMarker( eMarker );

	// Stop the effects of the hidden markers
	for ( TINT i = MARKER_NUMOF; i != 0; i-- )
	{
		if ( !m_abShowMarker[ i - 1 ] )
			StopMarkerEffect( i - 1 );
	}
}
