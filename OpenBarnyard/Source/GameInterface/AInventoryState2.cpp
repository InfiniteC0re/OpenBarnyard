#include "pch.h"
#include "AInventoryState2.h"
#include "AGameStateController.h"
#include "AHintDisplayState.h"
#include "AMobilePhoneUIState2.h"
#include "AMapState.h"
#include "ARecipeDisplayState.h"
#include "AGamesPlayedDisplayState.h"
#include "GUI/AGUI2.h"
#include "Helpers/ABikeManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ATerrainSectionHudElement.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitFactory.h"
#include "Units/AObjectTraits.h"
#include "Units/AUnitModelStaticData.h"
#include "Units/AInventoryItem.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Math/TMathInline.h>
#include <Math/TMatrix44.h>
#include <Render/TCameraObject.h>
#include <Render/TViewport.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00434150, CreateObject 00435f60, CreateObjectInPlace 00435fc0 (the constructor is inlined in both)
TDEFINE_CLASS( AInventoryState2 );

TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( BackPack );
TPSTRING8_DECLARE( Knapsack );
TPSTRING8_DECLARE( FlowerPack );

// Places of the items on the sides of the selected one
static const TVector4 s_vecPreviousPosition = TVector4( -0.7f, 0.0f, 0.0f, 1.0f ); // 007734a8
static const TVector4 s_vecNextPosition     = TVector4( 0.7f, 0.0f, 0.0f, 1.0f );  // 007734b8

static constexpr TFLOAT SLIDE_SPEED = 5.0f; // 007734cc
static constexpr TFLOAT FADE_SPEED  = 8.0f; // 007734c8

// Colours of the models of the items
static const TVector4 s_vecLightColour         = TVector4( 1.0f, 1.0f, 1.0f, 1.0f ); // 00773498
static const TVector4 s_vecAmbientColour       = TVector4( 0.4f, 0.4f, 0.4f, 0.0f ); // 00773478
static const TVector4 s_vecFlowerAmbientColour = TVector4( 0.4f, 0.4f, 0.4f, 0.0f ); // 00773488

// The highest point of the pop up animation of the shelf
static const TFLOAT s_fMaxPopTime = TMath::PI - 0.5f; // 007819d8

// Camera of the model renderer
// NOTE: the x and y of the camera position (007819d4, 007819d0) are never written
static TFLOAT s_fCameraX        = 0.0f; // 007819d4
static TFLOAT s_fCameraY        = 0.0f; // 007819d0
static TFLOAT s_fCameraDistance = 1.5f; // 007734d0

// $Barnyard: FUNCTION 00434770
AInventoryState2::AInventoryState2()
    : m_vecShelfSize( TVector2::VEC_ZERO )
    , m_vecFrameSize( TVector2::VEC_ZERO )
    , m_fWoodMidWidth( 0.0f )
    , m_fWoodLeftWidth( 0.0f )
    , m_fWoodRightWidth( 0.0f )
    , m_fPopTime( 0.0f )
    , m_fFade( 0.0f )
    , m_fArrowFade( 0.0f )
    , m_eState( STATE_HIDDEN )
    , m_iCurrent( -1 )
    , m_iPrevious( -1 )
    , m_iNext( -1 )
    , m_iLeaving( -1 )
    , m_eScroll( SCROLL_NONE )
    , m_pCategory( TNULL )
    , m_pCurrentObject( TNULL )
    , m_pPreviousObject( TNULL )
    , m_pNextObject( TNULL )
    , m_pLeavingObject( TNULL )
    , m_iNumObjects( 0 )
    , m_iNumItems( 0 )
    , m_fStickTime( 0.0f )
    , m_bOpen( TFALSE )
    , m_fCloseTime( 0.0f )
    , m_iUnk84C( 0 )
    , m_iUnk850( 0 )
    , m_iUnk854( 0 )
    , m_iUnk858( 0 )
    , m_pPlayer( TNULL )
    , m_bMouseMoved( TFALSE )
    , m_bMouseOnOK( TFALSE )
    , m_bMouseOnLeftArrow( TFALSE )
    , m_bMouseOnRightArrow( TFALSE )
{
}

// $Barnyard: FUNCTION 00434990
// $Barnyard: FUNCTION 00436000
AInventoryState2::~AInventoryState2()
{
}

// $Barnyard: FUNCTION 00434230
TBOOL AInventoryState2::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( !m_bOpen )
		return AGameState::ProcessInput( a_pInputEvent );

	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		// The arrows under the mouse are checked in OnUpdate
		m_bMouseMoved = TTRUE;
		return TTRUE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 )
		return AGameState::ProcessInput( a_pInputEvent );

	// Clicking the arrows scrolls the items
	TBOOL bStopEvents;

	if ( m_bMouseOnOK == TTRUE )
		return ProcessCommand( AInputCommand_OK, a_pInputEvent, bStopEvents );

	if ( m_bMouseOnLeftArrow == TTRUE )
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );

	if ( m_bMouseOnRightArrow == TTRUE )
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );

	return TTRUE;
}

// $Barnyard: FUNCTION 00436020
TBOOL AInventoryState2::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( !m_bOpen )
		return TFALSE;

	if ( m_eState == STATE_IDLE )
	{
		if ( a_eInputCommand == AInputCommand_Move )
		{
			// The analog stick scrolls the items
			if ( m_fStickTime > 0.2f )
			{
				TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
				a_pInputEvent->GetMagnitudeFloat( 1 );

				if ( fX < -0.5f && ( m_iNumObjects >= 4 || m_iCurrent > 0 ) )
				{
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
					m_oLeftArrow.GetTransform().SetScale( 1.25f, 1.25f );
					m_eScroll = SCROLL_START_LEFT;
				}
				else if ( fX > 0.5f && ( m_iNumObjects >= 4 || m_iCurrent < m_iNumObjects - 1 ) )
				{
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
					m_oRightArrow.GetTransform().SetScale( 1.25f, 1.25f );
					m_eScroll = SCROLL_START_RIGHT;
				}

				if ( fX < -0.5f || fX > 0.5f )
					m_fStickTime = 0.0f;
			}

			return TTRUE;
		}

		if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
		{
			if ( a_eInputCommand == AInputCommand_Inventory )
			{
				// Close the inventory
				ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
				m_bOpen = TFALSE;
				SetOpen( TFALSE );
				m_fCloseTime = 0.0f;
				m_eState     = STATE_FADING_OUT;
				return TTRUE;
			}

			if ( m_iNumItems >= 1 )
			{
				switch ( a_eInputCommand )
				{
					case AInputCommand_Left:
						ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
						m_oLeftArrow.GetTransform().SetScale( 1.25f, 1.25f );
						a_pInputEvent->StartRepeat( 0.5f, 0.2f );

						if ( m_eScroll == SCROLL_LEFT )
						{
							// Turn back while sliding to the left
							TINT iLeaving = m_iLeaving;
							m_iLeaving    = m_iNext;
							m_iNext       = m_iCurrent;
							m_iCurrent    = m_iPrevious;
							m_iPrevious   = iLeaving;

							TVector4 vecLeavingPos = m_vecLeavingPos;
							m_vecLeavingPos        = m_vecNextPos;
							m_vecNextPos           = m_vecCurrentPos;
							m_vecCurrentPos        = m_vecPreviousPos;
							m_vecPreviousPos       = vecLeavingPos;

							// NOTE: an index of -1 reads m_pCategory in the original
							m_pCurrentObject  = m_apObjects[ m_iCurrent ];
							m_pPreviousObject = m_apObjects[ iLeaving ];
							m_pNextObject     = m_apObjects[ m_iNext ];
							m_pLeavingObject  = m_apObjects[ m_iLeaving ];

							UpdateItemText();
							m_eScroll = SCROLL_RIGHT;
						}
						else if ( m_iNumObjects >= 4 || m_iCurrent > 0 )
						{
							m_eScroll = SCROLL_START_LEFT;
						}

						return TTRUE;
					case AInputCommand_Right:
						ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
						m_oRightArrow.GetTransform().SetScale( 1.25f, 1.25f );
						a_pInputEvent->StartRepeat( 0.5f, 0.2f );

						if ( m_eScroll == SCROLL_RIGHT )
						{
							// Turn back while sliding to the right
							TINT iCurrent  = m_iCurrent;
							TINT iPrevious = m_iPrevious;
							TINT iLeaving  = m_iLeaving;
							m_iCurrent     = m_iNext;
							m_iLeaving     = iPrevious;
							m_iPrevious    = iCurrent;
							m_iNext        = iLeaving;

							TVector4 vecLeavingPos = m_vecLeavingPos;
							m_vecLeavingPos        = m_vecPreviousPos;
							m_vecPreviousPos       = m_vecCurrentPos;
							m_vecCurrentPos        = m_vecNextPos;
							m_vecNextPos           = vecLeavingPos;

							// NOTE: an index of -1 reads m_pCategory in the original
							m_pCurrentObject  = m_apObjects[ m_iCurrent ];
							m_pPreviousObject = m_apObjects[ m_iPrevious ];
							m_pNextObject     = m_apObjects[ iLeaving ];
							m_pLeavingObject  = m_apObjects[ m_iLeaving ];

							UpdateItemText();
							m_eScroll = SCROLL_LEFT;
						}
						else if ( m_iNumObjects >= 4 || m_iCurrent < m_iNumObjects - 1 )
						{
							m_eScroll = SCROLL_START_RIGHT;
						}

						return TTRUE;
					case AInputCommand_Interact:
					{
						if ( AGameStateController::IsBusy() )
						{
							ASoundManager::GetSingleton()->PlayCue( soundbank::UI_NEGATIVE );
							return TTRUE;
						}

						ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
						AInventoryItem* pItem = m_apItems[ m_iCurrent % m_iNumItems ];

						if ( !pItem )
							return TTRUE;

						// The bags open their categories
						if ( TPS8D( "flowerpack" ) == pItem->GetUnitType()->GetName() )
						{
							SetCategory( TPS8( FlowerPack ) );
							return TTRUE;
						}

						if ( TPS8D( "knapsack" ) == pItem->GetUnitType()->GetName() )
						{
							SetCategory( TPS8( Knapsack ) );
							return TTRUE;
						}

						// The other items open their screens or are taken out into the hand
						if ( TPS8D( "mobile" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new AMobilePhoneUIState2() );
						}
						else if ( TPS8D( "map" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new AMapState() );
						}
						else if ( TPS8D( "cookbook" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new ARecipeDisplayState( TFALSE ) );
						}
						else if ( TPS8D( "cocktails" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new ARecipeDisplayState( TTRUE ) );
						}
						else if ( TPS8D( "notebook" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new AHintDisplayState( TFALSE ) );
						}
						else if ( TPS8D( "scorebook" ) == pItem->GetUnitType()->GetName() )
						{
							AGameStateController::GetSingleton()->PushState( new AGamesPlayedDisplayState() );
						}
						else
						{
							AInventoryGroupComponent::TakeOutItem( pItem );
							UpdateItemText();
						}

						// Stop the sliding and close the inventory
						if ( m_eScroll == SCROLL_LEFT || m_eScroll == SCROLL_RIGHT )
						{
							m_vecCurrentPos = TVector4::VEC_ZERO;
							UpdateSlide( 0.0f, m_eScroll );
						}

						m_bOpen = TFALSE;
						SetOpen( TFALSE );
						m_fCloseTime = 0.0f;
						m_eState     = STATE_HIDING;
						m_eScroll    = SCROLL_NONE;
						return TTRUE;
					}
				}
			}
		}
	}

	if ( a_eInputCommand != AInputCommand_Quit )
		return TTRUE;

	// NOTE: the original calls RETURNS_FALSE (006528c0) here
	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 004381e0
TBOOL AInventoryState2::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseMoved == TTRUE )
	{
		// Find the arrow under the mouse
		T2GUITransform oInvTransform;
		TVector2       vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
		TVector2       vecLocalPos;

		m_bMouseOnOK = TFALSE;

		m_oLeftArrow.GetInvScreenTransform( oInvTransform );
		oInvTransform.Transform( vecLocalPos, vecCursorPos );
		m_bMouseOnLeftArrow = m_oLeftArrow.IsPointInside( vecLocalPos.x, vecLocalPos.y ) ? TTRUE : TFALSE;

		if ( m_bMouseOnOK == TFALSE && m_bMouseOnLeftArrow == TFALSE )
		{
			m_oRightArrow.GetInvScreenTransform( oInvTransform );
			oInvTransform.Transform( vecLocalPos, vecCursorPos );
			m_bMouseOnRightArrow = m_oRightArrow.IsPointInside( vecLocalPos.x, vecLocalPos.y ) != TFALSE;
		}

		m_bMouseMoved = TFALSE;
	}

	m_fStickTime += a_fDeltaTime;

	if ( m_eState == STATE_IDLE && !m_bOpen )
	{
		// The inventory has popped up on its own, hide it after a while
		m_fCloseTime += a_fDeltaTime;

		if ( m_fCloseTime > 2.0f )
			m_eState = STATE_FADING_OUT;
	}

	// The arrows are shown while there's something to scroll to
	m_oLeftArrow.SetVisible( ( m_iNumObjects >= 4 || m_iCurrent != 0 ) && m_iNumObjects != 0 );
	m_oRightArrow.SetVisible( ( m_iNumObjects >= 4 || m_iCurrent != m_iNumObjects - 1 ) && m_iNumObjects != 0 );

	switch ( m_eState )
	{
		case STATE_OPENING:
			m_fPopTime += a_fDeltaTime * 8.0f;

			if ( m_fPopTime > s_fMaxPopTime )
			{
				m_fPopTime   = s_fMaxPopTime;
				m_eState     = STATE_FADING_IN;
				m_fFade      = 0.0f;
				m_fArrowFade = 0.0f;
				UpdatePopScale( TTRUE );
				return AGameState::OnUpdate( a_fDeltaTime );
			}

			UpdatePopScale( TFALSE );
			return AGameState::OnUpdate( a_fDeltaTime );
		case STATE_CLOSING:
			m_fPopTime -= a_fDeltaTime * 8.0f;

			if ( m_fPopTime < 0.0f )
			{
				m_fPopTime = 0.0f;
				m_eState   = STATE_HIDDEN;
				Remove();
				return TTRUE;
			}

			UpdatePopScale( TFALSE );
			return AGameState::OnUpdate( a_fDeltaTime );
		case STATE_FADING_IN:
		{
			TFLOAT fDelta = a_fDeltaTime * 5.0f;
			m_fFade += fDelta;

			if ( m_fFade >= 1.0f )
			{
				m_fFade  = 1.0f;
				m_eState = m_bOpen ? STATE_SHOWING : STATE_IDLE;
			}

			TMath::Clip( m_fFade, 0.0f, 1.0f );
			UpdateFade( m_fFade, fDelta );

			if ( m_bOpen )
				UpdateArrowFade( fDelta );

			return AGameState::OnUpdate( a_fDeltaTime );
		}
		case STATE_SHOWING:
			UpdateArrowFade( a_fDeltaTime * 5.0f );
			return AGameState::OnUpdate( a_fDeltaTime );
		case STATE_HIDING:
			UpdateArrowFade( a_fDeltaTime * -5.0f );
			return AGameState::OnUpdate( a_fDeltaTime );
		case STATE_FADING_OUT:
		{
			m_fFade -= a_fDeltaTime * 5.0f;

			if ( m_fFade <= 0.0f )
			{
				m_fFade    = 0.0f;
				m_eState   = STATE_CLOSING;
				m_fPopTime = s_fMaxPopTime;
			}

			TMath::Clip( m_fFade, 0.0f, 1.0f );

			TFLOAT fDelta = a_fDeltaTime * -5.0f;
			UpdateFade( m_fFade, fDelta );
			UpdateArrowFade( fDelta );
			return AGameState::OnUpdate( a_fDeltaTime );
		}
		case STATE_IDLE:
		{
			m_fSpinTime += a_fDeltaTime;

			// The arrows under the mouse are bigger
			TFLOAT fRightScale = 1.0f;
			TFLOAT fLeftScale  = 1.0f;

			if ( m_bMouseOnRightArrow == TTRUE )
				fRightScale = 1.25f;

			if ( m_bMouseOnLeftArrow == TTRUE )
				fLeftScale = 1.25f;

			// The arrows shrink back after they were pressed
			TFLOAT          fT              = a_fDeltaTime * 10.0f;
			T2GUITransform& rRightTransform = m_oRightArrow.GetTransform();
			T2GUITransform& rLeftTransform  = m_oLeftArrow.GetTransform();
			rRightTransform.SetScale( TMath::LERP( rRightTransform.GetScaleX(), fRightScale, fT ), TMath::LERP( rRightTransform.GetScaleY(), fRightScale, fT ) );
			rLeftTransform.SetScale( TMath::LERP( rLeftTransform.GetScaleX(), fLeftScale, fT ), TMath::LERP( rLeftTransform.GetScaleY(), fLeftScale, fT ) );

			if ( m_iNumObjects > 0 )
				UpdateScroll( a_fDeltaTime );

			return AGameState::OnUpdate( a_fDeltaTime );
		}
		default:
			return AGameState::OnUpdate( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 00437940
void AInventoryState2::OnInsertion()
{
	SetInputMap( TPS8( NormalBarnyard ) );

	// The wood of the shelf
	T2GUITextureSection* pWoodMid   = T2GUITextureSectionManager::GetTextureSection( "Inv_Wood_Mid" );
	TFLOAT               fMidWidth  = pWoodMid->GetWidth() * 1.3f;
	TFLOAT               fMidHeight = pWoodMid->GetHeight() * 1.3f;
	m_oWoodMid.SetDimensions( fMidWidth, fMidHeight );
	m_oWoodMid.SetTextureSection( pWoodMid );
	m_oWoodMid.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_fWoodMidWidth = fMidWidth;

	TFLOAT fShelfWidth  = fMidWidth;
	TFLOAT fShelfHeight = fMidHeight;

	T2GUITextureSection* pWoodLeft  = T2GUITextureSectionManager::GetTextureSection( "Inv_Wood_Left" );
	TFLOAT               fLeftWidth = pWoodLeft->GetWidth() * 1.3f;
	m_oWoodLeft.SetDimensions( fLeftWidth, pWoodLeft->GetHeight() * 1.3f );
	m_oWoodLeft.SetTextureSection( pWoodLeft );
	m_oWoodLeft.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	fShelfWidth      = fLeftWidth + fShelfWidth;
	m_fWoodLeftWidth = fLeftWidth;
	m_oWoodLeft.Hide();

	T2GUITextureSection* pWoodRight   = T2GUITextureSectionManager::GetTextureSection( "Inv_Wood_Right" );
	TFLOAT               fRightWidth  = pWoodRight->GetWidth() * 1.3f;
	TFLOAT               fRightHeight = pWoodRight->GetHeight() * 1.3f;
	m_oWoodRight.SetDimensions( fRightWidth, fRightHeight );
	m_oWoodRight.SetTextureSection( pWoodRight );
	m_oWoodRight.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	fShelfWidth       = ( fRightWidth + fShelfWidth ) - 4.0f;
	m_fWoodRightWidth = fRightWidth;
	m_oWoodRight.Hide();

	m_oShelf.SetDimensions( fShelfWidth, fShelfHeight );
	m_oShelf.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oShelf.SetTransform( 0.0f, fRightHeight * 0.4f, 0.0f );
	m_oShelf.AddChildTail( m_oWoodLeft );
	m_oShelf.AddChildTail( m_oWoodRight );
	m_oShelf.AddChildTail( m_oWoodMid );

	// The models of the items
	m_oModelRenderer.Create( fShelfWidth, fShelfHeight, TFALSE, TTRUE );
	m_oModelRenderer.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );

	// The arrows
	T2GUITextureSection* pLeftArrow   = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	TFLOAT               fArrowHeight = fShelfHeight * 0.5f;
	TFLOAT               fArrowScale  = fArrowHeight / pLeftArrow->GetHeight();
	m_oLeftArrow.SetDimensions( pLeftArrow->GetWidth() * fArrowScale, fArrowScale * pLeftArrow->GetHeight() );
	m_oLeftArrow.SetTextureSection( pLeftArrow );
	m_oLeftArrow.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

	TFLOAT fOffsetY = fShelfHeight * -0.05f;
	m_oLeftArrow.SetTransform( fShelfWidth * -0.05f, fOffsetY, 0.0f );
	m_oLeftArrow.SetAlpha( 0.0f );

	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	fArrowScale                      = fArrowHeight / pRightArrow->GetHeight();
	m_oRightArrow.SetDimensions( pRightArrow->GetWidth() * fArrowScale, fArrowScale * pRightArrow->GetHeight() );
	m_oRightArrow.SetTextureSection( pRightArrow );
	m_oRightArrow.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oRightArrow.SetTransform( 0.0f, fOffsetY, 0.0f );
	m_oRightArrow.SetAlpha( 0.0f );

	// The frame with the name of the category, the name of the item and its count
	T2GUITextureSection* pFrame      = T2GUITextureSectionManager::GetTextureSection( "Inv_Frame" );
	TFLOAT               fFrameScale = fShelfHeight / pFrame->GetHeight() * 1.3f;
	m_oFrame.SetDimensions( pFrame->GetWidth() * fFrameScale, fFrameScale * pFrame->GetHeight() );
	m_oFrame.SetTextureSection( pFrame );

	T2GUIFont* pFont       = T2GUIFontManager::FindFont( "Rekord26" );
	TFLOAT     fTextHeight = fShelfHeight * 0.2f;
	TFLOAT     fTextScale  = fTextHeight / pFont->GetLineHeight();
	m_oTitle.Create( pFont, fShelfWidth );
	m_oTitle.SetScale( fTextScale );
	m_oTitle.SetText( ALocaleManager::Interface()->GetString( 356 ) );
	m_oTitle.SetColour( 0xFFFFF000 );
	m_oTitle.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oTitle.SetTransform( 0.0f, fOffsetY, 0.0f );

	pFont      = T2GUIFontManager::FindFont( "Rekord26" );
	fTextScale = fTextHeight / pFont->GetLineHeight();
	m_oItemName.Create( pFont, fShelfWidth );
	m_oItemName.SetScale( fTextScale );
	m_oItemName.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oItemName.SetTransform( 0.0f, fOffsetY, 0.0f );

	pFont      = T2GUIFontManager::FindFont( "Rekord26" );
	fTextScale = fTextHeight / pFont->GetLineHeight();
	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"" );
	m_oItemCount.Create( pFont, m_oFrame.GetWidth() * 0.5f );
	m_oItemCount.SetScale( fTextScale );
	m_oItemCount.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oItemCount.SetText( m_wszCount );
	m_oItemCount.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oItemCount.SetTransform( m_oFrame.GetWidth() * 0.17f, m_oFrame.GetHeight() * -0.1f, 0.0f );

	m_oFrame.AddChildTail( m_oTitle );
	m_oFrame.AddChildTail( m_oItemCount );
	m_oFrame.AddChildTail( m_oItemName );
	m_oShelf.AddChildTail( m_oModelRenderer );
	m_oShelf.AddChildTail( m_oLeftArrow );
	m_oShelf.AddChildTail( m_oRightArrow );
	m_oShelf.AddChildTail( m_oFrame );
	AGUI2::GetRootElement()->AddChildTail( m_oShelf );

	m_oModelRenderer.UpdateViewport();

	TFLOAT fHeight   = m_oShelf.GetHeight();
	m_vecShelfSize.x = m_oShelf.GetWidth();
	m_vecShelfSize.y = fHeight;

	fHeight          = m_oFrame.GetHeight();
	m_vecFrameSize.x = m_oFrame.GetWidth();
	m_vecFrameSize.y = fHeight;

	UpdatePopScale( TFALSE );
	m_eState = STATE_HIDDEN;

	// The camera looks at the items from the front
	m_oModelRenderer.GetCamera()->SetFOV( TMath::PI / 3.0f );

	TMatrix44 matCamera;
	matCamera.Identity();
	matCamera.GetTranslation() = TVector4( s_fCameraX, s_fCameraY, -s_fCameraDistance, 1.0f );
	m_oModelRenderer.SetCameraMatrix( matCamera );

	// Select the item that was selected last time
	SetCategory( TPS8( BackPack ) );
	SelectType( AUnitManager::GetSingleton()->GetUnitFactory()->Find( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetInventoryItem() ), TFALSE );

	// The prompts at the bottom of the screen
	pFont = T2GUIFontManager::FindFont( "Rekord26" );
	ALocale::FormatInputTags( m_wszUsePrompt, TARRAYSIZE( m_wszUsePrompt ), ALocaleManager::Interface()->GetString( 1047 ) );

	TWCHAR wszClosePrompt[ 64 ];
	TStringManager::String16Format( wszClosePrompt, TARRAYSIZE( wszClosePrompt ), L"%s  %s", L"%c_NormalBarnyard[Inventory]", ALocaleManager::Interface()->GetString( 1212 ) );
	ALocale::FormatInputTags( m_wszClosePrompt, TARRAYSIZE( m_wszClosePrompt ), wszClosePrompt );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	m_oPromptBar.SetDimensions( pRoot->GetWidth(), pFont->GetLineHeight() * 1.5f );
	m_oPromptBar.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oPromptBar );

	TFLOAT fPromptOffset = m_oPromptBar.GetWidth() * 0.25f;

	m_aPrompts[ 0 ].Create( pFont, 0.0f );
	m_aPrompts[ 0 ].SetTextAndFitWidth( m_wszUsePrompt );
	m_aPrompts[ 0 ].SetTransform( -fPromptOffset, 0.0f, 0.0f );
	m_aPrompts[ 0 ].SetScale( 0.75f );

	m_aPrompts[ 1 ].Create( pFont, 0.0f );
	m_aPrompts[ 1 ].SetTextAndFitWidth( m_wszClosePrompt );
	m_aPrompts[ 1 ].SetTransform( fPromptOffset, 0.0f, 0.0f );
	m_aPrompts[ 1 ].SetScale( 0.75f );

	m_oPromptBar.AddChildTail( m_aPrompts[ 0 ] );
	m_oPromptBar.AddChildTail( m_aPrompts[ 1 ] );
	m_oPromptBar.Hide();

	AGameState::OnInsertion();
	Show();
}

// $Barnyard: FUNCTION 00434160
void AInventoryState2::OnRemoval()
{
	m_oModelRenderer.DestroyObjects();

	m_oItemCount.RemoveSelf();
	m_oItemName.RemoveSelf();
	m_oTitle.RemoveSelf();
	m_oLeftArrow.RemoveSelf();
	m_oRightArrow.RemoveSelf();
	m_oModelRenderer.RemoveSelf();
	m_oFrame.RemoveSelf();
	m_oWoodMid.RemoveSelf();
	m_oWoodLeft.RemoveSelf();
	m_oWoodRight.RemoveSelf();
	m_oShelf.RemoveSelf();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original leaves the prompts on the root element of the GUI
	m_oPromptBar.RemoveSelf();
#endif

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 004344f0
void AInventoryState2::OnActivate()
{
	Show();
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00434500
void AInventoryState2::OnDeactivate()
{
	m_oShelf.Hide();

	ATerrainSectionHudElement* pTerrainSectionHud = AHudManager::GetSingleton()->FindElement<ATerrainSectionHudElement>();

	if ( pTerrainSectionHud )
		pTerrainSectionHud->SetForceHide( TFALSE );

	m_oPromptBar.Hide();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00434480
void AInventoryState2::Show()
{
	m_oShelf.Show();

	if ( m_eState == STATE_HIDDEN )
	{
		m_eState    = STATE_OPENING;
		m_fSpinTime = 0.0f;
	}

	// The name of the area would be over the shelf
	ATerrainSectionHudElement* pTerrainSectionHud = AHudManager::GetSingleton()->FindElement<ATerrainSectionHudElement>();

	if ( pTerrainSectionHud )
		pTerrainSectionHud->SetForceHide( TTRUE );

	m_oPromptBar.SetVisible( m_bOpen == TTRUE );
}

// $Barnyard: FUNCTION 00434710
void AInventoryState2::Open()
{
	m_bOpen = TTRUE;
	SetOpen( TTRUE );

	m_oWoodLeft.Show();
	m_oWoodRight.Show();

	switch ( m_eState )
	{
		case STATE_IDLE:
		case STATE_HIDING:
			m_eState = STATE_SHOWING;
			break;
		case STATE_CLOSING:
			m_eState = STATE_OPENING;
			break;
		case STATE_FADING_OUT:
			m_eState = STATE_FADING_IN;
			break;
	}
}

// $Barnyard: FUNCTION 004346a0
void AInventoryState2::SetOpen( TBOOL a_bOpen )
{
	APlayerControlComponent* pControlComponent = TSTATICCAST( APlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

	if ( pControlComponent )
	{
		// The action button uses the item instead
		pControlComponent->SetOverrideText( TNULL );
		pControlComponent->SetContextEnabled( !a_bOpen );

		if ( a_bOpen )
			pControlComponent->SetOverrideText( TNULL );
	}

	m_oPromptBar.SetVisible( a_bOpen == TTRUE );
}

// $Barnyard: FUNCTION 00434540
void AInventoryState2::UpdateItemText()
{
	AInventoryItem*           pItem      = m_apItems[ m_iCurrent ];
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	// The slot can be gone if the items were used
	if ( !pInventory->ContainsItem( pItem ) )
	{
		m_apItems[ m_iCurrent ] = pInventory->FindItemByType( m_apTypes[ m_iCurrent ]->GetName() );
		pItem                   = m_apItems[ m_iCurrent ];
	}

	if ( !pItem )
	{
		pItem = m_pCategory->FindItemByType( m_apTypes[ m_iCurrent ]->GetName() );

		if ( !pItem )
		{
			m_oItemName.SetText( TNULL );
			TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"%d", 0 );
			return;
		}

		m_apItems[ m_iCurrent ] = pItem;
	}

	m_oItemName.SetText( pItem->GetScreenName() );

	// The count isn't shown for the items the player has only one of (phone, map, books...)
	TINT iScreenNameId = pItem->GetUnit()->GetType()->GetScreenNameId();

	if ( iScreenNameId == 453 || iScreenNameId == 447 || iScreenNameId == 448 || iScreenNameId == 1856 ||
	     iScreenNameId == 365 || iScreenNameId == 452 || iScreenNameId == 450 || iScreenNameId == 451 )
	{
		m_oItemCount.Hide();
	}
	else
	{
		m_oItemCount.Show();
	}

	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"%d", pItem->GetCount() );
}

// $Barnyard: FUNCTION 00434420
void AInventoryState2::GetItemScreenPosition( TVector2& a_rOutPosition )
{
	TViewport* pViewport = m_oModelRenderer.GetViewport();
	a_rOutPosition.x     = pViewport->GetX() + pViewport->GetWidth() * 0.5f;
	a_rOutPosition.y     = pViewport->GetY() + pViewport->GetHeight() * 0.25f;
}

// $Barnyard: FUNCTION 004342d0
void AInventoryState2::UpdatePopScale( TBOOL a_bFull )
{
	TFLOAT fScaleX = 1.0f;
	TFLOAT fScaleY = 1.0f;

	if ( !a_bFull )
	{
		TFLOAT fPop = ( TMath::Sin( m_fPopTime ) - 0.5f ) * m_vecShelfSize.x * 0.5f;
		fScaleX     = ( fPop + m_vecShelfSize.x ) / m_vecShelfSize.x;
		fScaleY     = ( fPop + m_vecShelfSize.y ) / m_vecShelfSize.y;
	}

	m_oShelf.SetDimensions( fScaleX * m_vecShelfSize.x, fScaleY * m_vecShelfSize.y );
	m_oWoodMid.SetDimensions( fScaleX * m_fWoodMidWidth, fScaleY * m_vecShelfSize.y );
	m_oWoodRight.SetDimensions( fScaleX * m_fWoodRightWidth, fScaleY * m_vecShelfSize.y );
	m_oWoodLeft.SetDimensions( fScaleX * m_fWoodLeftWidth, fScaleY * m_vecShelfSize.y );
	m_oFrame.SetDimensions( fScaleX * m_vecFrameSize.x, fScaleY * m_vecFrameSize.y );
}

// $Barnyard: FUNCTION 00434ae0
void AInventoryState2::UpdateFade( TFLOAT a_fAlpha, TFLOAT a_fDelta )
{
	m_oTitle.SetAlpha( a_fAlpha );
	m_oItemName.SetAlpha( a_fAlpha );
	m_oItemCount.SetAlpha( a_fAlpha );

	if ( a_fDelta != 0.0f && m_iNumObjects > 0 )
	{
		TFLOAT fOpacity = m_pCurrentObject->GetModelInstance()->GetOpacity() + a_fDelta;
		TMath::Clip( fOpacity, 0.0f, 1.0f );
		m_oModelRenderer.SetObjectOpacity( m_iCurrent, fOpacity );
	}
}

// $Barnyard: FUNCTION 00434be0
void AInventoryState2::UpdateArrowFade( TFLOAT a_fDelta )
{
	m_fArrowFade += a_fDelta;

	if ( m_fArrowFade < 1.0f )
	{
		if ( m_fArrowFade <= 0.0f && m_eState == STATE_HIDING )
		{
			m_oWoodLeft.Hide();
			m_oWoodRight.Hide();
			m_eState   = STATE_IDLE;
			m_fPopTime = 0.0f;
		}
	}
	else if ( m_eState == STATE_SHOWING )
	{
		m_eState   = STATE_IDLE;
		m_fPopTime = 0.0f;
	}

	// NOTE: the original clamps m_fFade here, not m_fArrowFade
	TMath::Clip( m_fFade, 0.0f, 1.0f );

	TFLOAT fAlpha = m_oLeftArrow.GetAlpha() + a_fDelta;
	TMath::Clip( fAlpha, 0.0f, 1.0f );
	m_oLeftArrow.SetAlpha( fAlpha );

	fAlpha = m_oRightArrow.GetAlpha() + a_fDelta;
	TMath::Clip( fAlpha, 0.0f, 1.0f );
	m_oRightArrow.SetAlpha( fAlpha );

	if ( a_fDelta != 0.0f && m_iNumObjects > 0 )
	{
		// The items on the sides are barely visible
		if ( m_iPrevious != -1 )
		{
			TFLOAT fOpacity = m_pPreviousObject->GetModelInstance()->GetOpacity() + a_fDelta;
			TMath::Clip( fOpacity, 0.0f, 0.1f );
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, fOpacity );
		}

		if ( m_iNext != -1 )
		{
			TFLOAT fOpacity = m_pNextObject->GetModelInstance()->GetOpacity() + a_fDelta;
			TMath::Clip( fOpacity, 0.0f, 0.1f );
			m_oModelRenderer.SetObjectOpacity( m_iNext, fOpacity );
		}

		if ( m_iLeaving != -1 && a_fDelta < 0.0f )
		{
			TFLOAT fOpacity = m_pLeavingObject->GetModelInstance()->GetOpacity() + a_fDelta;
			TMath::Clip( fOpacity, 0.0f, 1.0f );
			m_oModelRenderer.SetObjectOpacity( m_iLeaving, fOpacity );
		}
	}
}

// $Barnyard: FUNCTION 00434ed0
void AInventoryState2::SetObjectTransform( AAnimatableObject* a_pObject, AObjectTraits* a_pTraits, TBOOL a_bSetPosition, TVector4 a_vecPosition, TVector4 a_vecRotation, TFLOAT a_fScale )
{
	TMatrix44 matTransform = TMatrix44::IDENTITY;
	matTransform.RotateX( TMath::HALF_PI );
	matTransform.Rotate( a_vecRotation.x, a_vecRotation.y, a_vecRotation.z );
	matTransform.Rotate( a_pTraits->m_vecHoldRotate.x, a_pTraits->m_vecHoldRotate.y, a_pTraits->m_vecHoldRotate.z );

	a_fScale = a_pTraits->m_fItemScale * a_fScale;
	matTransform.Scale( a_fScale, a_fScale, a_fScale );

	if ( a_bSetPosition )
		matTransform.GetTranslation() = a_vecPosition;

	TMatrix44::TransformVector( matTransform.GetTranslation(), matTransform, a_pTraits->m_vecItemOffset );

	// Center the model around its bounding sphere
	TVector4 vecCenter = a_pObject->GetModelInstance()->GetSceneObject()->GetInstance()->GetModel()->GetLOD( 0 ).BoundingSphere.AsVector4();
	vecCenter.x        = -vecCenter.x;
	vecCenter.y        = -vecCenter.y;
	vecCenter.z        = -vecCenter.z;
	TMatrix44::TransformVector( matTransform.GetTranslation3(), matTransform, vecCenter.AsVector3() );

	a_pObject->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 00435090
void AInventoryState2::UpdateSlide( TFLOAT a_fDeltaTime, SCROLL a_eScroll )
{
	// Remember the selected item of the "BackPack"
	if ( m_strCategory == TPS8( BackPack ) )
		APlayerProfileManager::GetSingleton()->GetMainProfile()->SetInventoryItem( m_apTypes[ m_iCurrent ]->GetName() );

	if ( TMath::Abs( m_vecCurrentPos.x ) > 0.01f )
	{
		if ( m_iPrevious != -1 )
		{
			m_vecPreviousPos.x = TMath::LERPClamped( m_vecPreviousPos.x, s_vecPreviousPosition.x, SLIDE_SPEED * a_fDeltaTime );
			TFLOAT fOpacity    = TMath::LERPClamped( m_pPreviousObject->GetModelInstance()->GetOpacity(), 0.1f, FADE_SPEED * a_fDeltaTime );
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, fOpacity );
		}

		if ( m_iNext != -1 )
		{
			m_vecNextPos.x  = TMath::LERPClamped( m_vecNextPos.x, s_vecNextPosition.x, SLIDE_SPEED * a_fDeltaTime );
			TFLOAT fOpacity = TMath::LERPClamped( m_pNextObject->GetModelInstance()->GetOpacity(), 0.1f, FADE_SPEED * a_fDeltaTime );
			m_oModelRenderer.SetObjectOpacity( m_iNext, fOpacity );
		}

		if ( m_iLeaving != -1 )
		{
			// The leaving item slides out of the shelf
			if ( a_eScroll == SCROLL_LEFT )
				m_vecLeavingPos.x = TMath::LERPClamped( m_vecLeavingPos.x, s_vecPreviousPosition.x + s_vecPreviousPosition.x, SLIDE_SPEED * a_fDeltaTime );
			else
				m_vecLeavingPos.x = TMath::LERPClamped( m_vecLeavingPos.x, s_vecNextPosition.x + s_vecNextPosition.x, SLIDE_SPEED * a_fDeltaTime );

			TFLOAT fOpacity = TMath::LERPClamped( m_pLeavingObject->GetModelInstance()->GetOpacity(), 0.0f, FADE_SPEED * a_fDeltaTime );
			m_oModelRenderer.SetObjectOpacity( m_iLeaving, fOpacity );
		}

		m_vecCurrentPos.x = TMath::LERPClamped( m_vecCurrentPos.x, 0.0f, SLIDE_SPEED * a_fDeltaTime );
		TFLOAT fOpacity   = TMath::LERPClamped( m_pCurrentObject->GetModelInstance()->GetOpacity(), 1.0f, FADE_SPEED * a_fDeltaTime );
		m_oModelRenderer.SetObjectOpacity( m_iCurrent, fOpacity );

		SetObjectTransform( m_pCurrentObject, m_apTraits[ m_iCurrent ], TTRUE, m_vecCurrentPos, TVector4::VEC_ZERO, 1.0f );

		if ( m_iPrevious != -1 )
			SetObjectTransform( m_pPreviousObject, m_apTraits[ m_iPrevious ], TTRUE, m_vecPreviousPos, TVector4::VEC_ZERO, 1.0f );

		if ( m_iNext != -1 )
			SetObjectTransform( m_pNextObject, m_apTraits[ m_iNext ], TTRUE, m_vecNextPos, TVector4::VEC_ZERO, 1.0f );

		if ( m_iLeaving != -1 )
			SetObjectTransform( m_pLeavingObject, m_apTraits[ m_iLeaving ], TTRUE, m_vecLeavingPos, TVector4::VEC_ZERO, 1.0f );
	}
	else
	{
		// The items have reached their places
		if ( m_iPrevious != -1 )
		{
			m_vecPreviousPos = s_vecPreviousPosition;
			SetObjectTransform( m_pPreviousObject, m_apTraits[ m_iPrevious ], TTRUE, m_vecPreviousPos, TVector4::VEC_ZERO, 1.0f );
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, 0.1f );
		}

		if ( m_iNext != -1 )
		{
			m_vecNextPos = s_vecNextPosition;
			SetObjectTransform( m_pNextObject, m_apTraits[ m_iNext ], TTRUE, m_vecNextPos, TVector4::VEC_ZERO, 1.0f );
			m_oModelRenderer.SetObjectOpacity( m_iNext, 0.1f );
		}

		if ( m_iLeaving != -1 )
		{
			m_vecLeavingPos = ( a_eScroll == SCROLL_LEFT ) ? s_vecPreviousPosition : s_vecNextPosition;
			m_vecLeavingPos.x += m_vecLeavingPos.x;
			SetObjectTransform( m_pLeavingObject, m_apTraits[ m_iLeaving ], TTRUE, m_vecLeavingPos, TVector4::VEC_ZERO, 1.0f );
			m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.0f );
			m_oModelRenderer.SetObjectVisible( m_iLeaving, TFALSE );
		}

		m_vecCurrentPos = TVector4::VEC_ZERO;
		SetObjectTransform( m_pCurrentObject, m_apTraits[ m_iCurrent ], TTRUE, m_vecCurrentPos, TVector4::VEC_ZERO, 1.0f );
		m_pCurrentObject->GetModelInstance()->SetOpacity( 1.0f );

		m_fSpinTime      = 0.0f;
		m_pLeavingObject = TNULL;
		m_iLeaving       = -1;
		m_eScroll        = SCROLL_NONE;
	}
}

// $Barnyard: FUNCTION 00435930
void AInventoryState2::CreateItemObject( AUnitType* a_pType, TINT a_iIndex )
{
	m_apTypes[ a_iIndex ]  = a_pType;
	m_apTraits[ a_iIndex ] = TSTATICCAST( AObjectTraits, a_pType->FindStaticData( &TGetClass( AObjectTraits ) ) );

	AUnitModelStaticData*     pModelData     = TSTATICCAST( AUnitModelStaticData, a_pType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) );
	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
	m_apObjects[ a_iIndex ]                  = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( pModelData->GetModelName( 0 ) ), TNULL, 0 );

	m_apObjects[ a_iIndex ]->GetModelInstance()->SetUnlit();
	m_apObjects[ a_iIndex ]->GetModelInstance()->DisableDrawWithDefaultMaterials();

	if ( m_apTraits[ a_iIndex ]->m_strInventoryForStore == TPS8( FlowerPack ) )
		m_oModelRenderer.AddObject( m_apObjects[ a_iIndex ], &s_vecLightColour, &s_vecFlowerAmbientColour );
	else
		m_oModelRenderer.AddObject( m_apObjects[ a_iIndex ], &s_vecLightColour, &s_vecAmbientColour );

	m_oModelRenderer.SetObjectOpacity( a_iIndex, 0.0f );
	SetObjectTransform( m_apObjects[ a_iIndex ], m_apTraits[ a_iIndex ], TTRUE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );
}

// $Barnyard: FUNCTION 00435b10
void AInventoryState2::SetCategory( const TPString8& a_rcName )
{
	if ( a_rcName == m_strCategory )
		return;

	if ( !m_strCategory.IsEmpty() )
		m_oModelRenderer.DestroyObjects();

	m_strCategory = a_rcName;

	if ( a_rcName == TPS8( BackPack ) )
		m_oTitle.SetText( ALocaleManager::Interface()->GetString( 356 ) );
	else if ( a_rcName == TPS8( Knapsack ) )
		m_oTitle.SetText( ALocaleManager::Interface()->GetString( 360 ) );
	else if ( a_rcName == TPS8( FlowerPack ) )
		m_oTitle.SetText( ALocaleManager::Interface()->GetString( 359 ) );

	m_pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	// While the player rides the bike the inventory belongs to the rider
	// NOTE: retail doesn't check if ABikeManager exists
	if ( ABikeManager::IsSingletonCreated() && ABikeManager::GetSingleton()->IsPlayerOnBike() )
		m_pPlayer = ABikeManager::GetSingleton()->GetRider();

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	m_pCategory                          = pInventory->GetCategory( a_rcName );
	m_iCurrent                           = 1;
	m_iNumItems                          = m_pCategory->GetNumSlots();

	for ( TINT i = 0; i < m_pCategory->GetNumSlots(); i++ )
		m_apItems[ i ] = m_pCategory->GetItem( i );

	m_oItemName.Show();
	m_oItemCount.Show();

	if ( m_iNumItems == 0 )
	{
		m_iNumObjects = 0;
		m_iCurrent    = -1;
		m_iPrevious   = -1;
		m_iNext       = -1;
		m_iLeaving    = -1;
		m_oItemName.Hide();
		m_oItemCount.Hide();
	}
	else
	{
		if ( m_iNumItems == 1 )
		{
			m_iNumObjects = 1;
			m_iCurrent    = 0;
			m_iPrevious   = -1;
			m_iNext       = -1;
		}
		else if ( m_iNumItems == 2 )
		{
			m_iCurrent    = 1;
			m_iNumObjects = 2;
			m_iPrevious   = 0;
			m_iNext       = -1;
		}
		else
		{
			m_iNumObjects = m_iNumItems;
			m_iPrevious   = m_iCurrent - 1;

			if ( m_iCurrent - 1 < 0 )
				m_iPrevious = m_iNumItems - 1;

			m_iNext = m_iCurrent + 1;

			if ( m_iCurrent + 1 >= m_iNumItems )
				m_iNext = 0;
		}

		m_iLeaving = -1;
	}

	m_pLeavingObject = TNULL;

	for ( TINT i = 0; i < m_iNumObjects; i++ )
		CreateItemObject( m_apItems[ i ]->GetUnit()->GetType(), i );

	if ( m_iPrevious != -1 )
	{
		m_pPreviousObject = m_apObjects[ m_iPrevious ];
		m_oModelRenderer.SetObjectVisible( m_iPrevious, TTRUE );
		m_vecPreviousPos = s_vecPreviousPosition;
		SetObjectTransform( m_pPreviousObject, m_apTraits[ m_iPrevious ], TTRUE, m_vecPreviousPos, TVector4::VEC_ZERO, 1.0f );
	}

	if ( m_iNext != -1 )
	{
		m_pNextObject = m_apObjects[ m_iNext ];
		m_oModelRenderer.SetObjectVisible( m_iNext, TTRUE );
		m_vecNextPos = s_vecNextPosition;
		SetObjectTransform( m_pNextObject, m_apTraits[ m_iNext ], TTRUE, m_vecNextPos, TVector4::VEC_ZERO, 1.0f );
	}

	if ( m_iCurrent != -1 )
	{
		m_pCurrentObject = m_apObjects[ m_iCurrent ];
		m_oModelRenderer.SetObjectVisible( m_iCurrent, TTRUE );
		UpdateItemText();
	}

	m_fFade = 0.0f;
	UpdateFade( 0.0f, 0.0f );

	if ( m_eState != STATE_OPENING && m_eState != STATE_HIDDEN )
	{
		m_eState  = STATE_FADING_IN;
		m_eScroll = SCROLL_NONE;
	}
}

// $Barnyard: FUNCTION 00436a10
void AInventoryState2::UpdateScroll( TFLOAT a_fDeltaTime )
{
	switch ( m_eScroll )
	{
		case SCROLL_NONE:
			// The selected item spins
			SetObjectTransform( m_apObjects[ m_iCurrent ], m_apTraits[ m_iCurrent ], TFALSE, TVector4::VEC_ZERO, TVector4( TVector4::VEC_ZERO.x, TVector4::VEC_ZERO.y, TMath::HALF_PI * m_fSpinTime, TVector4::VEC_ZERO.w ), 1.0f );
			return;
		case SCROLL_START_RIGHT:
		{
			if ( m_iLeaving != -1 )
			{
				m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.0f );
				m_oModelRenderer.SetObjectVisible( m_iLeaving, TFALSE );
			}

			SetObjectTransform( m_apObjects[ m_iCurrent ], m_apTraits[ m_iCurrent ], TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );

			// Everything moves one item to the left
			TINT iCurrent = m_iCurrent;
			m_iLeaving    = m_iPrevious;
			m_iCurrent    = m_iNext;
			m_iPrevious   = iCurrent;
			m_iNext       = m_iNext + 1;

			if ( m_iNumObjects >= 4 )
			{
				if ( m_iNext >= m_iNumObjects )
					m_iNext = 0;
			}
			else if ( m_iNext >= m_iNumObjects )
			{
				m_iNext       = -1;
				m_pNextObject = TNULL;
			}

			if ( m_iNext != -1 )
			{
				// The new item comes in from the right
				m_pNextObject = m_apObjects[ m_iNext ];
				m_oModelRenderer.SetObjectVisible( m_iNext, TTRUE );
				m_vecNextPos = s_vecNextPosition;
				m_vecNextPos.x += m_vecNextPos.x;
				SetObjectTransform( m_pNextObject, m_apTraits[ m_iNext ], TTRUE, m_vecNextPos, TVector4::VEC_ZERO, 1.0f );
				m_oModelRenderer.SetObjectOpacity( m_iNext, 0.0f );
			}

			if ( m_iLeaving != -1 )
			{
				m_pLeavingObject = m_apObjects[ m_iLeaving ];
				m_vecLeavingPos  = s_vecPreviousPosition;
				m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.1f );
			}

			m_pCurrentObject  = m_apObjects[ m_iCurrent ];
			m_vecCurrentPos   = s_vecNextPosition;
			m_pPreviousObject = m_apObjects[ m_iPrevious ];
			m_vecPreviousPos  = TVector4::VEC_ZERO;
			m_oModelRenderer.SetObjectOpacity( m_iCurrent, 0.1f );
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, 1.0f );
			UpdateItemText();
			m_eScroll = SCROLL_LEFT;
			return;
		}
		case SCROLL_LEFT:
			UpdateSlide( a_fDeltaTime, SCROLL_LEFT );
			return;
		case SCROLL_START_LEFT:
		{
			if ( m_iLeaving != -1 )
			{
				m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.0f );
				m_oModelRenderer.SetObjectVisible( m_iLeaving, TFALSE );
			}

			SetObjectTransform( m_pCurrentObject, m_apTraits[ m_iCurrent ], TFALSE, TVector4::VEC_ZERO, TVector4::VEC_ZERO, 1.0f );

			// Everything moves one item to the right
			TINT iCurrent = m_iCurrent;
			m_iLeaving    = m_iNext;
			m_iNext       = iCurrent;
			m_iCurrent    = m_iPrevious;
			m_iPrevious   = m_iPrevious - 1;

			if ( m_iNumObjects >= 4 )
			{
				if ( m_iPrevious < 0 )
					m_iPrevious = m_iNumObjects - 1;
			}
			else if ( m_iPrevious < 0 )
			{
				m_iPrevious       = -1;
				m_pPreviousObject = TNULL;
			}

			if ( m_iPrevious != -1 )
			{
				// The new item comes in from the left
				m_oModelRenderer.SetObjectVisible( m_iPrevious, TTRUE );
				m_pPreviousObject = m_apObjects[ m_iPrevious ];
				m_vecPreviousPos  = s_vecPreviousPosition;
				m_vecPreviousPos.x += m_vecPreviousPos.x;
				SetObjectTransform( m_pPreviousObject, m_apTraits[ m_iPrevious ], TTRUE, m_vecPreviousPos, TVector4::VEC_ZERO, 1.0f );
				m_oModelRenderer.SetObjectOpacity( m_iPrevious, 0.0f );
			}

			m_pCurrentObject = m_apObjects[ m_iCurrent ];
			m_vecCurrentPos  = s_vecPreviousPosition;
			m_pNextObject    = m_apObjects[ m_iNext ];
			m_vecNextPos     = TVector4::VEC_ZERO;

			if ( m_iLeaving != -1 )
			{
				m_pLeavingObject = m_apObjects[ m_iLeaving ];
				m_vecLeavingPos  = s_vecNextPosition;
				m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.1f );
			}

			m_oModelRenderer.SetObjectOpacity( m_iCurrent, 0.1f );
			m_oModelRenderer.SetObjectOpacity( m_iNext, 1.0f );
			UpdateItemText();
			m_eScroll = SCROLL_RIGHT;
			return;
		}
		case SCROLL_RIGHT:
			UpdateSlide( a_fDeltaTime, SCROLL_RIGHT );
			return;
	}
}

// $Barnyard: FUNCTION 004370d0
void AInventoryState2::SelectType( AUnitType* a_pType, TBOOL a_bAdd )
{
	if ( !a_pType )
		return;

	TINT           iIndex  = -1;
	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pType->FindStaticData( &TGetClass( AObjectTraits ) ) );

	// Show the category of the item
	if ( pTraits->m_strInventoryForStore != m_strCategory )
	{
		SetCategory( pTraits->m_strInventoryForStore );
	}
	else if ( m_strCategory == TPS8( FlowerPack ) )
	{
		// Recreate the flowers
		SetCategory( TPS8( BackPack ) );
		SetCategory( TPS8( FlowerPack ) );
	}

	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		if ( m_apTypes[ i ] == a_pType )
		{
			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, m_pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

			if ( !pInventory->ContainsItem( m_apItems[ i ] ) )
				m_apItems[ i ] = pInventory->FindItemByType( a_pType->GetName() );

			iIndex = i;
			break;
		}
	}

	TINT iNumItems = m_iNumItems;

	if ( iIndex == -1 )
	{
		if ( !a_bAdd )
			return;

		// The item has just been added to the inventory
		iIndex              = m_iNumItems;
		m_apItems[ iIndex ] = TNULL;
		CreateItemObject( a_pType, iIndex );

		m_iNumItems++;
		m_iNumObjects++;

		if ( m_iNumItems == 1 )
		{
			m_oItemName.Show();
			m_oItemCount.Show();
		}
	}

	if ( iIndex != m_iCurrent )
	{
		// Keep the opacities of the items
		TFLOAT fCurrentOpacity  = 0.0f;
		TFLOAT fPreviousOpacity = 0.0f;
		TFLOAT fNextOpacity     = 0.0f;
		TFLOAT fLeavingOpacity  = 0.0f;

		if ( m_iCurrent != -1 )
		{
			m_vecCurrentPos = TVector4::VEC_ZERO;
			fCurrentOpacity = m_oModelRenderer.GetObject( m_iCurrent )->GetModelInstance()->GetOpacity();
		}

		if ( m_iPrevious != -1 )
		{
			fPreviousOpacity = m_oModelRenderer.GetObject( m_iPrevious )->GetModelInstance()->GetOpacity();
			m_vecPreviousPos = s_vecPreviousPosition;
		}

		if ( m_iNext != -1 )
		{
			fNextOpacity = m_oModelRenderer.GetObject( m_iNext )->GetModelInstance()->GetOpacity();
			m_vecNextPos = s_vecNextPosition;
		}

		if ( m_iLeaving != -1 )
			fLeavingOpacity = m_oModelRenderer.GetObject( m_iLeaving )->GetModelInstance()->GetOpacity();

		// Hide the items that were shown
		if ( m_iLeaving != -1 )
		{
			m_oModelRenderer.SetObjectVisible( m_iLeaving, TFALSE );
			m_oModelRenderer.SetObjectOpacity( m_iLeaving, 0.0f );

			if ( ( m_iPrevious + iNumItems - 1 ) % iNumItems == m_iLeaving )
				m_iLeaving = ( m_iPrevious + m_iNumItems - 1 ) % m_iNumItems;
			else if ( ( m_iNext + 1 ) % iNumItems == m_iLeaving )
				m_iLeaving = ( m_iNext + 1 ) % m_iNumItems;
			else
				m_iLeaving = -1;
		}

		if ( m_iPrevious != -1 )
		{
			m_oModelRenderer.SetObjectVisible( m_iPrevious, TFALSE );
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, 0.0f );
			m_iPrevious = ( m_iNumItems + iIndex - 1 ) % m_iNumItems;
		}

		if ( m_iNext != -1 )
		{
			m_oModelRenderer.SetObjectVisible( m_iNext, TFALSE );
			m_oModelRenderer.SetObjectOpacity( m_iNext, 0.0f );
			m_iNext = ( iIndex + 1 ) % m_iNumItems;
		}

		if ( m_iCurrent != -1 )
		{
			m_oModelRenderer.SetObjectVisible( m_iCurrent, TFALSE );
			m_oModelRenderer.SetObjectOpacity( m_iCurrent, 0.0f );
		}

		m_iCurrent = iIndex;
		m_iLeaving = -1;

		// Find the neighbours of the new item
		TBOOL bNextOnly     = TFALSE;
		TBOOL bSetPositions = TTRUE;
		TBOOL bSetNextPos   = TFALSE;

		switch ( m_iNumItems )
		{
			case 1:
				m_iPrevious   = -1;
				m_iNext       = -1;
				bSetPositions = TFALSE;
				break;
			case 2:
				if ( iIndex != 0 )
				{
					m_iPrevious = 0;
					m_iNext     = -1;
				}
				else
				{
					bNextOnly = TTRUE;
				}

				break;
			case 3:
				if ( iIndex == 0 )
				{
					bNextOnly = TTRUE;
				}
				else if ( iIndex != 1 )
				{
					m_iPrevious = 1;
					m_iNext     = -1;
				}
				else
				{
					m_iPrevious = 0;
					m_iNext     = 2;
					bSetNextPos = TTRUE;
				}

				break;
			default:
				m_iPrevious = ( m_iNumItems + iIndex - 1 ) % m_iNumItems;
				m_iNext     = ( iIndex + 1 ) % m_iNumItems;
				bSetNextPos = TTRUE;
				break;
		}

		if ( bSetPositions )
		{
			if ( bNextOnly )
			{
				m_iNext      = 1;
				m_iPrevious  = -1;
				m_vecNextPos = s_vecNextPosition;
			}
			else
			{
				if ( bSetNextPos )
					m_vecNextPos = s_vecNextPosition;

				m_vecPreviousPos = s_vecPreviousPosition;
			}
		}

		// Show the items at their places
		m_pCurrentObject = m_apObjects[ iIndex ];
		m_oModelRenderer.SetObjectOpacity( iIndex, fCurrentOpacity );
		m_oModelRenderer.SetObjectVisible( m_iCurrent, TTRUE );
		m_vecCurrentPos = TVector4::VEC_ZERO;
		SetObjectTransform( m_pCurrentObject, m_apTraits[ m_iCurrent ], TTRUE, m_vecCurrentPos, TVector4::VEC_ZERO, 1.0f );

		if ( m_iPrevious != -1 )
		{
			m_pPreviousObject = m_apObjects[ m_iPrevious ];
			m_oModelRenderer.SetObjectOpacity( m_iPrevious, fPreviousOpacity );
			m_oModelRenderer.SetObjectVisible( m_iPrevious, TTRUE );
			SetObjectTransform( m_pPreviousObject, m_apTraits[ m_iPrevious ], TTRUE, m_vecPreviousPos, TVector4::VEC_ZERO, 1.0f );
		}

		if ( m_iNext != -1 )
		{
			m_pNextObject = m_apObjects[ m_iNext ];
			m_oModelRenderer.SetObjectOpacity( m_iNext, fNextOpacity );
			m_oModelRenderer.SetObjectVisible( m_iNext, TTRUE );
			SetObjectTransform( m_pNextObject, m_apTraits[ m_iNext ], TTRUE, m_vecNextPos, TVector4::VEC_ZERO, 1.0f );
		}

		if ( m_iLeaving != -1 )
		{
			m_pLeavingObject = m_apObjects[ m_iLeaving ];
			m_oModelRenderer.SetObjectOpacity( m_iLeaving, fLeavingOpacity );
			m_oModelRenderer.SetObjectVisible( m_iLeaving, TTRUE );
			SetObjectTransform( m_pLeavingObject, m_apTraits[ m_iLeaving ], TTRUE, m_vecLeavingPos, TVector4::VEC_ZERO, 1.0f );
		}
	}

	UpdateItemText();
	m_fCloseTime = 0.0f;
}
