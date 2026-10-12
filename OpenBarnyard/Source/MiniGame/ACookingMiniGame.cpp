#include "pch.h"
#include "ACookingMiniGame.h"
#include "ACookingHUD.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Assets/AAssetLoader.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AGameTimeFXManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AHackedQuickSave.h"
#include "GameInterface/ARecipeDisplayState.h"
#include "GameInterface/AStartMenuState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Input/AInputMapManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/cocktails.h"
#include "SoundBank/cooking.h"
#include "SoundBank/music.h"
#include "SoundBank/pigc.h"
#include "SoundBank/ui.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitFactory.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AInventoryItem.h"

#include <Math/TMathInline.h>
#include <Math/TQuaternion.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004fcbc0, CreateObject 004fd840, CreateObjectInPlace 004fd8a0
TDEFINE_CLASS( ACookingMiniGame );

TPSTRING8_DECLARE( ACookingMiniGame );

// Locale strings of AMenuNavHudElement
enum
{
	NAVTEXT_COOK          = 854, // the pan is full (blender)
	NAVTEXT_ADD           = 855,
	NAVTEXT_REMOVE        = 856,
	NAVTEXT_COOK_PAN      = 857, // the pan is full (cooker)
	NAVTEXT_LEAVE         = 858,
	NAVTEXT_RECIPE_BOOK   = 859,
	NAVTEXT_COCKTAIL_BOOK = 860,
};

// cooking.AllIngredients
struct CookingIngredientData
{
	const TCHAR* szName;   // 0x00, the unit type
	TINT         iCentred; // 0x04, 1 if the model is centred on its bounding sphere
	TFLOAT       fScale;   // 0x08
	TINT         iUnk24;   // 0x0c
	TINT         aUnk[ 3 ];
};

TSTATICASSERT( sizeof( CookingIngredientData ) == 0x1c );

// $Barnyard: FUNCTION 004fd560
ACookingMiniGame::ACookingMiniGame()
    : m_iMoney( 0 )
    , m_pHUD( TNULL )
    , m_pCook( TNULL )
    , m_pTray( TNULL )
    , m_iCookAnim( ANIM_IDLE )
    , m_fIdleTime( 0.0f )
    , m_eState( STATE_CHOOSING )
    , m_fTimer( 0.0f )
    , m_pResult( TNULL )
    , m_iResult( -1 )
    , m_iUnk23E0( -1 )
    , m_iCookingCue( -1 )
    , m_iSoundCue( -1 )
    , m_iVoiceCue( -1 )
    , m_bQuestRecipeNotMade( TTRUE )
    , m_bLeaveAfterResult( TFALSE )
    , m_bFailed( TFALSE )
    , m_bExiting( TFALSE )
    , m_bQuestRecipe( TFALSE )
{
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( ACookingMiniGame ) );
}

// $Barnyard: FUNCTION 004fd8e0
// $Barnyard: FUNCTION 004fd730
ACookingMiniGame::~ACookingMiniGame()
{
}

// $Barnyard: FUNCTION 004fcbd0
TBOOL ACookingMiniGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eState == STATE_RESULT )
		return TFALSE;

	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		if ( m_pHUD )
			m_pHUD->SetMouseMoved();

		return AGameState::ProcessInput( a_pInputEvent );
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 || !m_pHUD )
		return AGameState::ProcessInput( a_pInputEvent );

	// Clicks on the arrows scroll the carousel
	TBOOL bStopEvents;

	if ( m_pHUD->IsMouseOnOK() == TTRUE )
		return ProcessCommand( AInputCommand_Shoot, a_pInputEvent, bStopEvents );

	if ( m_pHUD->IsMouseOnLeftArrow() == TTRUE )
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );

	if ( m_pHUD->IsMouseOnRightArrow() == TTRUE )
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );

	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 004fef30
TBOOL ACookingMiniGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_bExiting )
		return TFALSE;

	if ( m_eState != STATE_CHOOSING && m_eState != STATE_RESULT )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( m_eState != STATE_RESULT && m_pHUD->ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents ) )
		return TTRUE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Cancel:
			if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
				return TTRUE;

			// Take the last ingredient out of the pan
			if ( m_eState == STATE_CHOOSING && m_oPan.GetNumIngredients() > 0 )
			{
				ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
				m_eState      = STATE_REMOVING;
				m_fRemoveTime = 0.1f;
				return TTRUE;
			}

			if ( ( m_eState != STATE_CHOOSING || m_oPan.GetNumIngredients() != 0 ) && m_eState != STATE_RESULT )
				return TTRUE;

			{
				// Leave?
				TINT iMessage = IsCocktails() ? 878 : 861;
				ACookingHUD::SetNavHidden( TTRUE );

				ALocaleManager*     pLocale = ALocaleManager::Interface();
				AMessagePopupState* pPopup  = new AMessagePopupState( pLocale->GetString( 1064 ), pLocale->GetString( iMessage ), TPString8(), 11, TFALSE, AInputCommand_Yes, AInputCommand_No, AInputCommand_Empty, AInputCommand_Empty, TFALSE, 60.0f, 0 );
				pPopup->SetHasRectangle2( TTRUE );
				pPopup->SetUnk19b0( TTRUE );
				pPopup->SetUnk19b8( 0.18f );
				m_oPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnLeavePopupResponse, 0 );

				AGameStateController::GetSingleton()->PushState( pPopup );
			}

			return TTRUE;
		case AInputCommand_Quit:
		{
			// The start menu can leave the minigame
			ACookingHUD::SetNavHidden( TTRUE );

			AStartMenuState* pStartMenu = new AStartMenuState();
			pStartMenu->SetHasRectangle2( TTRUE );
			AStartMenuState::SetQuitText( IsCocktails() ? 331 : 330 );
			pStartMenu->SetQuitCallback( OnQuitCallback, this );

			AGameStateController::GetSingleton()->PushState( pStartMenu );
			return TTRUE;
		}
		case AInputCommand_Use:
			// The recipe book (or the cocktail book)
			m_GUIElement.Hide();
			ms_bIgnoreShoot = TTRUE;
			AGameStateController::GetSingleton()->PushState( new ARecipeDisplayState( IsCocktails() ) );
			return TTRUE;
		case AInputCommand_Shoot:
			if ( m_bQuestRecipe )
				return TTRUE;

			if ( ms_bIgnoreShoot )
			{
				ms_bIgnoreShoot = TFALSE;
				return TTRUE;
			}

			if ( m_eState == STATE_RESULT )
			{
				// Put the dish away
				ASoundManager::GetSingleton()->FadeOutCue( m_iVoiceCue, 1.0f, TTRUE, 3.0f );
				m_eState = STATE_HIDING;
				m_fTimer = 0.1f;
				m_pHUD->FadeDishInfo();

				m_pCook->FinishAnimation( m_astrAnims[ m_iCookAnim ] );
				m_iCookAnim = ANIM_PUT_AWAY;
				m_pCook->PlayAnimation( m_astrAnims[ ANIM_PUT_AWAY ] );
				return TTRUE;
			}

			if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || !m_pHUD->GetSelectedIngredient() || m_eState != STATE_CHOOSING )
				return TTRUE;

			// Put the selected ingredient in the pan
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
			AddIngredient( m_pHUD->GetSelectedIngredient() );
			return TTRUE;
		default:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 004fd3d0
void ACookingMiniGame::OnLoaded()
{
	UpdateOwnedIngredients();
	m_pHUD->ResetCarousel();
}

// $Barnyard: FUNCTION 004fe090
TBOOL ACookingMiniGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bExiting )
	{
		// Leave once the pig has said bye
		if ( !ASoundManager::GetSingleton()->IsCueUsed( m_iByeCue ) || m_iByeCue == -1 )
		{
			if ( !IsCocktails() )
			{
				AHackedQuickSave::GetSingleton()->SaveTime();
				AHackedQuickSave::GetSingleton()->SaveProfile();
			}

			AGameStateController::GetSingleton()->SetFadeSound( TTRUE );
			AGameLoader::LeaveMiniGame( TFALSE );
		}

		return TTRUE;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	Dev_Update( a_fDeltaTime );
#endif

	UpdateCook( a_fDeltaTime );
	UpdateRemoving( a_fDeltaTime );
	m_pHUD->Update( a_fDeltaTime );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	switch ( m_eState )
	{
		case STATE_START_CLEAR:
			m_eState     = STATE_CLEARING;
			m_fClearTime = 0.1f;
			break;
		case STATE_CLEARING:
			m_fClearTime -= a_fDeltaTime;

			if ( m_fClearTime > 0.0f )
			{
				m_pHUD->SetPanOpacity( m_fClearTime * 10.0f );
				break;
			}

			if ( m_oPan.GetNumIngredients() > 0 )
			{
				m_pHUD->ClearPan();

				while ( m_oPan.GetNumIngredients() > 0 )
					m_vecFree.PushBack( m_oPan.PopIngredient() );
			}

			ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_LEAVE, TTRUE );
			ACookingHUD::SetNavHidden( TFALSE );
			m_eState = STATE_CHOOSING;
			break;
		case STATE_COOKING:
			// Wait until the pig is quiet
			if ( !pSoundManager->IsCueUsed( m_iCookingCue ) )
			{
				m_eState = STATE_WAITING;
				m_fTimer = 7.0f;

				// NOTE: slop in the kitchen gets the blender too (its lid isn't created for the cooker)
				if ( IsCocktails() || m_bFailed )
				{
					m_iSoundCue = pSoundManager->PlayCue( soundbank::COCKTAILS_MAKE_DRINK );
					m_pHUD->SetBlenderShaking( TTRUE );
					m_pHUD->SetPanOpacity( 0.0f );
					m_pHUD->GetRectangle( ACookingHUD::RECT_BLENDER_LID ).Show();
				}
				else
				{
					m_iSoundCue = pSoundManager->PlayCue( soundbank::COOKING_COOKING );
				}

				AMusicManager* pMusicManager = AMusicManager::GetSingleton();
				pMusicManager->SetForcedSound( soundbank::MUSIC_COOKING );
				pMusicManager->ForceUpdate();
			}

			break;
		case STATE_WAITING:
		{
			m_fTimer -= a_fDeltaTime;

			if ( m_fTimer >= 0.0f )
				break;

			pSoundManager->FadeOutCue( m_iSoundCue, 1.0f, TTRUE, 3.0f );
			AMusicManager::GetSingleton()->ClearForcedSound( TFALSE );

			m_iCookingCue = -1;
			m_iSoundCue   = -1;
			m_fTimer      = 5.0f;
			m_fClearTime  = 0.1f;

			// Show the dish
			m_pHUD->ShowDish( m_pResult->GetObject() );

			if ( m_bFailed )
				m_iVoiceCue = pSoundManager->PlayCue( IsCocktails() ? soundbank::PIGC_PIG_DRINKLOSE : soundbank::PIGC_PIG_COOKLOOSE );
			else
				pSoundManager->PlayCue( soundbank::UI_COIN );

			// The pig presents it
			m_pCook->FinishAnimation( m_astrAnims[ m_iCookAnim ] );
			m_iCookAnim = ANIM_PRESENT;
			m_pCook->PlayAnimation( m_astrAnims[ ANIM_PRESENT ] );

			AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pDish          = m_apDishes[ m_iResult ];

			if ( IsCocktails() )
			{
				TINT iBone = m_pCook->GetModelInstance()->GetBoneID( "cb_rlocator" );
				pObjectManager->AttachObject( pDish, m_pCook, -1, TINT8( iBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				pDish->SetSkeletonUpdating( TTRUE, TTRUE );

				m_pHUD->SetBlenderShaking( TFALSE );
				m_pHUD->GetRectangle( ACookingHUD::RECT_BLENDER_TOP ).Hide();
				m_pHUD->GetRectangle( ACookingHUD::RECT_BLENDER_LID ).Hide();
			}
			else
			{
				// The pig holds the tray, the dish is on it
				TINT iBone = m_pCook->GetModelInstance()->GetBoneID( "cb_rlocator" );
				pObjectManager->AttachObject( m_pTray, m_pCook, -1, TINT8( iBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				m_pTray->SetSkeletonUpdating( TTRUE, TTRUE );

				TINT iFoodBone = m_pTray->GetModelInstance()->GetBoneID( "food_loc" );
				pObjectManager->AttachObject( pDish, m_pTray, -1, TINT8( iFoodBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
				pDish->SetSkeletonUpdating( TTRUE, TTRUE );

				m_pHUD->SetCookerOpen( TTRUE );
				m_pHUD->StartSmoke();
			}

			m_eState = STATE_RESULT;

			if ( m_bQuestRecipe )
				ACookingHUD::SetNavTexts( NAVTEXT_LEAVE, IsCocktails() ? NAVTEXT_COCKTAIL_BOOK : NAVTEXT_RECIPE_BOOK, TFALSE );
			else
				ACookingHUD::SetNavTexts( IsCocktails() ? NAVTEXT_COOK : NAVTEXT_COOK_PAN, NAVTEXT_LEAVE, TTRUE );

			ACookingHUD::SetNavHidden( TFALSE );

			TINT iPrice = m_pResult->GetPrice();
			m_pHUD->ShowDishInfo( ALocaleManager::Interface()->GetString( m_pResult->GetNameString() ), iPrice );
			m_iCookingCue = -1;

			// What the pig thinks of it
			TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

			if ( IsCocktails() )
			{
				if ( m_bFailed )
					break;

				TFLOAT fRandom = rRandom.GetFloat();

				if ( fRandom > 0.6f )
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_DRINKWIN );
				else if ( fRandom <= 0.3f )
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_DRINKUP );
				else
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_DRINKNEW );
			}
			else
			{
				if ( m_bFailed )
					break;

				TFLOAT fRandom = rRandom.GetFloat();

				if ( fRandom > 0.8f )
					m_iVoiceCue = pSoundManager->PlayCue( ASoundId( soundbank::PIGC_PIG_COOK4 + rRandom.GetInt( 4 ) ) );
				else if ( fRandom > 0.6f )
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_COOKUP );
				else if ( fRandom <= 0.4f )
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_EAT_1 );
				else
					m_iVoiceCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_COOKWIN );
			}

			break;
		}
		case STATE_HIDING:
		{
			m_fTimer -= a_fDeltaTime;
			m_pHUD->SetDishOpacity( TMath::Max( m_fTimer, 0.0f ) * 10.0f );

			if ( m_fTimer >= 0.0f )
				break;

			m_pHUD->ClearPan();

			while ( m_oPan.GetNumIngredients() > 0 )
				m_vecFree.PushBack( m_oPan.PopIngredient() );

			m_pHUD->HideDish( m_pResult->GetObject() );
			m_pHUD->SetCookerOpen( TFALSE );

			// The recipe of the quest was made, leave
			if ( m_bLeaveAfterResult )
			{
				AMessagePopupState::ResponseEvent oEvent;
				oEvent.eButton = AInputCommand_Yes;
				OnLeavePopupResponse( this, TNULL, &oEvent );

				return TFALSE;
			}

			ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_LEAVE, TTRUE );
			ACookingHUD::SetNavHidden( TFALSE );

			if ( IsCocktails() )
			{
				m_pHUD->GetRectangle( ACookingHUD::RECT_BLENDER_TOP ).Show();
				m_pHUD->GetRectangle( ACookingHUD::RECT_BLENDER_LID ).Hide();
			}

			m_eState = STATE_CLEANING;
			break;
		}
		case STATE_CLEANING:
		{
			if ( !m_pCook->IsAnimationFinished( m_astrAnims[ ANIM_PUT_AWAY ] ) || m_iResult == -1 )
				break;

			AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
			AAnimatableObject*        pObject;

			if ( IsCocktails() )
			{
				pObjectManager->DetachObject( m_apDishes[ m_iResult ], m_pCook );
				pObject = m_apDishes[ m_iResult ];
			}
			else
			{
				pObjectManager->DetachObject( m_pTray, m_pCook );
				pObjectManager->DetachObject( m_apDishes[ m_iResult ], m_pTray );
				m_apDishes[ m_iResult ]->SetSkeletonUpdating( TFALSE, TTRUE );
				pObject = m_pTray;
			}

			pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			m_iResult = -1;
			m_eState  = STATE_CHOOSING;
			break;
		}
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004ff410
void ACookingMiniGame::OnInsertion()
{
	// The animations of the pig
	ms_pszStrings  = TSTATICCAST( const TCHAR*, AAssetLoader::GetSymbolAddress( "cookingstrings", "StringTable", AAssetType_AssetPack ) );
	ms_iNumStrings = *TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cookingstrings", "NumStrings", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < ms_iNumStrings; i++ )
		m_astrAnims[ i ] = TPString8( ms_pszStrings[ i ] );

	// Cocktails in the barn
	ms_bCocktails = TStringManager::String8Compare( AGameLoader::GetPrevGameStateName().GetString(), "ABarnExploreMinigameState" ) == 0;
	GetHUDParams().SetFlags( AHudElement::MASK_MENU_NAV );

	if ( IsCocktails() )
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_Cocktails, TFALSE, TTRUE, 0, 0, 0, 0 );

		// NOTE: the original sets a copy of the effect (AGameTimeFX copy constructor 00492d30)
		AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
		AGameTimeFX         oFX        = *pFXManager->FindFX( "Cocktail" );
		pFXManager->SetForcedFX( &oFX );
	}
	else
	{
		AGameLoader::Terrain_Set( AGameLoader::Terrain_Kitchen, TFALSE, TTRUE, 0, 0, 0, 0 );
		AGameTimeManager::GetSingleton()->PushUpdateState();
	}

	// No sky and no sun flare
	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	AGameLoader::SpawnSimPlayer();
	LoadRecipes();

	// The player isn't there
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetPlayerUnit( 0 );
	pPlayer->SetAIEnabled( TTRUE );
	pPlayer->SetMovement( TFALSE );
	pPlayer->SetExternalUpdate( TFALSE );
	TSTATICCAST( AVisibleUnit, pPlayer )->SetVisible( TFALSE );

	Load();

	// The camera looks at the pig
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	TMatrix44          matLocator;
	pTerrain->FindLocator( "camera" )->GetMatrix( matLocator );

	TVector4 vecCamera    = matLocator.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TVector4 vecDirection = matLocator.AsBasisVector4( BASISVECTOR_FORWARD );
	pTerrain->GetLocatorPosAndAxis( "camera", vecCamera, vecDirection, 2 );

	ACamera* pCamera                                               = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecCamera;
	pCamera->LookAtDirection( vecDirection );
	pCamera->SetFOV( 0.69813168f );

	AGameStateController::GetSingleton()->SetFlags( AGameStateController::FLAGS_FADING_IN );

	// The pig
	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	m_pCook                                  = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8D( "pig_cook" ) ), TNULL, 0 );
	m_pCook->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pCook->SetVisible( TTRUE );

	if ( IsCocktails() )
	{
		m_pCook->GetModelInstance()->SetTranslation( TVector3( 27.584944f, -0.659678f, 13.827115f ) );
		m_pCook->GetModelInstance()->SetDirection( TVector4( -0.987614f, 0.069201f, -0.140817f, 1.0f ) );
	}
	else
	{
		m_pCook->GetModelInstance()->SetTranslation( TVector3( 1.148403f, -9.5f, -14.5f ) );
		m_pCook->GetModelInstance()->SetDirection( TVector4( -1.0f, 0.0f, 0.0f, 1.0f ) );
	}

	m_pCook->PlayAnimation( m_astrAnims[ ANIM_IDLE ] );

	if ( IsCocktails() )
		m_pCook->SwitchToNamedAnimationSet( TPS8D( "cocktail" ) );

	if ( !IsCocktails() )
	{
		// The tray the dish is served on
		m_pTray = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( TPS8D( "cooking_tray" ) ), TNULL, 0 );
		m_pTray->SetSkeletonUpdating( TFALSE, TTRUE );
		m_pTray->SetVisible( TTRUE );
	}

	// The dishes the pig can show
	TUtil::MemSet( m_apDishes, 0, sizeof( m_apDishes ) );

	for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
	{
		m_apDishes[ i ] = pObjectManager->CreateAnimatableObject( m_oRecipes[ i ].GetObject()->GetObjectType(), TNULL, 0 );
		m_apDishes[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );
	}

	CreateHUD();
	AGameState::OnInsertion();
	m_eState = STATE_CHOOSING;

	// Smaller texts on the bar at the bottom
	AMenuNavHudElement* pMenuNav = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();
	pMenuNav->SetSpacing( 1.3f );
	ms_fOldNavTextScale = pMenuNav->GetTextScale();
	pMenuNav->SetTextScale( 0.65f );

	if ( IsCocktails() )
	{
		pMenuNav->m_abFixedWidth[ 2 ] = TTRUE;
		pMenuNav->m_aTexts[ 2 ].SetWidth( 250.0f );
		pMenuNav->SetY( -20.0f );
	}
}

// $Barnyard: FUNCTION 004fcf30
void ACookingMiniGame::OnRemoval()
{
	m_oTRB.Close();
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	AMenuNavHudElement* pMenuNav = AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>();
	pMenuNav->SetSpacing( 1.0f );
	pMenuNav->SetTextScale( ms_fOldNavTextScale );

	if ( IsCocktails() )
	{
		pMenuNav->m_abFixedWidth[ 2 ] = TFALSE;
		pMenuNav->SetY( 0.0f );
	}

	ACookingHUD::SetNavTexts( 1047, 1048, TFALSE );

	if ( !IsCocktails() )
		AGameTimeManager::GetSingleton()->PopUpdateState();

	if ( m_pHUD )
		delete m_pHUD;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	pObjectManager->DestroyAnimatableObject( m_pCook );
	pObjectManager->DestroyAnimatableObject( m_pTray );

	for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
	{
		m_oRecipes[ i ].Destroy();
		pObjectManager->DestroyAnimatableObject( m_apDishes[ i ] );
		m_apDishes[ i ] = TNULL;
	}

	for ( TINT i = 0; i < m_oIngredients.Size(); i++ )
		m_oIngredients[ i ].Destroy();

	// Every copy of the ingredients is deleted
	while ( m_oPan.GetNumIngredients() > 0 )
		m_vecFree.PushBack( m_oPan.PopIngredient() );

	for ( TINT i = 0; i < m_vecFree.Size(); i++ )
	{
		m_vecFree[ i ]->Destroy();

		if ( m_vecFree[ i ] )
			delete m_vecFree[ i ];
	}

	m_vecFree.Clear();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004fd380
void ACookingMiniGame::OnActivate()
{
	AGameState::OnActivate();
	m_GUIElement.Show();
	ACookingHUD::SetNavHidden( TFALSE );
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );
}

// $Barnyard: FUNCTION 004fd3b0
void ACookingMiniGame::OnDeactivate()
{
	AGameState::OnDeactivate();
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
}

// $Barnyard: FUNCTION 004fe9d0
void ACookingMiniGame::LoadRecipes()
{
	AAssetLoader::GetSymbolAddress( "cooking", "MaxIngredients", AAssetType_AssetPack );
	TINT*                  pNumIngredients = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfIngredients", AAssetType_AssetPack ) );
	CookingIngredientData* pIngredients    = TSTATICCAST( CookingIngredientData, AAssetLoader::GetSymbolAddress( "cooking", "AllIngredients", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < *pNumIngredients; i++ )
	{
		ACookingIngredient oIngredient;
		oIngredient.Create( pIngredients[ i ].szName, pIngredients[ i ].fScale, pIngredients[ i ].iCentred == 1 );
		oIngredient.SetUnk24( pIngredients[ i ].iUnk24 );
		AUnitManager::GetSingleton()->GetUnitFactory()->Find( oIngredient.GetName() );

		m_oIngredients.PushBack( oIngredient );

		// Up to three of each can be in the pan
		m_vecFree.PushBack( oIngredient.Clone() );
		m_vecFree.PushBack( oIngredient.Clone() );
		m_vecFree.PushBack( oIngredient.Clone() );
	}

	TINT*            pNumRecipes = TSTATICCAST( TINT, AAssetLoader::GetSymbolAddress( "cooking", "NumberOfRecipes", AAssetType_AssetPack ) );
	RecipeEntryData* pRecipes    = TSTATICCAST( RecipeEntryData, AAssetLoader::GetSymbolAddress( "cooking", "AllRecipes", AAssetType_AssetPack ) );

	for ( TINT i = 0; i < *pNumRecipes; i++ )
	{
		ACookingRecipe oRecipe;
		oRecipe.Create( pRecipes[ i ].iNameString, pRecipes[ i ].szName );
		oRecipe.SetPrice( pRecipes[ i ].iPrice );
		oRecipe.SetQuest( pRecipes[ i ].szQuest );
		oRecipe.SetType( pRecipes[ i ].iType );

		for ( TINT k = 0; k < ACookingRecipe::MAX_INGREDIENTS; k++ )
		{
			if ( !pRecipes[ i ].aszIngredients[ k ] )
				break;

			oRecipe.AddIngredient( FindIngredient( pRecipes[ i ].aszIngredients[ k ] ) );
		}

		m_oRecipes.PushBack( oRecipe );
	}

	// What anything else makes
	ACookingRecipe oFailed;

	if ( IsCocktails() )
	{
		oFailed.Create( 852, "swill" );
		oFailed.SetType( ACookingRecipe::TYPE_COCKTAIL );
		oFailed.SetPrice( 1 );
	}
	else
	{
		oFailed.Create( 852, "slop" );
		oFailed.SetType( ACookingRecipe::TYPE_DISH );
		oFailed.SetPrice( 1 );
	}

	m_oRecipes.PushBack( oFailed );
}

// $Barnyard: FUNCTION 004fcd30
void ACookingMiniGame::CreateHUD()
{
	m_GUIElement.Show();

	m_pHUD = new ACookingHUD();
	m_pHUD->Create( &m_GUIElement );

	ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_LEAVE, TTRUE );
	ACookingHUD::SetNavHidden( TFALSE );

	ASoundManager::GetSingleton()->PlayCue( soundbank::PIGC_PIG_HELLO1 );
}

// $Barnyard: FUNCTION 004fcc60
void ACookingMiniGame::UpdateOwnedIngredients()
{
	TUtil::MemSet( m_apOwned, 0, sizeof( m_apOwned ) );

	AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	ACookingIngredient** ppOwned = m_apOwned;

	// NOTE: the original doesn't check MAX_OWNED (nor ACookingHUD::MAX_INGREDIENTS)
	for ( TINT i = 0; i < m_oIngredients.Size(); i++ )
	{
		ACookingIngredient& rIngredient = m_oIngredients[ i ];
		AInventoryItem*     pItem       = pInventory->FindItemByType( rIngredient.GetName() );

		if ( pItem )
		{
			*ppOwned = &rIngredient;
			( *ppOwned )->SetCount( pItem->GetCount() );
			( *ppOwned )->SetScreenName( pItem->GetScreenName() );
			m_pHUD->AddIngredient( &rIngredient );

			ppOwned++;
		}
	}
}

// $Barnyard: FUNCTION 004fcde0
void ACookingMiniGame::UpdateCook( TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_CHOOSING )
	{
		if ( m_iCookAnim == ANIM_PRESENT )
		{
			if ( m_pCook->IsAnimationFinished( m_astrAnims[ ANIM_PRESENT ] ) )
			{
				m_iCookAnim = ANIM_WAIT;
				m_pCook->PlayAnimation( m_astrAnims[ ANIM_WAIT ] );
			}
		}
		else if ( m_iCookAnim == ANIM_START_COOK )
		{
			if ( m_pCook->IsAnimationFinished( m_astrAnims[ ANIM_START_COOK ] ) )
			{
				m_iCookAnim = ANIM_COOK;
				m_pCook->PlayAnimation( m_astrAnims[ ANIM_COOK ] );
			}
		}

		return;
	}

	// Now and then another idle animation
	m_fIdleTime += a_fDeltaTime;

	if ( m_fIdleTime > 5.0f )
	{
		m_fIdleTime = 0.0f;

		TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

		if ( m_iCookAnim == ANIM_IDLE && rRandom.GetFloat() > 0.5f )
		{
			m_iCookAnim = rRandom.GetInt( 3 );
			m_pCook->PlayAnimation( m_astrAnims[ m_iCookAnim ] );
		}
	}

	if ( m_pCook->IsAnimationFinished( m_astrAnims[ m_iCookAnim ] ) == TTRUE )
	{
		m_iCookAnim = ANIM_IDLE;
		m_pCook->PlayAnimation( m_astrAnims[ ANIM_IDLE ] );
	}
}

// $Barnyard: FUNCTION 004fd300
void ACookingMiniGame::UpdateRemoving( TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_REMOVING )
		return;

	TINT iNumIngredients = m_oPan.GetNumIngredients();

	if ( iNumIngredients <= 0 )
		return;

	ACookingIngredient* pIngredient = m_oPan.GetIngredient( iNumIngredients - 1 );
	pIngredient->GetObject()->SetOpacity( m_fRemoveTime * 10.0f, TTRUE );

	m_fRemoveTime -= a_fDeltaTime;

	if ( m_fRemoveTime < 0.0f )
	{
		RemoveLastIngredient();
		m_eState = STATE_CHOOSING;
	}
}

// $Barnyard: FUNCTION 004fd160
TINT ACookingMiniGame::FindOwned( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < MAX_OWNED; i++ )
	{
		if ( !m_apOwned[ i ] )
			return -1;

		if ( m_apOwned[ i ]->GetName().GetPooledString() == a_rcName.GetPooledString() )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 004fd1a0
ACookingIngredient* ACookingMiniGame::FindIngredient( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_oIngredients.Size(); i++ )
	{
		if ( m_oIngredients[ i ].GetName().GetString8().Compare( a_szName ) == 0 )
			return &m_oIngredients[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004feda0
void ACookingMiniGame::AddIngredient( ACookingIngredient* a_pIngredient )
{
	if ( !a_pIngredient )
		return;

	if ( m_oPan.GetNumIngredients() >= ACookingRecipe::MAX_INGREDIENTS || a_pIngredient->GetCount() <= 0 )
	{
		// The pan is full, cook
		if ( m_oPan.GetNumIngredients() == ACookingRecipe::MAX_INGREDIENTS )
			Cook();

		return;
	}

	a_pIngredient->SetCount( a_pIngredient->GetCount() - 1 );
	m_pHUD->SetIngredientText( a_pIngredient->GetScreenName(), a_pIngredient->GetCount() );

	// A free copy of the ingredient goes into the pan
	ACookingIngredient* pCopy = TNULL;
	auto                it    = m_vecFree.Begin();

	for ( ; it != m_vecFree.End(); it++ )
	{
		if ( ( *it )->GetName().GetPooledString() == a_pIngredient->GetName().GetPooledString() )
		{
			pCopy = *it;
			break;
		}
	}

	// NOTE: when there's no free copy, the original removes the end (the last element of m_vecFree)
	m_vecFree.Erase( it );

	m_oPan.AddIngredient( pCopy );
	m_pHUD->AddToPan( pCopy );

	if ( m_oPan.GetNumIngredients() == ACookingRecipe::MAX_INGREDIENTS )
		ACookingHUD::SetNavTexts( IsCocktails() ? NAVTEXT_COOK : NAVTEXT_COOK_PAN, NAVTEXT_REMOVE, TTRUE );
	else
		ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_REMOVE, TTRUE );

	ACookingHUD::SetNavHidden( TFALSE );
}

// $Barnyard: FUNCTION 004fd210
void ACookingMiniGame::RemoveLastIngredient()
{
	if ( m_oPan.GetNumIngredients() <= 0 )
		return;

	ACookingIngredient* pCopy = m_oPan.PopIngredient();
	m_pHUD->RemoveLastFromPan();

	// Give it back
	ACookingIngredient* pOwned = m_apOwned[ FindOwned( pCopy->GetName() ) ];
	pOwned->IncrementCount();

	if ( m_pHUD->GetSelectedIngredient()->GetName().GetPooledString() == pOwned->GetName().GetPooledString() )
		m_pHUD->SetIngredientText( pOwned->GetScreenName(), pOwned->GetCount() );

	m_vecFree.PushBack( pCopy );

	if ( m_oPan.GetNumIngredients() != 0 )
		ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_REMOVE, TTRUE );
	else
		ACookingHUD::SetNavTexts( NAVTEXT_ADD, NAVTEXT_LEAVE, TTRUE );

	ACookingHUD::SetNavHidden( TFALSE );
}

// $Barnyard: FUNCTION 004fdbb0
void ACookingMiniGame::Cook()
{
	TINT iAct  = 0;
	TINT iTier = 0;

	m_pResult = TNULL;
	m_iResult = -1;

	// The recipe of the ingredients (of the right type)
	for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
	{
		ACookingRecipe& rRecipe = m_oRecipes[ i ];

		if ( rRecipe.HasSameIngredients( m_oPan ) == TTRUE )
		{
			TUINT uiType = IsCocktails() ? ( TUINT( rRecipe.GetType() ) >> 1 ) : TUINT( rRecipe.GetType() );

			if ( uiType & 1 )
			{
				m_pResult = &rRecipe;
				m_iResult = i;
				break;
			}
		}
	}

	m_fTimer  = 1.0f;
	m_bFailed = TFALSE;

	if ( m_pResult )
	{
		m_pHUD->SetResultText( TTRUE );
	}
	else
	{
		// Slop (or swill), the recipe without ingredients
		m_bFailed = TTRUE;

		for ( TINT i = 0; i < m_oRecipes.Size(); i++ )
		{
			if ( m_oRecipes[ i ].GetNumIngredients() == 0 )
			{
				m_pResult = &m_oRecipes[ i ];
				m_iResult = i;
				break;
			}
		}

		m_pHUD->SetResultText( TFALSE );
	}

	// The recipe of the current quest
	if ( !m_pResult->GetQuest().IsEmpty() && !m_bFailed )
	{
		AQuestManager* pQuestManager = AQuestManager::GetSingleton();
		pQuestManager->FindQuestTier( m_pResult->GetQuest().GetString(), &iAct, &iTier );
		TINT iAbsTier = pQuestManager->GetAbsQuestTier( iAct, iTier );

		if ( m_bQuestRecipeNotMade && iAbsTier == pQuestManager->GetCurAbsQuestTier() )
		{
			m_bQuestRecipeNotMade = TFALSE;
			m_bQuestRecipe        = TTRUE;
			m_bLeaveAfterResult   = TTRUE;
		}
	}

	m_fTimer = 5.0f;

	if ( !m_bFailed )
	{
		// Count it
		TINT i = 0;

		for ( ; i < m_oMade.Size(); i++ )
		{
			if ( m_pResult->GetNameString() == m_oMade[ i ].GetNameString() )
				break;
		}

		if ( i < m_oMade.Size() )
		{
			ACookingRecipe& rMade = m_oMade[ i ];
			rMade.SetNumTotal( rMade.GetNumTotal() + 1 );
			rMade.SetNumMade( rMade.GetNumMade() + 1 );
		}
		else
		{
			TINT iIndex = m_oMade.PushBack( *m_pResult );
			m_oMade[ iIndex ].SetNumMade( 1 );
			m_oMade[ iIndex ].SetNumTotal( 1 );
		}
	}

	if ( !m_bFailed )
		m_iMoney += m_pResult->GetPrice();

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( IsCocktails() )
	{
		if ( ARandom::GetSingleton()->m_oRandom.GetFloat() <= 0.5f )
			m_iCookingCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_DRINKNEW );
		else
			m_iCookingCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_DRINKS1 );
	}
	else
	{
		m_iCookingCue = pSoundManager->PlayCue( soundbank::PIGC_PIG_COOK1 );
	}

	m_pCook->FinishAnimation( m_astrAnims[ m_iCookAnim ] );
	pSoundManager->FadeOutCue( pSoundManager->FindPlayingCue( soundbank::PIGC_PIG_EAT_1 ), 1.0f, TTRUE, 3.0f );

	m_eState = STATE_COOKING;
	ACookingHUD::SetNavHidden( TTRUE );

	m_iCookAnim = ANIM_START_COOK;
	m_pCook->PlayAnimation( m_astrAnims[ ANIM_START_COOK ] );
}

// $Barnyard: FUNCTION 004fd900
void ACookingMiniGame::Finish()
{
	AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	// Take the used ingredients
	for ( TINT i = 0; i < MAX_OWNED; i++ )
	{
		ACookingIngredient* pOwned = m_apOwned[ i ];

		if ( !pOwned )
			break;

		AInventoryItem* pItem = pInventory->FindItemByType( pOwned->GetName() );

		if ( pOwned->GetCount() != 0 )
			pItem->SetCount( pOwned->GetCount() );
		else
			pInventory->RemoveItem( pItem );
	}

	// Give the dishes of the current quest
	for ( TINT i = 0; i < m_oMade.Size(); i++ )
	{
		ACookingRecipe& rMade = m_oMade[ i ];

		if ( rMade.GetQuest().IsEmpty() )
			continue;

		TINT           iAct          = 0;
		TINT           iTier         = 0;
		AQuestManager* pQuestManager = AQuestManager::GetSingleton();
		pQuestManager->FindQuestTier( rMade.GetQuest().GetString(), &iAct, &iTier );

		if ( pQuestManager->GetAbsQuestTier( iAct, iTier ) != pQuestManager->GetCurAbsQuestTier() )
			continue;

		AInventoryItem* pItem = pInventory->FindItemByType( rMade.GetName() );

		// NOTE: a dish the player didn't have is given once, however many were made
		if ( pItem )
			pItem->AddCount( rMade.GetNumMade() );
		else
			pInventory->AddItems( rMade.GetName(), 1, AInventoryComponentInterface::ms_strDefaultInventory );
	}

	// The bucks are counted in the sim
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( m_iMoney );
	Save();
}

// $Barnyard: FUNCTION 004fd4e0
void ACookingMiniGame::Save()
{
	AHackedQuickSave::Buffer* pBuffer = AHackedQuickSave::GetSingleton()->GetBuffer( AHackedQuickSave::BUFFER_COOKING );
	pBuffer->SetCursor( 0 );

	pBuffer->WriteBool( m_bQuestRecipeNotMade );
	pBuffer->WriteInt( m_oMade.Size() );

	for ( TINT i = 0; i < m_oMade.Size(); i++ )
	{
		pBuffer->WriteInt( m_oMade[ i ].GetNameString() );
		pBuffer->WriteUInt8( TUINT8( m_oMade[ i ].GetType() ) );
		pBuffer->WriteUInt8( m_oMade[ i ].GetNumTotal() );
	}

	pBuffer->SetSaved( TTRUE );
}

// $Barnyard: FUNCTION 004fd3f0
void ACookingMiniGame::Load()
{
	AHackedQuickSave::Buffer* pBuffer = AHackedQuickSave::GetSingleton()->GetBuffer( AHackedQuickSave::BUFFER_COOKING );

	if ( !pBuffer->IsSaved() )
		return;

	pBuffer->SetCursor( 0 );
	pBuffer->ReadBool( m_bQuestRecipeNotMade );

	TINT iNumMade = 0;
	pBuffer->ReadInt( iNumMade );

	for ( TINT i = 0; i < iNumMade; i++ )
	{
		TUINT  uiNameString;
		TUINT8 uiType;
		TUINT8 uiNumTotal;
		pBuffer->ReadUInt( uiNameString );
		pBuffer->ReadUInt8( uiType );
		pBuffer->ReadUInt8( uiNumTotal );

		for ( TINT k = 0; k < m_oRecipes.Size(); k++ )
		{
			ACookingRecipe& rRecipe = m_oRecipes[ k ];

			if ( rRecipe.GetNameString() == TINT( uiNameString ) )
			{
				// NOTE: the copy doesn't get the total (operator= doesn't copy it)
				rRecipe.SetNumTotal( uiNumTotal );
				m_oMade.PushBack( rRecipe );
				break;
			}
		}
	}

	pBuffer->SetSaved( TFALSE );
}

// $Barnyard: FUNCTION 004fdf80
TBOOL __stdcall ACookingMiniGame::OnQuitCallback( AStartMenuState* a_pStartMenu, void* a_pUserData, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	a_pPopup->Remove();

	if ( a_pEvent->eButton != AInputCommand_Yes )
		return TTRUE;

	// Yes, leave
	a_pStartMenu->Remove();

	ACookingMiniGame* pGame = TSTATICCAST( ACookingMiniGame, a_pUserData );

	while ( pGame->m_oPan.GetNumIngredients() > 0 )
		pGame->RemoveLastIngredient();

	pGame->Finish();

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	pSoundManager->StopCue( pGame->m_iVoiceCue );
	pGame->m_iVoiceCue = -1;
	pGame->m_iByeCue   = pSoundManager->PlayCue( soundbank::PIGC_PIG_BYE1 );
	pGame->m_bExiting  = TTRUE;

	return TTRUE;
}

// $Barnyard: FUNCTION 004fe010
TBOOL __stdcall ACookingMiniGame::OnLeavePopupResponse( ACookingMiniGame* a_pGame, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	if ( a_pPopup )
		a_pPopup->Remove();

	if ( a_pEvent->eButton == AInputCommand_Yes )
	{
		// Leave
		a_pGame->Finish();

		ASoundManager* pSoundManager = ASoundManager::GetSingleton();
		pSoundManager->StopCue( a_pGame->m_iVoiceCue );
		a_pGame->m_iVoiceCue = -1;
		a_pGame->m_iByeCue   = pSoundManager->PlayCue( soundbank::PIGC_PIG_BYE1 );
		a_pGame->m_bExiting  = TTRUE;
	}
	else
	{
		ACookingHUD::SetNavHidden( TFALSE );
		ms_bIgnoreShoot = TTRUE;
	}

	return TFALSE;
}

#ifdef BARNYARD_COMMUNITY_PATCH

#  include <Input/TInputDeviceMouse.h>

// Development: "-devautoplay" runs. Puts the ingredients of the first recipe the
// player has them for in the pan one by one (anything the player has if there's none), cooks, shows
// the dish, then leaves through the popup.
void ACookingMiniGame::Dev_Update( TFLOAT a_fDeltaTime )
{
	static TINT s_iEnabled = -1;

	if ( s_iEnabled == -1 )
		s_iEnabled = TStringManager::String8FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;

	if ( !s_iEnabled )
		return;

	static TFLOAT            s_fTime  = 0.0f;
	static TINT              s_iPhase = 0;
	static TINT              s_iAdded = 0;
	static TINT              s_aiOwned[ ACookingRecipe::MAX_INGREDIENTS ];
	static ACookingMiniGame* s_pGame = TNULL;

	// Every visit of the kitchen starts over
	if ( s_pGame != this )
	{
		s_pGame  = this;
		s_fTime  = 0.0f;
		s_iPhase = 0;
		s_iAdded = 0;
	}

	s_fTime += a_fDeltaTime;

	TInputInterface::InputEvent oEvent( TNULL, TInputDeviceMouse::BUTTON_1, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TBOOL                       bStopEvents;

	if ( s_iPhase == 0 && s_fTime > 4.0f && m_eState == STATE_CHOOSING )
	{
		s_iPhase = 1;
		s_fTime  = 0.0f;

		TINT iNumOwned = 0;
		while ( iNumOwned < MAX_OWNED && m_apOwned[ iNumOwned ] ) iNumOwned++;

		TINFO( "[devautoplay] cooking: %d ingredients owned, %d recipes, cocktails %d\n", iNumOwned, m_oRecipes.Size(), IsCocktails() );

		// The first recipe of the right type the player has the ingredients of
		TINT iRecipe = -1;

		for ( TINT i = 0; i < m_oRecipes.Size() && iRecipe == -1; i++ )
		{
			ACookingRecipe& rRecipe = m_oRecipes[ i ];
			TUINT           uiType  = IsCocktails() ? ( TUINT( rRecipe.GetType() ) >> 1 ) : TUINT( rRecipe.GetType() );

			if ( !( uiType & 1 ) || rRecipe.GetNumIngredients() != ACookingRecipe::MAX_INGREDIENTS )
				continue;

			TBOOL bHasAll = TTRUE;

			for ( TINT k = 0; k < ACookingRecipe::MAX_INGREDIENTS; k++ )
			{
				TINT iOwned    = FindOwned( rRecipe.GetIngredient( k )->GetName() );
				s_aiOwned[ k ] = iOwned;

				// As many as the recipe uses of the ingredient
				TINT iNeeded = 0;

				for ( TINT j = 0; j < ACookingRecipe::MAX_INGREDIENTS; j++ )
				{
					if ( rRecipe.GetIngredient( j )->GetName() == rRecipe.GetIngredient( k )->GetName() )
						iNeeded++;
				}

				if ( iOwned == -1 || m_apOwned[ iOwned ]->GetCount() < iNeeded )
					bHasAll = TFALSE;
			}

			if ( bHasAll )
				iRecipe = i;
		}

		if ( iRecipe != -1 )
		{
			TINFO( "[devautoplay] cooking recipe %d (string %d)\n", iRecipe, m_oRecipes[ iRecipe ].GetNameString() );
		}
		else
		{
			TINFO( "[devautoplay] no recipe, cooking the first ingredient three times\n" );

			for ( TINT k = 0; k < ACookingRecipe::MAX_INGREDIENTS; k++ )
				s_aiOwned[ k ] = ( iNumOwned > 0 ) ? 0 : -1;
		}

		fflush( stdout );
	}
	else if ( s_iPhase == 1 && s_fTime > 1.5f && m_eState == STATE_CHOOSING )
	{
		s_fTime = 0.0f;

		if ( s_iAdded < ACookingRecipe::MAX_INGREDIENTS )
		{
			TINT iOwned = s_aiOwned[ s_iAdded++ ];

			if ( iOwned != -1 )
			{
				AddIngredient( m_apOwned[ iOwned ] );
				TINFO( "[devautoplay] added %s, %d in the pan\n", m_apOwned[ iOwned ]->GetName().GetString(), m_oPan.GetNumIngredients() );
			}
		}
		else
		{
			// The pan is full, Shoot cooks
			s_iPhase = 2;
			ProcessCommand( AInputCommand_Shoot, &oEvent, bStopEvents );
			TINFO( "[devautoplay] cooking, state %d\n", m_eState );
		}

		fflush( stdout );
	}
	else if ( s_iPhase == 2 && m_eState == STATE_RESULT )
	{
		s_iPhase = 3;
		s_fTime  = 0.0f;
		TINFO( "[devautoplay] made %d (failed %d), price %d, money %d\n", m_iResult, m_bFailed, m_pResult->GetPrice(), m_iMoney );
		fflush( stdout );
	}
	else if ( s_iPhase == 3 && s_fTime > 6.0f && m_bQuestRecipe )
	{
		// The recipe of the quest: the dish can't be put away, Leave (Cancel) and Yes
		s_iPhase = 5;
		s_fTime  = 0.0f;
		TINFO( "[devautoplay] the quest's recipe, leaving\n" );
		fflush( stdout );

		ProcessCommand( AInputCommand_Cancel, &oEvent, bStopEvents );

		AMessagePopupState* pPopup = TDYNAMICCAST( AMessagePopupState, AGameStateController::GetSingleton()->GetCurrentState() );

		if ( pPopup )
		{
			TINFO( "[devautoplay] yes\n" );
			pPopup->Dev_ChooseButton( 0 );
		}
	}
	else if ( s_iPhase == 3 && s_fTime > 6.0f )
	{
		s_iPhase = 4;
		s_fTime  = 0.0f;
		TINFO( "[devautoplay] putting the dish away\n" );
		fflush( stdout );

		ProcessCommand( AInputCommand_Shoot, &oEvent, bStopEvents );
	}
	else if ( s_iPhase == 4 && s_fTime > 4.0f && m_eState == STATE_CHOOSING )
	{
		s_iPhase = 5;
		s_fTime  = 0.0f;
		TINFO( "[devautoplay] leaving\n" );
		fflush( stdout );

		ProcessCommand( AInputCommand_Cancel, &oEvent, bStopEvents );

		// This state isn't updated while the popup is on top of it, choose Yes now
		AMessagePopupState* pPopup = TDYNAMICCAST( AMessagePopupState, AGameStateController::GetSingleton()->GetCurrentState() );

		if ( pPopup )
		{
			TINFO( "[devautoplay] yes\n" );
			pPopup->Dev_ChooseButton( 0 );
		}

		fflush( stdout );
	}
}

#endif
