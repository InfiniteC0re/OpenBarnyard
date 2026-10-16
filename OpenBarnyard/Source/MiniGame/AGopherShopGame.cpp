#include "pch.h"
#include "AGopherShopGame.h"
#include "AGopherShopMenu.h"
#include "AGopherShopInterface.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AGameTimeFXManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AStartMenuState.h"
#include "GameInterface/AMessagePopupState.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2FadeRectangle.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/ABikeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/AMenuNavHudElement.h"
#include "Input/AInputMapManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ASunFlareEffect.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AInventoryItem.h"

#include <File/TTRB.h>
#include <Toshi/TStringManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Math/TQuaternion.h>
#include "SoundBank/gophers.h"
#include "SoundBank/music.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004927d0, CreateObject 004956c0, CreateObjectInPlace 00495720
TDEFINE_CLASS( AGopherShopGame );

TPSTRING8_DECLARE( AGopherShopGame );
TPSTRING8_DECLARE( gopher_gs_anims );
TPSTRING8_DECLARE( gopher_talk );
TPSTRING8_DECLARE( gopher_idle );
TPSTRING8_DECLARE( gopher_callout );
TPSTRING8_DECLARE( honeycomb );

// The FOV of the camera before the shop (DAT_00782bd0)
static TFLOAT s_fOldFOV;

// The D-pad is used to scroll (the stick is ignored meanwhile) (DAT_00782c04)
static TBOOL s_bUsingDPad;

// Purchases of these items don't get the discount
static TBOOL HasNoDiscount( AGopherShopItem* a_pItem, TINT a_iPrice )
{
	return a_pItem->GetId() == 896 || a_pItem->GetId() == 313 || a_iPrice == 1;
}

// $Barnyard: FUNCTION 004946d0
AGopherShopGame::AGopherShopGame()
    : m_fInputTime( 0.0f )
    , m_eMenu( MENU_MAIN )
    , m_eMode( MODE_BUY )
    , m_iNumPurchases( 0 )
    , m_iNumOwned( 0 )
    , m_iUnk1164( 0 )
    , m_iCategory( 0 )
    , m_pMenu( TNULL )
    , m_bFirstVisit( TTRUE )
    , m_bBought( TFALSE )
    , m_iCue( -1 )
    , m_iIdleCue( -1 )
    , m_fTalkTime( 0.0f )
    , m_fTime( 0.0f )
    , m_bExiting( TFALSE )
    , m_bMouseOnLeftArrow( TFALSE )
    , m_bMouseOnRightArrow( TFALSE )
    , m_bMouseOnModel( TFALSE )
    , m_uiJoeyOffer( 0xff )
    , m_bDiscount( TFALSE )
{
	// NOTE: m_pMenu isn't initialised by retail
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AGopherShopGame ) );
}

// $Barnyard: FUNCTION 00493bb0
// $Barnyard: FUNCTION 004947f0
AGopherShopGame::~AGopherShopGame()
{
	m_oListener.Disconnect();
}

// $Barnyard: FUNCTION 004927e0
TBOOL AGopherShopGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_pMenu->ProcessInputEvent( a_pInputEvent ) )
		return TTRUE;

	if ( m_eMenu != MENU_ITEM || a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 )
		return AGameState::ProcessInput( a_pInputEvent );

	// Clicks on the model buy it, clicks on the arrows scroll
	TBOOL bStopEvents;

	if ( m_bMouseOnModel == TTRUE )
	{
		m_bMouseOnModel = TFALSE;
		return ProcessCommand( AInputCommand_Shoot, a_pInputEvent, bStopEvents );
	}
	else if ( m_bMouseOnLeftArrow == TTRUE )
	{
		m_bMouseOnLeftArrow = TFALSE;
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );
	}
	else if ( m_bMouseOnRightArrow == TTRUE )
	{
		m_bMouseOnRightArrow = TFALSE;
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00495760
TBOOL AGopherShopGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_pMenu && m_pMenu->IsTransitioning() )
		return TTRUE;

	if ( m_bExiting == TTRUE )
		return TFALSE;

	if ( a_eInputCommand == AInputCommand_Quit )
	{
		// The start menu can leave the shop
		AStartMenuState* pStartMenu = new AStartMenuState();
		pStartMenu->SetHasRectangle2( TTRUE );
		AStartMenuState::SetQuitText( 332 );
		pStartMenu->SetQuitCallback( OnQuitCallback, this );
		AGameStateController::GetSingleton()->PushState( pStartMenu );

		return TTRUE;
	}

	if ( m_eMenu >= MENU_MAIN )
	{
		if ( m_eMenu > MENU_ITEM )
		{
			s_bUsingDPad = TFALSE;
			goto PROCESS_COMMAND;
		}

		if ( m_eMenu != MENU_ITEM || m_pMenu->IsConfirmShown() )
		{
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
			{
				m_pMenu->ProcessCommand( a_eInputCommand, a_pInputEvent );
				return TTRUE;
			}
		}
	}

	if ( m_eMenu == MENU_ITEM )
	{
		// The D-pad
		TBOOL bDPad = TFALSE;

		switch ( a_eInputCommand )
		{
			case AInputCommand_LookUpWithUp:
				if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				{
					m_vecStick.y = 0.0f;
				}
				else
				{
					bDPad        = TTRUE;
					m_vecStick.y = -1.0f;
					s_bUsingDPad = TTRUE;
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				}
				break;
			case AInputCommand_LookDownWithUp:
				if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				{
					m_vecStick.y = 0.0f;
				}
				else
				{
					bDPad        = TTRUE;
					s_bUsingDPad = TTRUE;
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
					m_vecStick.y = 1.0f;
				}
				break;
			case AInputCommand_LookLeftWithUp:
				if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				{
					m_vecStick.x = 0.0f;
				}
				else
				{
					bDPad        = TTRUE;
					m_vecStick.x = -1.0f;
					s_bUsingDPad = TTRUE;
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				}
				break;
			case AInputCommand_LookRightWithUp:
				if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				{
					m_vecStick.x = 0.0f;
				}
				else
				{
					bDPad        = TTRUE;
					m_vecStick.x = 1.0f;
					s_bUsingDPad = TTRUE;
					a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				}
				break;
			default:
				break;
		}

		if ( a_eInputCommand != AInputCommand_Look && a_eInputCommand != AInputCommand_Move && !bDPad )
			s_bUsingDPad = TFALSE;
	}
	else
	{
		s_bUsingDPad = TFALSE;
	}

PROCESS_COMMAND:
	if ( m_eMenu != MENU_ITEM || a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
		return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Cancel:
			if ( m_pMenu->IsQuantityShown() )
			{
				m_bCancel = TTRUE;
				return TTRUE;
			}

			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );

			if ( m_eMode != MODE_BUY )
			{
				m_eMenu = MENU_MAIN;
				SetPage( AGopherShopMenu::PAGE_MAIN );
			}
			else
			{
				m_eMenu = MENU_CATEGORIES;
				m_eMode = MODE_BUY;
				SetPage( AGopherShopMenu::PAGE_CATEGORIES );
			}

			return TTRUE;
		case AInputCommand_Left:
			m_bLeft = TTRUE;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Right:
			m_bRight = TTRUE;
			a_pInputEvent->StartRepeat( 0.5f, 0.1f );
			return TTRUE;
		case AInputCommand_Look:
		{
			if ( s_bUsingDPad || m_fInputTime <= 0.5f )
				return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

			TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
			TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

			if ( TMath::Abs( fX ) <= TMath::TFLOAT_EPSILON && TMath::Abs( fY ) <= TMath::TFLOAT_EPSILON )
			{
				m_bStick     = TFALSE;
				m_vecStick.x = 0.0f;
				m_vecStick.y = 0.0f;
				return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
			}

			m_vecStick.y = fY;
			m_bStick     = TTRUE;
			m_vecStick.x = fX;
			return TTRUE;
		}
		case AInputCommand_Shoot:
		{
			if ( m_pMenu->IsQuantityShown() )
			{
				m_bConfirm = TTRUE;
				return TTRUE;
			}

			if ( m_pCurrentItem->GetNumRemaining() != 0 || ( m_eMode == MODE_SELL && m_pCurrentItem->GetCount() != 0 ) )
			{
				if ( !m_pCurrentItem->IsUnlocked() )
					return TTRUE;

				if ( !HasRequiredItems( m_pCurrentItem ) )
					return TTRUE;

				TINT iMoney = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups();

				if ( iMoney < m_pCurrentItem->GetPrice() && m_eMode == MODE_BUY )
					return TTRUE;

				if ( m_eMode != MODE_SELL )
				{
					// Are you sure?
					m_pMenu->ShowConfirm( TTRUE );
					return TTRUE;
				}

				// How many?
				m_pMenu->ShowQuantity( TTRUE );
				m_pMenu->SetQuantity( 1 );

				TINT iMultiplier = GetPriceMultiplier( m_pCurrentItem, 1 );
				m_pMenu->SetPrice( iMultiplier * m_pCurrentItem->GetPrice(), iMultiplier );
			}

			return TTRUE;
		}
		default:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 00495c50
void AGopherShopGame::OnLoaded()
{
	LoadPriceList();
	CountAvailableItems();
}

// $Barnyard: FUNCTION 00492a00
ASoundId AGopherShopGame::GetSound()
{
	return soundbank::MUSIC_GOPHERS;
}

// $Barnyard: FUNCTION 00496170
TBOOL AGopherShopGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	UpdateMouse( a_fDeltaTime );
	m_fTime += a_fDeltaTime;

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( m_bExiting == TTRUE )
	{
		// Leave once the gopher has said goodbye
		if ( !pSoundManager->IsCueUsed( m_iCue ) )
		{
			m_pGopher->FinishAnimation( TPS8( gopher_talk ) );
			Exit();
			return TTRUE;
		}

		return AGameState::OnUpdate( a_fDeltaTime );
	}

	// The gopher idles when he isn't talking
	if ( ( !m_pGopher->IsAnimPlaying( TPS8( gopher_callout ), 0.0f ) && !m_pGopher->IsAnimPlaying( TPS8( gopher_talk ), 0.0f ) && !m_pGopher->IsAnimPlaying( TPS8( gopher_idle ), 0.0f ) ) ||
	     ( !pSoundManager->IsCueUsed( m_iIdleCue ) && !pSoundManager->IsCueUsed( m_iCue ) ) )
	{
		m_pGopher->PlayAnimation( TPS8( gopher_idle ) );
	}

	if ( !pSoundManager->IsCueUsed( m_iCue ) && !pSoundManager->IsCueUsed( m_iIdleCue ) )
	{
		// Says something now and then
		m_fTalkTime += a_fDeltaTime;

		if ( m_fTalkTime > 2.0f )
		{
			if ( ARandom::GetSingleton()->m_oRandom.GetFloat() > 0.8f )
			{
				m_pGopher->FinishAnimation( TPS8( gopher_talk ) );
				m_pGopher->PlayAnimation( TPS8( gopher_talk ) );
				m_iIdleCue = pSoundManager->PlayCueAtPosition( soundbank::GOPHERS_CHAT, m_vecGopherPos );
			}

			m_fTalkTime = 0.0f;
		}
	}

	m_pMenu->Update( a_fDeltaTime );
	m_fInputTime += a_fDeltaTime;

	if ( m_pMenu->IsTransitioning() )
		return TTRUE;

#ifdef BARNYARD_COMMUNITY_PATCH
	Dev_Update( a_fDeltaTime );
#endif

	if ( m_eMenu == MENU_ITEM )
	{
		AGopherShopCategory* pCategory = &m_aCategories[ m_iCategory ];
		pCategory->Update( a_fDeltaTime, m_eMode );

		if ( m_pMenu->IsQuantityShown() )
		{
			// Selling: the arrows change the quantity
			if ( m_bRight == TTRUE )
			{
				pSoundManager->PlayCue( soundbank::UI_MENUCLICK );
				m_pMenu->FlashArrow( TFALSE );

				TINT iQuantity = m_pMenu->GetQuantity() + 1;
				TMath::Clip( iQuantity, 1, m_pCurrentItem->GetCount() );
				m_pMenu->SetQuantity( iQuantity );

				TINT iMultiplier = GetPriceMultiplier( m_pCurrentItem, iQuantity );
				m_pMenu->SetPrice( iQuantity * iMultiplier * m_pCurrentItem->GetPrice(), iMultiplier );
			}
			else if ( m_bLeft == TTRUE )
			{
				pSoundManager->PlayCue( soundbank::UI_MENUCLICK );
				m_pMenu->FlashArrow( TTRUE );

				TINT iQuantity = m_pMenu->GetQuantity() - 1;
				TMath::Clip( iQuantity, 1, m_pCurrentItem->GetCount() );
				m_pMenu->SetQuantity( iQuantity );

				TINT iMultiplier = GetPriceMultiplier( m_pCurrentItem, iQuantity );
				m_pMenu->SetPrice( iQuantity * iMultiplier * m_pCurrentItem->GetPrice(), iMultiplier );
			}
			else if ( m_bConfirm == TTRUE )
			{
				SellMultiple( m_pCurrentItem, m_pMenu->GetQuantity() );
				m_pMenu->ShowQuantity( TFALSE );

				if ( m_pCurrentItem->GetCount() == 0 )
				{
					// Sold all of them
					m_aCategories[ m_iCategory ].RemoveItemAndKeep( m_pCurrentItem );
					m_pCurrentItem = m_aCategories[ m_iCategory ].GetCurrentItem();

					if ( !m_pCurrentItem )
					{
						// Nothing more to sell
						pSoundManager->PlayCue( soundbank::UI_MENUCLICK );

						if ( m_eMode != MODE_BUY )
						{
							m_eMenu = MENU_MAIN;
							SetPage( AGopherShopMenu::PAGE_MAIN );
						}
						else
						{
							m_eMenu = MENU_CATEGORIES;
							m_eMode = MODE_BUY;
							SetPage( AGopherShopMenu::PAGE_CATEGORIES );
						}

						goto UPDATE_DONE;
					}
				}

				UpdateArrows( pCategory );
				UpdateItemInfo( TTRUE );
			}
			else if ( m_bCancel == TTRUE )
			{
				m_pMenu->ShowQuantity( TFALSE );
				UpdateArrows( pCategory );
				UpdateItemInfo( TTRUE );
			}
		}
		else if ( m_bRight == TTRUE )
		{
			pSoundManager->PlayCue( soundbank::UI_MENUCLICK );

			if ( pCategory->GetNumItems() > 1 )
			{
				m_pMenu->FlashArrow( TFALSE );
				pCategory->ScrollRight();
				UpdateArrows( pCategory );
				m_pCurrentItem = m_aCategories[ m_iCategory ].GetCurrentItem();
				UpdateItemInfo( TTRUE );
			}
		}
		else if ( m_bLeft == TTRUE )
		{
			pSoundManager->PlayCue( soundbank::UI_MENUCLICK );

			if ( pCategory->GetNumItems() > 1 )
			{
				m_pMenu->FlashArrow( TTRUE );
				pCategory->ScrollLeft();
				UpdateArrows( pCategory );
				m_pCurrentItem = m_aCategories[ m_iCategory ].GetCurrentItem();
				UpdateItemInfo( TTRUE );
			}
		}
	}

UPDATE_DONE:
	m_bLeft    = TFALSE;
	m_bRight   = TFALSE;
	m_bUnk1B2  = TFALSE;
	m_bUnk1B3  = TFALSE;
	m_bConfirm = TFALSE;
	m_bCancel  = TFALSE;

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00495ef0
void AGopherShopGame::OnInsertion()
{
	GetHUDParams().SetFlags( AHudElement::MASK_MENU_NAV );
	AGameTimeManager::GetSingleton()->PushUpdateState();

	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvBlackMarket, TFALSE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::SpawnSimPlayer();

	// The player isn't there
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetPlayerUnit( 0 );
	pPlayer->SetAIEnabled( TFALSE );
	pPlayer->SetMovement( TFALSE );
	pPlayer->SetExternalUpdate( TFALSE );
	TSTATICCAST( AVisibleUnit, pPlayer )->SetVisible( TFALSE );

	// The camera looks at the gopher
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	ATerrainInterface* pTerrain  = ATerrainInterface::GetSingleton();
	TVector4           vecCamera = pTerrain->GetLocatorPos( "camera" );
	pTerrain->GetLocatorPos( "gopher" );

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TVector4 vecDirection( 0.0f, 0.0f, 1.0f, 1.0f );
	vecDirection.Normalise();

	pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecCamera;
	pCamera->LookAtDirection( vecDirection );

	s_fOldFOV = pCamera->GetFOV();
	pCamera->SetFOV( 0.69813168f );

	AHackedQuickSave::Buffer* pBuffer = AGopherShopInterface::GetShopBuffer();

	if ( pBuffer->IsSaved() )
	{
		pBuffer->SetCursor( 0 );
		Load( pBuffer );
		pBuffer->SetSaved( TFALSE );
	}

	InitCategories();
	CreateGopher();
	CreateMenu();

	m_eMenu    = MENU_MAIN;
	m_bLeft    = TFALSE;
	m_bRight   = TFALSE;
	m_bUnk1B2  = TFALSE;
	m_bUnk1B3  = TFALSE;
	m_bConfirm = TFALSE;
	m_bCancel  = TFALSE;

	AGameTimeFXManager* pFXManager = AGameTimeFXManager::GetSingleton();
	AGameTimeFX         oFX        = *pFXManager->FindFX( "gophershop" );
	pFXManager->SetForcedFX( &oFX );

	m_pFade = new AGUI2FadeRectangle();
	m_pFade->Start( 0xff000000, 0x00000000, 2.0f );

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );
	ASunFlareEffect::GetSingleton()->SetEnabled( TFALSE );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00493000
void AGopherShopGame::OnRemoval()
{
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( s_fOldFOV );
	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetDefaultTexts();

	if ( m_pFade )
		delete m_pFade;

	AGameTimeFXManager::GetSingleton()->ClearForcedFX();
	AGameTimeManager::GetSingleton()->PopUpdateState();

	if ( m_pMenu )
		delete m_pMenu;

	for ( TINT i = 0; i < NUM_CATEGORIES; i++ )
	{
		// The items to sell are the ones of the inventory
		if ( i != CATEGORY_SELL )
			m_aCategories[ i ].DestroyItems();
	}

	for ( TINT i = 0; i < m_iNumOwned; i++ )
	{
		if ( m_apOwned[ i ] )
			delete m_apOwned[ i ];
	}

	AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pGopher );

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_RENDER );
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );
	ASunFlareEffect::GetSingleton()->SetEnabled( TTRUE );

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00492fc0
void AGopherShopGame::OnActivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TTRUE );
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00492fe0
void AGopherShopGame::OnDeactivate()
{
	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00492920
TINT AGopherShopGame::GetCategoryTextId( TINT a_iCategory )
{
	switch ( a_iCategory )
	{
		case 0: return 977;
		case 1: return 978;
		case 2: return 979;
		case 3: return 980;
		case 4: return 981;
		case 5: return 982;
		case 6: return 912;
		default: return -1;
	}
}

// $Barnyard: FUNCTION 00492990
void AGopherShopGame::InitCategories()
{
	for ( TINT i = 0; i < NUM_CATEGORIES; i++ )
		m_aCategories[ i ].SetName( GetCategoryTextId( i ) );
}

// $Barnyard: FUNCTION 00493420
void AGopherShopGame::CreateGopher()
{
	TQuaternion quatRotation;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( "gopher", m_vecGopherPos, quatRotation );

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
	m_pGopher                          = pManager->CreateAnimatableObject( pManager->FindType( TPS8( gopher_gs_anims ) ), TNULL, 0 );
	m_pGopher->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pGopher->SetVisible( TTRUE );
	m_pGopher->GetModelInstance()->SetTranslation( m_vecGopherPos.AsVector3() );

	// NOTE: retail passes the rotation of the locator as the direction
	m_pGopher->GetModelInstance()->SetDirection( *TREINTERPRETCAST( TVector4*, &quatRotation ) );
}

// $Barnyard: FUNCTION 00495c60
void AGopherShopGame::CreateMenu()
{
	GetGUIElement().Show();

	m_pMenu = new AGopherShopMenu();
	m_pMenu->Create( &GetGUIElement() );

	for ( TINT i = 0; i < AGopherShopMenu::MENU_NUMOF; i++ )
	{
		m_pMenu->GetMenu( i )->SetItemActivationCallback( OnMenuItemActivatedCallback );
		m_pMenu->GetMenu( i )->SetCallbackUserData( this );
	}

	// The buttons of the categories
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_0 )->SetId( 0 );
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_1 )->SetId( 1 );
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_1 + 1 )->SetId( 2 );
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_1 + 2 )->SetId( 3 );
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_1 + 3 )->SetId( 4 );
	m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_1 + 4 )->SetId( 5 );

	m_pMenu->SetPage( AGopherShopMenu::PAGE_MAIN, TFALSE );
	m_pMenu->SetMoney( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups() );

	// Hello!
	m_pGopher->PlayAnimation( TPS8( gopher_callout ) );

	if ( m_bFirstVisit == TTRUE )
	{
		m_iCue        = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOPHERS_FRANKIE_NAME, m_vecGopherPos );
		m_bFirstVisit = TFALSE;
	}
	else
	{
		m_iCue = ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOPHERS_SHIPMENT, m_vecGopherPos );
	}

	for ( TINT i = 0; i < NUM_CATEGORIES; i++ )
		m_aCategories[ i ].SetModelRenderer( m_pMenu->GetModelRenderer( 0 ) );
}

// $Barnyard: FUNCTION 00494810
TBOOL AGopherShopGame::LoadPriceList()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data/MinigameData/GopherShop/GopherPriceList.trb", 0 ) != TTRB::ERROR_OK )
		return TFALSE;

	TINT*             pNumPrices = TSTATICCAST( TINT, oTRB.GetSymbolAddress( "NumberOfCostItems" ) );
	AGopherShopPrice* pPrices    = TSTATICCAST( AGopherShopPrice, oTRB.GetSymbolAddress( "CostMapping" ) );

	AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	for ( TINT i = 0; i < *pNumPrices; i++ )
	{
		AGopherShopPrice& rPrice = pPrices[ i ];

		if ( ( rPrice.iMaxCount == 0 && rPrice.iCategory != 3 ) || rPrice.iCategory == 5 || ( rPrice.iCategory == 4 && rPrice.iTextId == 313 ) )
			continue;

		AGopherShopItem*    pItem = AGopherShopItem::Create( rPrice );
		AGopherShopPurchase oPurchase;

		if ( rPrice.iCategory == 3 )
		{
			// An item of the inventory that can be sold
			TPString8 strType = pItem->GetModel();

			if ( pItem->GetId() == 381 )
				strType = TPS8( honeycomb );

			AInventoryItem* pInventoryItem = pInventory->FindItemByType( strType );
			m_apOwned[ m_iNumOwned++ ]     = pItem;

			if ( pInventoryItem )
			{
				pItem->SetCount( pInventoryItem->GetCount() );
				m_aPurchases[ m_iNumPurchases++ ] = pItem->GetPurchase();
			}
			else
			{
				pItem->SetCount( -1 );
			}
		}
		else
		{
			// Restore the count of the item from the save
			if ( FindPurchase( pItem->GetId(), &oPurchase ) == TTRUE )
			{
				for ( TINT k = 0; k < oPurchase.iCount; k++ )
					pItem->IncreaseCount( 1 );
			}

			// The tab of the shop (offset 0x18), not the kind of the item
			m_aCategories[ rPrice.iShopCategory ].AddItem( pItem );
		}
	}

	// The saved purchases only have the ids and the counts, fill them in from the items
	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		AGopherShopPurchase& rPurchase = m_aPurchases[ i ];

		for ( TINT k = 0; k < NUM_CATEGORIES; k++ )
		{
			if ( k == CATEGORY_SELL )
				continue;

			AGopherShopItem* pItem = m_aCategories[ k ].FindItem( rPurchase );

			if ( pItem )
			{
				AGopherShopPurchase oPurchase = pItem->GetPurchase();
				rPurchase.iCategory           = oPurchase.iCategory;
				rPurchase.strInventoryType    = oPurchase.strInventoryType;
				rPurchase.strName2            = oPurchase.strName2;
				rPurchase.strModel            = oPurchase.strModel;
				rPurchase.strQuest            = oPurchase.strQuest;
				rPurchase.strName3            = oPurchase.strName3;
				rPurchase.iMaxCount           = oPurchase.iMaxCount;
				break;
			}
		}
	}

	for ( TINT i = 0; i < NUM_CATEGORIES; i++ )
		m_aCategories[ i ].SetActive( TFALSE );

	oTRB.Close();
	return TTRUE;
}

// $Barnyard: FUNCTION 00494420
void AGopherShopGame::CountAvailableItems()
{
	m_iNumAvailable = 0;

	for ( TINT i = 0; i < NUM_CATEGORIES; i++ )
	{
		if ( i == CATEGORY_SELL )
			continue;

		AGopherShopCategory& rCategory = m_aCategories[ i ];

		for ( TINT k = 0; k < rCategory.GetNumItems(); k++ )
		{
			AGopherShopItem* pItem = rCategory.GetItem( k );

			if ( pItem->IsUnlocked() == TTRUE && pItem->IsSoldOut() == TFALSE && HasRequiredItems( pItem ) == TTRUE )
				m_iNumAvailable++;
		}
	}
}

// $Barnyard: FUNCTION 00493210
void AGopherShopGame::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	// Only the bought items of these categories are saved
	TINT iNumSaved = 0;

	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		TINT iCategory = m_aPurchases[ i ].iCategory;

		if ( iCategory == 0 || iCategory == 4 || iCategory == 5 )
			iNumSaved++;
	}

	a_pBuffer->WriteUInt8( m_uiJoeyOffer );
	a_pBuffer->WriteInt( iNumSaved );
	a_pBuffer->WriteBool( m_bFirstVisit );
	a_pBuffer->WriteInt( m_iNumAvailable );

	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		TINT iCategory = m_aPurchases[ i ].iCategory;

		if ( iCategory == 0 || iCategory == 4 || iCategory == 5 )
		{
			a_pBuffer->WriteInt( m_aPurchases[ i ].iId );
			a_pBuffer->WriteInt( m_aPurchases[ i ].iCount );
		}
	}
}

// $Barnyard: FUNCTION 00493c20
void AGopherShopGame::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadUInt8( m_uiJoeyOffer );

	TINT iNumSaved = 0;
	a_pBuffer->ReadInt( iNumSaved );
	a_pBuffer->ReadBool( m_bFirstVisit );
	a_pBuffer->ReadInt( m_iSavedNumAvailable );

	for ( TINT i = 0; i < iNumSaved; i++ )
	{
		// NOTE: retail leaves the category and the max count uninitialised (LoadPriceList fills them in)
		AGopherShopPurchase oPurchase;
		a_pBuffer->ReadInt( oPurchase.iId );
		a_pBuffer->ReadInt( oPurchase.iCount );

		if ( oPurchase.iId == 896 )
			m_bDiscount = TTRUE;

		m_aPurchases[ m_iNumPurchases++ ] = oPurchase;
	}
}

// $Barnyard: FUNCTION 00494cd0
void AGopherShopGame::SetCategory( TINT a_iCategory )
{
	AGopherShopCategory* pCategory = &m_aCategories[ a_iCategory ];
	m_pMenu->SetDescription( pCategory->GetTextId() );
	m_aCategories[ m_iCategory ].SetActive( TFALSE );
	m_pMenu->GetModelRenderer( 0 )->RemoveAllObjects();

	m_iCategory = a_iCategory;
	pCategory->Show();
	m_aCategories[ m_iCategory ].SetActive( TTRUE );

	TBOOL bScrolls = m_aCategories[ m_iCategory ].GetNumItems() > 1;
	m_pMenu->GetRectangle( 3 )->SetVisible( bScrolls );
	m_pMenu->GetRectangle( 4 )->SetVisible( bScrolls );

	if ( m_aCategories[ m_iCategory ].GetNumItems() == 2 )
		m_pMenu->GetRectangle( 3 )->Hide();

	UpdateArrows( &m_aCategories[ m_iCategory ] );
	UpdateItemInfo( TTRUE );
}

// $Barnyard: FUNCTION 00494e10
void AGopherShopGame::SetPage( TINT a_ePage )
{
	if ( a_ePage == AGopherShopMenu::PAGE_MAIN )
	{
		// Forget the items to sell
		if ( m_eMode == MODE_SELL )
			m_aCategories[ CATEGORY_SELL ].Clear();
	}
	else if ( a_ePage == AGopherShopMenu::PAGE_ITEM )
	{
		m_pMenu->SetMode( m_eMode );

		if ( m_eMode == MODE_SELL )
		{
			// The items of the inventory that can be sold
			TINT iNumItems = 0;

			for ( TINT i = 0; i < m_iNumOwned; i++ )
			{
				AGopherShopItem* pItem = m_apOwned[ i ];

				if ( pItem->GetCount() > 0 )
				{
					m_aCategories[ CATEGORY_SELL ].AddItem( pItem );
					iNumItems++;
				}
			}

			if ( iNumItems <= 0 )
			{
				m_eMenu = MENU_MAIN;
				return;
			}

			a_ePage     = AGopherShopMenu::PAGE_ITEM;
			m_iCategory = CATEGORY_SELL;
		}

		m_bShowingItem = TTRUE;
		SetCategory( m_iCategory );
		m_pMenu->SetPage( a_ePage, m_eMode == MODE_SELL );
		return;
	}

	m_aCategories[ m_iCategory ].SetActive( TFALSE );
	m_pMenu->SetPage( a_ePage, m_eMode == MODE_SELL );
}

// $Barnyard: FUNCTION 004944b0
void AGopherShopGame::UpdateItemInfo( TBOOL a_bUpdateStatus )
{
	AGopherShopItem* pItem  = m_aCategories[ m_iCategory ].GetCurrentItem();
	TINT             iMoney = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups();

	m_pMenu->SetMoney( iMoney );
	m_pCurrentItem = pItem;
	m_pMenu->SetItemName( pItem->GetId() );
	m_pMenu->SetItemCount( ( m_eMode != MODE_BUY ) ? pItem->GetCount() : -1 );

	if ( a_bUpdateStatus )
	{
		AGopherShopMenu::STATUS eStatus = AGopherShopMenu::STATUS_UNAVAILABLE;

		if ( !IsFull( pItem ) || m_eMode != MODE_BUY )
		{
			if ( pItem->IsSoldOut() == TTRUE && m_eMode == MODE_BUY )
			{
				eStatus = AGopherShopMenu::STATUS_SOLD_OUT;
			}
			else if ( ( pItem->IsUnlocked() && HasRequiredItems( pItem ) ) || m_eMode != MODE_BUY )
			{
				if ( iMoney < pItem->GetPrice() && m_eMode == MODE_BUY )
					eStatus = AGopherShopMenu::STATUS_EXPENSIVE;
				else if ( pItem->HasCount() || m_eMode != MODE_SELL )
					eStatus = AGopherShopMenu::STATUS_AVAILABLE;
				else
					eStatus = AGopherShopMenu::STATUS_FULL;
			}
		}

		m_pMenu->SetStatus( eStatus, TFALSE );
	}

	TINT iPrice = pItem->GetPrice();

	if ( m_eMode == MODE_BUY && m_bDiscount && !HasNoDiscount( pItem, iPrice ) )
		iPrice = TINT( TFLOAT( iPrice ) * 0.9f );

	m_pMenu->SetPrice( iPrice, 1 );
}

// $Barnyard: FUNCTION 00492a10
void AGopherShopGame::UpdateArrows( AGopherShopCategory* a_pCategory )
{
	TBOOL bRight = TFALSE;
	TBOOL bLeft  = TFALSE;

	if ( a_pCategory->GetNumItems() > 1 )
	{
		if ( a_pCategory->GetNumItems() >= 3 )
		{
			bLeft  = TTRUE;
			bRight = TTRUE;
		}
		else
		{
			TINT iCurrent = a_pCategory->GetCurrentIndex();
			bLeft         = iCurrent == 0;
			bRight        = iCurrent == a_pCategory->GetNumItems() - 1;
		}
	}

	m_pMenu->GetRectangle( 3 )->SetVisible( bRight );
	m_pMenu->GetRectangle( 4 )->SetVisible( bLeft );
}

// $Barnyard: FUNCTION 00492ac0
void AGopherShopGame::UpdateMouse( TFLOAT a_fDeltaTime )
{
	T2GUITransform oInvTransform;
	TVector2       vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
	TVector2       vecLocalPos;

	if ( m_bMouseOnModel == TFALSE )
	{
		AGUI2ModelRenderer* pModelRenderer = m_pMenu->GetModelRenderer( 0 );
		pModelRenderer->GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bMouseOnModel = pModelRenderer->IsPointInside( vecLocalPos.x, vecLocalPos.y );
	}

	if ( m_bMouseOnRightArrow == TFALSE )
	{
		T2GUIRectangle* pArrow = m_pMenu->GetRectangle( 3 );
		pArrow->GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bMouseOnLeftArrow = pArrow->IsPointInside( vecLocalPos.x, vecLocalPos.y );

		// The arrow under the mouse pulses
		TFLOAT fScale = m_bMouseOnLeftArrow ? TMath::Sin( TMath::TWO_PI * m_fTime ) * 0.05f + 1.0f : 1.0f;
		pArrow->GetTransform().SetScale( fScale, fScale );
	}

	if ( m_bMouseOnLeftArrow == TFALSE )
	{
		T2GUIRectangle* pArrow = m_pMenu->GetRectangle( 4 );
		pArrow->GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bMouseOnRightArrow = pArrow->IsPointInside( vecLocalPos.x, vecLocalPos.y );

		TFLOAT fScale = m_bMouseOnRightArrow ? TMath::Sin( TMath::TWO_PI * m_fTime ) * 0.05f + 1.0f : 1.0f;
		pArrow->GetTransform().SetScale( fScale, fScale );
	}
}

// $Barnyard: FUNCTION 004929c0
TINT AGopherShopGame::GetPriceMultiplier( AGopherShopItem* a_pItem, TINT a_iUnused )
{
	if ( m_uiJoeyOffer == 0xff )
		return 1;

	return ( a_pItem->GetId() == AGopherShopInterface::GetJoeyOffer( m_uiJoeyOffer ).iItemId ) ? 2 : 1;
}

// $Barnyard: FUNCTION 00493980
TBOOL AGopherShopGame::HasRequiredItems( AGopherShopItem* a_pItem )
{
	for ( TINT i = 0; i < a_pItem->GetNumRequired(); i++ )
	{
		TPString8        strRequired = a_pItem->GetRequired( i );
		AGopherShopItem* pRequired   = TNULL;

		for ( TINT k = 0; k < NUM_CATEGORIES; k++ )
		{
			if ( k == CATEGORY_SELL )
				continue;

			pRequired = m_aCategories[ k ].FindItemByModel( strRequired );

			if ( pRequired )
				break;
		}

		if ( !pRequired )
			return TFALSE;

		if ( pRequired->HasCount() == TTRUE )
			return TTRUE;

		if ( i + 1 >= a_pItem->GetNumRequired() )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00493120
TBOOL AGopherShopGame::IsFull( AGopherShopItem* a_pItem )
{
	TINT iCount = 0;

	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		if ( m_aPurchases[ i ].iCount != 0 && m_aPurchases[ i ].iId == a_pItem->GetId() )
		{
			iCount = m_aPurchases[ i ].iCount;
			break;
		}
	}

	return iCount >= 20;
}

// $Barnyard: FUNCTION 00494650
TBOOL AGopherShopGame::FindPurchase( TINT a_iId, AGopherShopPurchase* a_pOutPurchase )
{
	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		if ( m_aPurchases[ i ].iId == a_iId )
		{
			if ( a_pOutPurchase )
				*a_pOutPurchase = m_aPurchases[ i ];

			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00493190
TINT AGopherShopGame::FindPurchaseIndex( const AGopherShopPurchase& a_rcPurchase )
{
	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		if ( a_rcPurchase.iId == m_aPurchases[ i ].iId )
			return i;
	}

	return m_iNumPurchases;
}

// $Barnyard: FUNCTION 00493ae0
void AGopherShopGame::RemovePurchase( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < m_iNumPurchases - 1; i++ )
		m_aPurchases[ i ] = m_aPurchases[ i + 1 ];

	m_aPurchases[ m_iNumPurchases - 1 ] = AGopherShopPurchase();
	m_iNumPurchases--;
}

// $Barnyard: FUNCTION 00494f20
void AGopherShopGame::Buy( AGopherShopItem* a_pItem )
{
	if ( a_pItem->GetNumRemaining() == 0 || !a_pItem->IsUnlocked() || !HasRequiredItems( a_pItem ) || ( IsFull( a_pItem ) && m_eMode == MODE_BUY ) )
		return;

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
	TINT               iMoney   = pProfile->GetDisplayedPickups();

	if ( iMoney >= a_pItem->GetPrice() )
	{
		AGopherShopPurchase oPurchase;

		if ( a_pItem->GetCategory() == 4 && m_aCategories[ 0 ].GetNumItems() > 0 )
		{
			// Another bike that was bought is returned
			// NOTE: retail looks for the bikes in the first category
			for ( TINT i = 0; i < m_aCategories[ 0 ].GetNumItems(); i++ )
			{
				AGopherShopItem* pBike = m_aCategories[ 0 ].GetItem( i );

				if ( pBike->GetCategory() == 4 && pBike->IsSoldOut() )
				{
					pBike->DecreaseCount( 1 );
					RemovePurchase( FindPurchaseIndex( pBike->GetPurchase() ) );
					break;
				}
			}
		}

		a_pItem->IncreaseCount( 1 );

		if ( FindPurchase( a_pItem->GetId(), &oPurchase ) )
		{
			m_aPurchases[ FindPurchaseIndex( oPurchase ) ].iCount++;
		}
		else
		{
			m_aPurchases[ m_iNumPurchases++ ] = a_pItem->GetPurchase();

			if ( a_pItem->GetId() == 896 )
				m_bDiscount = TTRUE;
		}

		// The item is in the inventory now
		for ( TINT i = 0; i < m_iNumOwned; i++ )
		{
			AGopherShopItem* pOwned = m_apOwned[ i ];

			if ( pOwned->GetId() == a_pItem->GetId() )
			{
				if ( pOwned->GetCount() == -1 )
					pOwned->SetCount( 0 );

				pOwned->SetCount( pOwned->GetCount() + 1 );
			}
		}

		TINT iPrice = a_pItem->GetPrice();

		if ( m_bDiscount && !HasNoDiscount( a_pItem, iPrice ) )
			iPrice = TINT( TFLOAT( iPrice ) * 0.9f );

		pProfile->SetPickups( iMoney - iPrice, TTRUE, TTRUE );

		if ( a_pItem->IsSoldOut() == TTRUE )
		{
			m_pMenu->SetStatus( AGopherShopMenu::STATUS_SOLD_OUT, TTRUE );
		}
		else
		{
			// NOTE: the displayed money is still the old one here
			m_pMenu->SetStatus( ( pProfile->GetDisplayedPickups() >= a_pItem->GetPrice() ) ? AGopherShopMenu::STATUS_AVAILABLE : AGopherShopMenu::STATUS_EXPENSIVE, TFALSE );
		}

		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		if ( !pSoundManager->IsCueUsed( m_iIdleCue ) && !pSoundManager->IsCueUsed( m_iCue ) )
		{
			m_pGopher->FinishAnimation( TPS8( gopher_talk ) );
			m_pGopher->PlayAnimation( TPS8( gopher_talk ) );
			m_iIdleCue = -1;
			m_iCue     = pSoundManager->PlayCueAtPosition( soundbank::GOPHERS_BUY, m_vecGopherPos );
		}

		m_bBought = TTRUE;
	}

	UpdateItemInfo( a_pItem->GetCategory() == 1 );
}

// $Barnyard: FUNCTION 004952e0
void AGopherShopGame::Sell( AGopherShopItem* a_pItem, TINT a_iMultiplier, TBOOL a_bPlaySound )
{
	a_pItem->DecreaseCount( 1 );

	if ( m_pCurrentItem->GetCount() != 0 )
		m_pMenu->SetStatus( AGopherShopMenu::STATUS_AVAILABLE, TFALSE );
	else
		m_pMenu->SetStatus( AGopherShopMenu::STATUS_SOLD_OUT, TFALSE );

	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		if ( m_aPurchases[ i ].iId == a_pItem->GetId() )
			m_aPurchases[ i ].iCount--;
	}

	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
	pProfile->SetPickups( pProfile->GetDisplayedPickups() + a_iMultiplier * m_pCurrentItem->GetPrice(), a_bPlaySound, TTRUE );

	UpdateItemInfo( TTRUE );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( a_bPlaySound == TTRUE && !pSoundManager->IsCueUsed( m_iIdleCue ) && !pSoundManager->IsCueUsed( m_iCue ) )
	{
		m_pGopher->FinishAnimation( TPS8( gopher_talk ) );
		m_pGopher->PlayAnimation( TPS8( gopher_talk ) );
		m_iIdleCue = -1;
		m_iCue     = pSoundManager->PlayCueAtPosition( soundbank::GOPHERS_SELL, m_vecGopherPos );
	}
}

// $Barnyard: FUNCTION 00495e90
void AGopherShopGame::SellMultiple( AGopherShopItem* a_pItem, TINT a_iCount )
{
	TINT iMultiplier = 1;

	if ( m_uiJoeyOffer != 0xff && a_pItem->GetId() == AGopherShopInterface::GetJoeyOffer( m_uiJoeyOffer ).iItemId )
		iMultiplier = 2;

	for ( TINT i = a_iCount; i > 0; i-- )
		Sell( a_pItem, iMultiplier, TFALSE );
}

// $Barnyard: FUNCTION 00495450
void AGopherShopGame::OnMainMenuItem( AGUI2MenuItem* a_pItem )
{
	if ( a_pItem == m_pMenu->GetButton( AGopherShopMenu::BUTTON_BUY ) )
	{
		m_eMenu = MENU_CATEGORIES;
		m_eMode = MODE_BUY;
		SetPage( AGopherShopMenu::PAGE_CATEGORIES );
		m_pMenu->GetMenu( AGopherShopMenu::MENU_CATEGORIES )->SetFocusAt( *m_pMenu->GetButton( AGopherShopMenu::BUTTON_CATEGORY_0 ) );
	}
	else if ( a_pItem == m_pMenu->GetButton( AGopherShopMenu::BUTTON_SELL ) )
	{
		m_eMenu = MENU_ITEM;
		m_eMode = MODE_SELL;
		SetPage( AGopherShopMenu::PAGE_ITEM );
	}
	else if ( a_pItem == m_pMenu->GetButton( AGopherShopMenu::BUTTON_LEAVE ) )
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		if ( !pSoundManager->IsCueUsed( m_iIdleCue ) && !pSoundManager->IsCueUsed( m_iCue ) )
		{
			// Goodbye (thanks if something was bought)
			m_pGopher->FinishAnimation( TPS8( gopher_talk ) );
			m_pGopher->PlayAnimation( TPS8( gopher_talk ) );
			m_iIdleCue = -1;
			m_iCue     = pSoundManager->PlayCueAtPosition( ASoundId( ( m_bBought == TTRUE ) ? 491 : 493 ), m_vecGopherPos );
		}

		m_bExiting  = TTRUE;
		m_fTalkTime = 2.0f;
	}
}

// $Barnyard: FUNCTION 004955b0
void AGopherShopGame::OnCategoryMenuItem( AGUI2MenuItem* a_pItem )
{
	TINT ePage = AGopherShopMenu::PAGE_ITEM;
	m_eMenu    = MENU_ITEM;

	if ( a_pItem == m_pMenu->GetButton( AGopherShopMenu::BUTTON_BACK ) )
	{
		m_eMenu = MENU_MAIN;
		ePage   = AGopherShopMenu::PAGE_MAIN;
	}
	else
	{
		m_iCategory = TSTATICCAST( AGUI2Button, a_pItem )->GetId();
	}

	// Nothing in the category
	if ( m_eMenu != MENU_MAIN && m_aCategories[ m_iCategory ].GetNumItems() == 0 )
	{
		ePage   = AGopherShopMenu::PAGE_CATEGORIES;
		m_eMenu = MENU_CATEGORIES;
	}

	SetPage( ePage );
}

// $Barnyard: FUNCTION 00495630
void AGopherShopGame::OnMenuItemActivated( AGUI2MenuItem* a_pItem )
{
	if ( m_eMenu == MENU_MAIN )
	{
		OnMainMenuItem( a_pItem );
	}
	else if ( m_eMenu == MENU_CATEGORIES )
	{
		OnCategoryMenuItem( a_pItem );
	}
	else if ( m_eMenu == MENU_ITEM )
	{
		m_pMenu->ShowConfirm( TFALSE );

		if ( a_pItem == m_pMenu->GetButton( AGopherShopMenu::BUTTON_YES ) )
			Buy( m_pCurrentItem );
	}
}

// $Barnyard: FUNCTION 004956a0
void AGopherShopGame::OnMenuItemActivatedCallback( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	TSTATICCAST( AGopherShopGame, a_pUserData )->OnMenuItemActivated( a_pItem );
}

// $Barnyard: FUNCTION 00493ab0
TBOOL __stdcall AGopherShopGame::OnQuitCallback( AStartMenuState* a_pStartMenu, void* a_pUserData, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	a_pPopup->Remove();

	// Yes, leave the shop
	if ( a_pEvent->eButton == AInputCommand_Yes )
	{
		a_pStartMenu->Remove();
		TSTATICCAST( AGopherShopGame, a_pUserData )->Exit();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00493530
void AGopherShopGame::Exit()
{
	if ( m_pGopher->IsAnimationFinished( TPS8( gopher_talk ) ) == TTRUE )
		m_pGopher->PlayAnimation( TPS8( gopher_talk ) );

	// Save the shop
	AHackedQuickSave::Buffer* pBuffer = AGopherShopInterface::GetShopBuffer();
	pBuffer->SetCursor( 0 );
	Save( pBuffer );
	pBuffer->SetSaved( TTRUE );

	// Give the bought items
	AUnit*                    pPlayer    = APlayerManager::GetSingleton()->GetPlayerSet( 0 )->GetHumanPlayerUnit( 0 );
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	for ( TINT i = 0; i < m_iNumPurchases; i++ )
	{
		AGopherShopPurchase& rPurchase = m_aPurchases[ i ];

		if ( rPurchase.iCount == 0 )
			continue;

		if ( rPurchase.iCategory == 1 )
		{
			TPString8 strType = rPurchase.strModel;

			if ( rPurchase.iId == 381 )
				strType = TPS8( honeycomb );

			AInventoryItem* pInventoryItem = pInventory->FindItemByType( strType );

			if ( pInventoryItem )
				pInventoryItem->AddCount( rPurchase.iCount );
			else
				pInventory->AddItems( strType, rPurchase.iCount, AInventoryComponentInterface::ms_strDefaultInventory );
		}
		else if ( rPurchase.iCategory == 4 )
		{
			// The model of a bike ends with its number
			const TString8& rcModel = rPurchase.strModel.GetString8();
			TINT            iBike   = rcModel.GetString()[ rcModel.Length() - 1 ] - '1';
			ABikeManager::GetSingleton()->SetBikeTypeUnlocked( ABikeManager::GetBikeTypeFlag( iBike ), TTRUE );
		}
	}

	// Update the items of the inventory that were sold or bought
	for ( TINT i = 0; i < m_iNumOwned; i++ )
	{
		AGopherShopItem* pOwned = m_apOwned[ i ];

		if ( pOwned->GetCount() == -1 )
			continue;

		TPString8 strType = pOwned->GetModel();

		if ( pOwned->GetId() == 381 )
			strType = TPS8( honeycomb );

		AInventoryItem* pInventoryItem = pInventory->FindItemByType( strType );

		if ( pInventoryItem )
		{
			if ( pOwned->GetCount() != 0 )
				pInventoryItem->SetCount( pOwned->GetCount() );
			else
				pInventory->RemoveItem( pInventoryItem );
		}
	}

	AHackedQuickSave::GetSingleton()->SaveTime();
	AHackedQuickSave::GetSingleton()->SaveProfile();
	AGameLoader::LeaveMiniGame( TFALSE );
}

#ifdef BARNYARD_COMMUNITY_PATCH

void AGopherShopGame::Dev_Update( TFLOAT a_fDeltaTime )
{
	if ( !TStringManager::String8FindString( GetCommandLineA(), "-devgophershop" ) )
		return;

	static TFLOAT s_fTime  = 0.0f;
	static TINT   s_iPhase = 0;

	s_fTime += a_fDeltaTime;
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	if ( s_iPhase == 0 && s_fTime > 6.0f )
	{
		s_iPhase = 1;
		TINFO( "[devgophershop] in the shop, money %d, %d items available\n", pProfile->GetDisplayedPickups(), m_iNumAvailable );
		OnMainMenuItem( m_pMenu->GetButton( AGopherShopMenu::BUTTON_BUY ) );
	}
	else if ( s_iPhase == 1 && s_fTime > 9.0f )
	{
		s_iPhase = 2;

		// The first category with an item that can be bought
		for ( TINT i = 0; i < CATEGORY_SELL; i++ )
		{
			for ( TINT k = 0; k < m_aCategories[ i ].GetNumItems(); k++ )
			{
				AGopherShopItem* pItem = m_aCategories[ i ].GetItem( k );

				if ( pItem->GetNumRemaining() != 0 && pItem->IsUnlocked() && HasRequiredItems( pItem ) && pItem->GetPrice() <= pProfile->GetDisplayedPickups() )
				{
					TINT iButton = ( i == 0 ) ? AGopherShopMenu::BUTTON_CATEGORY_0 : AGopherShopMenu::BUTTON_CATEGORY_1 + i - 1;
					TINFO( "[devgophershop] category %d, item %d (id %d, price %d)\n", i, k, pItem->GetId(), pItem->GetPrice() );
					OnCategoryMenuItem( m_pMenu->GetButton( iButton ) );
					ms_iDevBoughtId = pItem->GetId();
					i               = CATEGORY_SELL;
					break;
				}
			}
		}
	}
	else if ( s_iPhase == 2 && s_fTime > 12.0f && m_eMenu == MENU_ITEM && m_pCurrentItem )
	{
		// Scroll to the item
		if ( m_pCurrentItem->GetId() != ms_iDevBoughtId )
		{
			// Scroll the shorter way
			AGopherShopCategory& rCategory = m_aCategories[ m_iCategory ];
			TINT                 iTarget   = 0;

			for ( TINT i = 0; i < rCategory.GetNumItems(); i++ )
			{
				if ( rCategory.GetItem( i )->GetId() == ms_iDevBoughtId )
					iTarget = i;
			}

			TINT iDistance = ( iTarget - rCategory.GetCurrentIndex() + rCategory.GetNumItems() ) % rCategory.GetNumItems();

			if ( iDistance <= rCategory.GetNumItems() / 2 )
				m_bRight = TTRUE;
			else
				m_bLeft = TTRUE;

			s_fTime = 11.6f;
			return;
		}

		s_iPhase = 3;
		TINFO( "[devgophershop] buying %d, money %d\n", m_pCurrentItem->GetId(), pProfile->GetDisplayedPickups() );
		Buy( m_pCurrentItem );
		TINFO( "[devgophershop] bought: count %d, purchases %d\n", m_pCurrentItem->GetCount(), m_iNumPurchases );
	}
	else if ( s_iPhase == 3 && s_fTime > 18.0f )
	{
		s_iPhase = 4;
		TINFO( "[devgophershop] leaving, money %d\n", pProfile->GetDisplayedPickups() );
		OnMainMenuItem( m_pMenu->GetButton( AGopherShopMenu::BUTTON_LEAVE ) );
	}

	fflush( stdout );
}

#endif
