#include "pch.h"
#include "ACameraOptionsState.h"

#ifdef BARNYARD_LATEST

#  include "AOptionsState.h"
#  include "AOptions.h"
#  include "Sound/ASoundManager.h"
#  include "SoundBank/ui.h"
#  include "GUI/T2GUIFontManager.h"
#  include "GUI/AGUI2Button.h"
#  include "GUI/AGUI2Style.h"
#  include "GUI/T2GUITextureSectionManager.h"
#  include "Locale/ALocaleManager.h"
#  include "HUD/AHudManager.h"
#  include "HUD/AMenuNavHudElement.h"
#  include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 8015425c, CreateObject 80154268, CreateObjectInPlace 801542a4
TDEFINE_CLASS( ACameraOptionsState );

//-----------------------------------------------------------------------------
// ACameraSpeedSlider
//-----------------------------------------------------------------------------

// Time an arrow takes to spin once
static constexpr TFLOAT CAMERASLIDER_SPIN_TIME = 0.25f;

// Scale of the arrow icons
static constexpr TFLOAT CAMERASLIDER_ARROW_SCALE = 0.4f;

// Scale of the bar icons
static constexpr TFLOAT CAMERASLIDER_BAR_SCALE = 0.5f;

// $BarnyardWiiJP: FUNCTION 801542bc
ACameraSpeedSlider::ACameraSpeedSlider()
    : m_iID( 0 )
    , m_fnArrowFocusCallback( TNULL )
    , m_pCallbackUserData( TNULL )
{
	m_aArrowSpin[ 0 ] = 1.0f;
	m_aArrowSpin[ 1 ] = 1.0f;
}

// $BarnyardWiiJP: FUNCTION 8015434c
ACameraSpeedSlider::~ACameraSpeedSlider()
{
}

// $BarnyardWiiJP: FUNCTION 801543dc
TBOOL ACameraSpeedSlider::Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon )
{
	// Setup left arrow
	T2GUITextureSection* pLeftArrow = T2GUITextureSectionManager::GetTextureSection( a_szLeftArrow );
	TVALIDPTR( pLeftArrow );

	TFLOAT fLeftArrowWidth = TMath::Abs( TFLOAT( pLeftArrow->GetWidth() ) ) * CAMERASLIDER_ARROW_SCALE;
	m_aArrows[ 0 ].SetDimensions( fLeftArrowWidth, TMath::Abs( TFLOAT( pLeftArrow->GetHeight() ) ) * CAMERASLIDER_ARROW_SCALE );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );

	TFLOAT fX = 0.0f - m_aArrows[ 0 ].GetWidth();
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLECENTER );
	m_aArrows[ 0 ].SetTransform( fX + 0.5f * m_aArrows[ 0 ].GetWidth(), 0.0f, 0.0f );
	AddChildTail( m_aArrows[ 0 ] );

	fX += fLeftArrowWidth;

	// Setup bar icons
	T2GUITextureSection* pBarIcon = T2GUITextureSectionManager::GetTextureSection( a_szBarIcon );
	TVALIDPTR( pBarIcon );

	TFLOAT fBarWidth  = TMath::Abs( TFLOAT( pBarIcon->GetWidth() ) );
	TFLOAT fBarHeight = TMath::Abs( TFLOAT( pBarIcon->GetHeight() ) );

	for ( TINT i = 0; i < NUM_BARS; i++ )
	{
		m_aBars[ i ].SetDimensions( fBarWidth, fBarHeight );
		m_aBars[ i ].SetTextureSection( pBarIcon );
		m_aBars[ i ].SetTransform( fX, 0.0f, 0.0f );

		T2GUITransform oScale;
		oScale.Set( TVector2( CAMERASLIDER_BAR_SCALE, 0.0f ), TVector2( 0.0f, CAMERASLIDER_BAR_SCALE ), TVector2::VEC_ZERO );
		m_aBars[ i ].GetTransform().PreMultiply( oScale );

		m_aBars[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		AddChildTail( m_aBars[ i ] );

		fX += fBarWidth * CAMERASLIDER_BAR_SCALE;
	}

	// Setup right arrow
	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( a_szRightArrow );
	TVALIDPTR( pRightArrow );

	TFLOAT fRightArrowWidth = TMath::Abs( TFLOAT( pRightArrow->GetWidth() ) ) * CAMERASLIDER_ARROW_SCALE;
	m_aArrows[ 1 ].SetDimensions( fRightArrowWidth, TMath::Abs( TFLOAT( pRightArrow->GetHeight() ) ) * CAMERASLIDER_ARROW_SCALE );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLECENTER );
	m_aArrows[ 1 ].SetTransform( fX + 0.5f * m_aArrows[ 1 ].GetWidth(), 0.0f, 0.0f );
	AddChildTail( m_aArrows[ 1 ] );

	SetDimensions( ( fX + fRightArrowWidth ) - CAMERASLIDER_ARROW_SCALE * m_aArrows[ 0 ].GetWidth(), CAMERASLIDER_BAR_SCALE * m_aBars[ 0 ].GetHeight() );
	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80154880
void ACameraSpeedSlider::RemoveElements()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		m_aArrows[ i ].RemoveSelf();

	for ( TINT i = 0; i < NUM_BARS; i++ )
		m_aBars[ i ].RemoveSelf();
}

// $BarnyardWiiJP: FUNCTION 80154990
void ACameraSpeedSlider::SetNumLitBars( TINT a_iNumBars )
{
	for ( TINT i = 0; i < NUM_BARS; i++ )
		m_aBars[ i ].SetColour( ( i < a_iNumBars ) ? 0xFFFFFFFF : 0xFF000000 );
}

// $BarnyardWiiJP: FUNCTION 80154a1c
void ACameraSpeedSlider::SpinArrow( TINT a_iDirection )
{
	if ( a_iDirection >= 0 )
	{
		if ( m_aArrowSpin[ 1 ] >= 1.0f )
			m_aArrowSpin[ 1 ] = 0.0f;
	}
	else if ( m_aArrowSpin[ 0 ] >= 1.0f )
	{
		m_aArrowSpin[ 0 ] = 0.0f;
	}
}

// $BarnyardWiiJP: FUNCTION 8015490c
void ACameraSpeedSlider::SetAlpha( TFLOAT a_fAlpha )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
		it->SetAlpha( a_fAlpha );

	T2GUIElement::SetAlpha( a_fAlpha );
}

// $BarnyardWiiJP: FUNCTION 80154778
void ACameraSpeedSlider::OnUpdate( TFLOAT a_fDeltaTime )
{
	// One full turn in CAMERASLIDER_SPIN_TIME
	static TFLOAT s_fSpinSpeed = 4.0f * TMath::TWO_PI;

	// The left arrow spins counterclockwise, the right one clockwise
	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
	{
		if ( m_aArrowSpin[ i ] < 1.0f )
		{
			TFLOAT fStep = a_fDeltaTime / CAMERASLIDER_SPIN_TIME;

			if ( m_aArrowSpin[ i ] + fStep > 1.0f )
				fStep = 1.0f - m_aArrowSpin[ i ];

			TFLOAT fSpeed = ( i == 0 ) ? -s_fSpinSpeed * CAMERASLIDER_SPIN_TIME : s_fSpinSpeed * CAMERASLIDER_SPIN_TIME;

			m_aArrowSpin[ i ] += fStep;
			m_aArrows[ i ].GetTransform().Rotate( fStep * fSpeed );
		}
	}

	AGUI2MenuGridItem::OnUpdate( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 80154254
TFLOAT ACameraSpeedSlider::GetFlowVisualOffset()
{
	return 0.0f;
}

// $BarnyardWiiJP: FUNCTION 8015424c
TFLOAT ACameraSpeedSlider::GetFlowOffset()
{
	return 0.0f;
}

//-----------------------------------------------------------------------------
// ACameraOptionsState
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80154a64
ACameraOptionsState::ACameraOptionsState()
    : m_iCameraSpeed( 0 )
    , m_iOriginalCameraSpeed( 0 )
    , m_iFocusedItemId( 0 )
    , m_bSettingsChangedOnInsertion( TFALSE )
{
	// NOTE: the original doesn't initialise the members above, OnInsertion sets them
}

// $BarnyardWiiJP: FUNCTION 80154b20
ACameraOptionsState::~ACameraOptionsState()
{
}

// NOTE: the PC locales stop before the texts of the latest version, and these screens have more options than it
// ("Camera Speed", CAMERA_SPEED_TEXT, is both the button and the label there), so the texts are in English
static const TWCHAR* s_aOptionTexts[ ACameraOptionsState::CAMERAOPTIONS_NUMOF ] = {
	L"Speed",
	L"Wii Style",
	L"Follow Jumps",
};

// Texts of the toggles
static constexpr TINT TEXT_OFF = 1096;
static constexpr TINT TEXT_ON  = 1097;

const TWCHAR* ACameraOptionsState::GetCameraText()
{
	return L"Camera";
}

// Reads a toggle from AOptions
static TBOOL GetOptionsToggle( TINT a_iOption )
{
	AOptions* pOptions = AOptions::GetSingleton();
	return ( a_iOption == ACameraOptionsState::CAMERAOPTIONS_WIISTYLE ) ? pOptions->IsWiiStyleCamera() : pOptions->IsFollowJumpsCamera();
}

void ACameraOptionsState::SetToggle( TINT a_iOption, TBOOL a_bValue )
{
	AOptions* pOptions = AOptions::GetSingleton();

	if ( a_iOption == CAMERAOPTIONS_WIISTYLE )
		pOptions->SetWiiStyleCamera( a_bValue );
	else
		pOptions->SetFollowJumpsCamera( a_bValue );

	TINT iToggle           = a_iOption - CAMERAOPTIONS_WIISTYLE;
	m_abToggles[ iToggle ] = a_bValue;
	m_aToggleSliders[ iToggle ].SetText( g_pLocaleManager->GetString( a_bValue ? TEXT_ON : TEXT_OFF ) );
}

void ACameraOptionsState::SwitchToggle( TINT a_iOption )
{
	// Applied right away like the speed, so the camera can be checked from the pause menu
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
	SetToggle( a_iOption, !m_abToggles[ a_iOption - CAMERAOPTIONS_WIISTYLE ] );

	AOptions::GetSingleton()->ApplySettings();
	AOptionsState::ms_bSettingsChanged = TTRUE;

	// Kept in OpenBarnyard.cfg, not in the save (AOptions::LoadExtraSettings)
	AOptions::GetSingleton()->SaveExtraSettings();
}

void ACameraOptionsState::UpdateLabelScale( AGUI2MenuGridTextItem& a_rLabel, TFLOAT a_fDeltaTime )
{
	// Grow the focused label and shrink it back
	T2GUITransform& rTransform = a_rLabel.GetTransform();

	TFLOAT   fScaleX      = rTransform.GetScaleX();
	TFLOAT   fScaleY      = rTransform.GetScaleY();
	TVector2 vTranslation = rTransform.GetTranslation();
	rTransform.SetIdentity();

	if ( &a_rLabel == m_oGridMenu.GetFocusedMenuItem() )
	{
		fScaleX = TMath::LERPClamped( fScaleX, 1.1f, a_fDeltaTime * 10.0f );
		fScaleY = TMath::LERPClamped( fScaleY, 1.1f, a_fDeltaTime * 10.0f );
	}
	else
	{
		fScaleX = TMath::LERPClamped( fScaleX, 1.0f, a_fDeltaTime * 20.0f );
		fScaleY = TMath::LERPClamped( fScaleY, 1.0f, a_fDeltaTime * 20.0f );
	}

	T2GUITransform oScale;
	oScale.Set( TVector2( fScaleX, 0.0f ), TVector2( 0.0f, fScaleY ), TVector2::VEC_ZERO );
	rTransform.PreMultiply( oScale );
	rTransform.SetTranslation( vTranslation );
}

// $BarnyardWiiJP: FUNCTION 8015555c
void ACameraOptionsState::ChangeCameraSpeed( AInputCommand a_eInputCommand )
{
	AOptions* pOptions;

	if ( a_eInputCommand == AInputCommand_Left )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
		pOptions = AOptions::GetSingleton();

		if ( m_iFocusedItemId == CAMERAOPTIONS_SPEED )
		{
			m_iCameraSpeed--;
			TMath::Clip( m_iCameraSpeed, 0, AOptions::CAMERA_SPEED_MAX );
			pOptions->SetCameraSpeed( m_iCameraSpeed );
			m_oSlider.SpinArrow( -1 );
		}
	}
	else if ( a_eInputCommand == AInputCommand_Right )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
		pOptions = AOptions::GetSingleton();

		if ( m_iFocusedItemId == CAMERAOPTIONS_SPEED )
		{
			m_iCameraSpeed++;
			TMath::Clip( m_iCameraSpeed, 0, AOptions::CAMERA_SPEED_MAX );
			pOptions->SetCameraSpeed( m_iCameraSpeed );
			m_oSlider.SpinArrow( 1 );
		}
	}
	else
	{
		return;
	}

	pOptions->ApplySettings();
	AOptionsState::ms_bSettingsChanged = TTRUE;

	// NOTE: not in the original, the camera settings are kept in OpenBarnyard.cfg (AOptions::LoadExtraSettings)
	pOptions->SaveExtraSettings();
}

// NOTE: not in the original, handles the mouse on the arrows of the toggles (like AControlOptionsState)
TBOOL ACameraOptionsState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_eMenuState == MENUSTATE_MENU_VISIBLE )
	{
		if ( m_oGridMenu.ProcessInputEvent( a_pInputEvent ) )
			return TTRUE;

		for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		{
			if ( m_aToggleSliders[ i ].ProcessInputEvent( a_pInputEvent ) )
				return TTRUE;
		}
	}

	return BaseClass::ProcessInput( a_pInputEvent );
}

// $BarnyardWiiJP: FUNCTION 80155318
TBOOL ACameraOptionsState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_eMenuState != MENUSTATE_MENU_VISIBLE )
		return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_Cancel )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUBACK );
		m_eMenuState     = MENUSTATE_MENU_DISAPPEAR;
		m_iFocusedItemId = -1;
		return TTRUE;
	}

#  ifndef TOSHI_SKU_WII
	// NOTE: the Wii changes the speed by tilting the Wii Remote (UpdateRemoteInput), here Left/Right do it like on
	// the audio options screen
	if ( a_eInputCommand == AInputCommand_Left || a_eInputCommand == AInputCommand_Right )
	{
		if ( m_iFocusedItemId == CAMERAOPTIONS_SPEED )
			ChangeCameraSpeed( a_eInputCommand );
		else if ( m_iFocusedItemId > CAMERAOPTIONS_SPEED )
			SwitchToggle( m_iFocusedItemId );

		a_pInputEvent->StartRepeat( 0.5f, 0.1f );
		return TTRUE;
	}
#  endif

	if ( a_eInputCommand != AInputCommand_Left && a_eInputCommand != AInputCommand_Right && m_oGridMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) )
		return TTRUE;

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

#  ifdef TOSHI_SKU_WII

// $BarnyardWiiJP: FUNCTION 80155678
void ACameraOptionsState::UpdateRemoteInput( TFLOAT a_fDeltaTime )
{
	// Auto repeat timer
	static TFLOAT s_fRepeatTime;

	TINT  iZone    = -1;
	TBOOL bRepeat  = TFALSE;
	TBOOL bNewZone = TFALSE;

	if ( m_bRemoteLeft ) iZone = 0;
	if ( m_bRemoteNeutral ) iZone = 1;
	if ( m_bRemoteRight ) iZone = 2;

	// Picks one of 3 zones from the tilt of the Wii Remote (sub_803f6aac)
	TINT iNewZone = AGUI2RevInput::GetTiltZone( 3, iZone, TTRUE, TFALSE, TFALSE, -999.99f, 999.99f );

	if ( iZone < 0 ) bRepeat = TTRUE;

	if ( iNewZone < 0 )
	{
		iNewZone = 1;
		iZone    = 1;
		bRepeat  = TTRUE;
	}

	if ( iNewZone != iZone )
	{
		bNewZone = TTRUE;
		bRepeat  = TTRUE;
	}

	m_bRemoteRight   = TFALSE;
	m_bRemoteLeft    = TFALSE;
	m_bRemoteNeutral = TFALSE;

	if ( iNewZone == 1 ) m_bRemoteNeutral = TTRUE;
	else if ( iNewZone == 2 ) m_bRemoteRight = TTRUE;
	else if ( iNewZone == 0 ) m_bRemoteLeft = TTRUE;

	if ( !bRepeat )
	{
		s_fRepeatTime -= a_fDeltaTime;
		bRepeat = s_fRepeatTime < 0.0f;
	}

	if ( bRepeat )
	{
		s_fRepeatTime = bNewZone ? 0.5f : 0.25f;

		if ( m_bRemoteLeft )
			ChangeCameraSpeed( AInputCommand_Left );
		else if ( m_bRemoteRight )
			ChangeCameraSpeed( AInputCommand_Right );
	}
}

#  endif // TOSHI_SKU_WII

// $BarnyardWiiJP: FUNCTION 80155004
TBOOL ACameraOptionsState::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Focusing the slider focuses its label
	// NOTE: the original has one row and always focuses m_oTextItem. Here the focus moves to the label of the focused
	// row, and only when a slider is focused (the labels are the only items that can have it)
	if ( AGUI2MenuGridItem* pFocused = m_oGridMenu.GetFocusedMenuItem() )
	{
		TINT iId = -1;

		if ( pFocused == &m_oSlider )
			iId = m_oSlider.GetId();

		for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		{
			if ( pFocused == &m_aToggleSliders[ i ] )
				iId = m_aToggleSliders[ i ].GetId();
		}

		if ( iId >= 0 && m_iFocusedItemId != -1 )
		{
			m_iFocusedItemId = iId;
			m_oGridMenu.SetFocusAt( &GetLabel( iId ) );
		}
	}

#  ifdef TOSHI_SKU_WII
	UpdateRemoteInput( a_fDeltaTime );
#  endif

	// Grow the focused label and shrink the others back
	for ( TINT i = FIRST_OPTION; i < CAMERAOPTIONS_NUMOF; i++ )
		UpdateLabelScale( GetLabel( i ), a_fDeltaTime );

	// Fade the RGB components of the bars to white (lit) or black, alpha is kept
	// NOTE: the original keeps the components in a static array
	for ( TINT i = 0; i < ACameraSpeedSlider::NUM_BARS; i++ )
	{
		T2GUIRectangle& rBar = m_oSlider.GetBar( i );

		TUINT32 uiColour  = rBar.GetColour();
		TINT    aRGB[ 3 ] = {
            TINT( ( uiColour >> 16 ) & 0xFF ),
            TINT( ( uiColour >> 8 ) & 0xFF ),
            TINT( uiColour & 0xFF )
		};

		TFLOAT fTarget = ( i < m_iCameraSpeed ) ? 255.0f : 0.0f;

		for ( TINT k = 0; k < 3; k++ )
			aRGB[ k ] = TINT( TMath::LERPClamped( TFLOAT( aRGB[ k ] ), fTarget, a_fDeltaTime * 5.0f ) );

		rBar.SetColour( ( uiColour & 0xFF000000 ) | ( aRGB[ 0 ] << 16 ) | ( aRGB[ 1 ] << 8 ) | aRGB[ 2 ] );
	}

	m_oGridMenu.Update( a_fDeltaTime );
	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 80154bc8
void ACameraOptionsState::OnInsertion()
{
	m_bSettingsChangedOnInsertion = AOptionsState::ms_bSettingsChanged;

	T2GUIFont* pFont = T2GUIFontManager::FindFont( AGUI2STYLE_FONT_PRIMARY );

	// NOTE: the original has one column (the label above the slider) and two rows. Here each row has the label on the
	// left and the slider or the toggle on the right, like AControlOptionsState
	m_oGridMenu.CreateGrid( 2, NUM_ROWS, 40.0f, 20.0f );
	m_oGridMenu.SetCallbackUserData( this );
	m_oGridMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oGridMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oGridMenu.SetSelectSound( soundbank::UI_MENUOK );

	// $BarnyardWiiJP: FUNCTION 8015542c
	m_oGridMenu.SetItemActivationCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pItem ) {
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
		TSTATICCAST( ACameraOptionsState, a_pUserData )->m_eMenuState = MENUSTATE_MENU_DISAPPEAR;
	} );

	// $BarnyardWiiJP: FUNCTION 8015546c
	m_oGridMenu.SetItemFocusCallback( []( void* a_pUserData, AGUI2MenuGridItem* a_pOldFocus, AGUI2MenuGridItem* a_pNewFocus ) {
		TSTATICCAST( ACameraOptionsState, a_pUserData )->OnMenuGridItemFocused( *a_pOldFocus, *a_pNewFocus );
	} );

	auto fnCreateLabel = [ pFont ]( AGUI2MenuGridTextItem& a_rLabel, const TWCHAR* a_wszText, TINT a_iID ) {
		a_rLabel.Create( pFont, pFont->GetTextWidth( a_wszText ), a_wszText, 10.0f );
		a_rLabel.SetFocusedColour( AGUI2STYLE_COLOR_FOCUSED );
		a_rLabel.SetEnabledColour( AGUI2STYLE_COLOR_SECONDARY );
		a_rLabel.SetDisabledColour( AGUI2STYLE_COLOR_DISABLED );
		a_rLabel.SetFocusedScale( 1.0f );
		a_rLabel.OnFocusLost();
		a_rLabel.SetId( a_iID );
		a_rLabel.SetTextShadow( TTRUE, 0xa0000000, 0.6f );
		a_rLabel.SetTextShadowOffset( 5.0f );
	};

	if constexpr ( SHOW_SPEED_SLIDER )
	{
		// Initialise label
		fnCreateLabel( m_oTextItem, s_aOptionTexts[ CAMERAOPTIONS_SPEED ], CAMERAOPTIONS_SPEED );
		m_oGridMenu.AddMenuGridItem( m_oTextItem );

		// Initialise slider
		// NOTE: the PC data doesn't have the arrows of the latest version
		const TCHAR* szLeftArrow  = T2GUITextureSectionManager::GetTextureSection( "Roll_Arrow2" ) ? "Roll_Arrow2" : "left_arrow";
		const TCHAR* szRightArrow = T2GUITextureSectionManager::GetTextureSection( "Roll_Arrow" ) ? "Roll_Arrow" : "left_arrow2";

		m_oSlider.Create( szLeftArrow, szRightArrow, "Statue_GoldStar" );
		m_oSlider.SetEnabled( TFALSE );
		m_oSlider.SetId( 0 );
		m_oSlider.SetCallbackUserData( this );

		// $BarnyardWiiJP: FUNCTION 80155488
		m_oSlider.SetArrowFocusedCallback( []( void* a_pUserData, TINT a_iSliderID ) {
			ACameraOptionsState* pState = TSTATICCAST( ACameraOptionsState, a_pUserData );
			pState->m_oGridMenu.SetFocusAt( &pState->m_oTextItem );
			pState->m_iFocusedItemId = a_iSliderID;
		} );
		m_oGridMenu.AddMenuGridItem( m_oSlider );
	}

	// NOTE: not in the original, the toggles. The text box is sized for the longest text
	const TWCHAR* wszOn      = g_pLocaleManager->GetString( TEXT_ON );
	const TWCHAR* wszOff     = g_pLocaleManager->GetString( TEXT_OFF );
	const TWCHAR* wszLongest = ( pFont->GetTextWidth( wszOn ) > pFont->GetTextWidth( wszOff ) ) ? wszOn : wszOff;

	auto fnArrowFocused = []( void* a_pUserData, TINT a_iSliderID ) {
		ACameraOptionsState* pState = TSTATICCAST( ACameraOptionsState, a_pUserData );
		pState->m_oGridMenu.SetFocusAt( &pState->GetLabel( a_iSliderID ) );
		pState->m_iFocusedItemId = a_iSliderID;
	};

	for ( TINT i = 0; i < NUM_TOGGLES; i++ )
	{
		TINT iOption = CAMERAOPTIONS_WIISTYLE + i;

		fnCreateLabel( m_aToggleTexts[ i ], s_aOptionTexts[ iOption ], iOption );
		m_oGridMenu.AddMenuGridItem( m_aToggleTexts[ i ] );

		m_aToggleSliders[ i ].Create( "left_arrow", "left_arrow2", wszLongest );
		m_aToggleSliders[ i ].SetEnabled( TFALSE );
		m_aToggleSliders[ i ].SetId( iOption );
		m_aToggleSliders[ i ].SetCallbackUserData( this );
		m_aToggleSliders[ i ].SetArrowFocusedCallback( fnArrowFocused );
		m_oGridMenu.AddMenuGridItem( m_aToggleSliders[ i ] );

		// Remember the value so it can be restored if the menu is cancelled
		m_abOriginalToggles[ i ] = GetOptionsToggle( iOption );
		m_abToggles[ i ]         = m_abOriginalToggles[ i ];
		m_aToggleSliders[ i ].SetText( g_pLocaleManager->GetString( m_abToggles[ i ] ? TEXT_ON : TEXT_OFF ) );
	}

	// Up and down go through the labels (the sliders can't be focused)
	for ( TINT i = 0; i < NUM_ROWS; i++ )
	{
		AGUI2MenuGridTextItem& rUp   = GetLabel( FIRST_OPTION + ( i + NUM_ROWS - 1 ) % NUM_ROWS );
		AGUI2MenuGridTextItem& rDown = GetLabel( FIRST_OPTION + ( i + 1 ) % NUM_ROWS );
		GetLabel( FIRST_OPTION + i ).SetSideElements( TNULL, TNULL, &rUp, &rDown );
	}

	// The second column is sized relatively to the first one
	TFLOAT fMaxLabelWidth  = 0.0f;
	TFLOAT fMaxWidgetWidth = SHOW_SPEED_SLIDER ? m_oSlider.GetWidth() : 0.0f;

	for ( TINT i = FIRST_OPTION; i < CAMERAOPTIONS_NUMOF; i++ )
		fMaxLabelWidth = TMath::Max( fMaxLabelWidth, GetLabel( i ).GetWidth() );

	for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		fMaxWidgetWidth = TMath::Max( fMaxWidgetWidth, m_aToggleSliders[ i ].GetWidth() );

	if ( fMaxLabelWidth <= fMaxWidgetWidth )
		m_oGridMenu.SetColumnWidth( 1, fMaxLabelWidth / fMaxWidgetWidth );
	else
		m_oGridMenu.SetColumnWidth( 1, fMaxWidgetWidth / fMaxLabelWidth );

	m_oGridMenu.ReflowChildren();

	// Center the labels in the first column
	for ( TINT i = FIRST_OPTION; i < CAMERAOPTIONS_NUMOF; i++ )
	{
		AGUI2MenuGridTextItem& rLabel = GetLabel( i );

		rLabel.SetTransform(
		    fMaxLabelWidth * 0.5f + rLabel.GetTransform().GetTranslation().x,
		    rLabel.GetTransform().GetTranslation().y
		);

		rLabel.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPCENTER );
	}

	m_oDialog.AddChildTail( m_oGridMenu );

	// Remember the speed so it can be restored if the menu is cancelled
	m_iCameraSpeed = m_iOriginalCameraSpeed = AOptions::GetSingleton()->GetCameraSpeed();

	m_oGridMenu.SetFocusAt( &GetLabel( FIRST_OPTION ) );

	m_oDialogTitle.SetText( g_pLocaleManager->GetString( TITLE_TEXT ) );
	m_bFocusedElementBloated = TFALSE;

	m_oSlider.SetNumLitBars( m_iCameraSpeed );

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1050, 1051, -1 );

	BaseClass::OnInsertion();

	m_oDialog.SetDimensions( m_oGridMenu.GetWidth() + 128.0f, m_oGridMenu.GetHeight() + 150.0f );

#  ifdef TOSHI_SKU_WII
	m_bRemoteRight   = TFALSE;
	m_bRemoteLeft    = TFALSE;
	m_bRemoteNeutral = TTRUE;

	if ( AGUI2RevControllerAnim::ms_bEnabled )
	{
		m_oControllerAnim.Create();
		m_oControllerAnim.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
		AGUI2::GetRootElement()->AddChildTail( m_oControllerAnim );
	}
#  endif
}

// $BarnyardWiiJP: FUNCTION 80154f3c
void ACameraOptionsState::OnRemoval()
{
#  ifdef TOSHI_SKU_WII
	m_oControllerAnim.RemoveSelf();
#  endif

	for ( TINT i = 0; i < NUM_TOGGLES; i++ )
	{
		m_aToggleSliders[ i ].RemoveElements();
		m_aToggleTexts[ i ].RemoveSelf();
	}

	if constexpr ( SHOW_SPEED_SLIDER )
	{
		m_oTextItem.RemoveSelf();
		m_oSlider.RemoveElements();
		m_oSlider.RemoveSelf();
	}
	m_oGridMenu.RemoveSelf();

	AHudManager::GetSingleton()->FindElement<AMenuNavHudElement>()->SetTexts( 1047, 1048, -1 );

	BaseClass::OnRemoval();
}

// $BarnyardWiiJP: FUNCTION 80154fdc
void ACameraOptionsState::OnActivate()
{
	m_iFocusedItemId = FIRST_OPTION;

#  ifdef TOSHI_SKU_WII
	m_oControllerAnim.Show();
#  endif

	BaseClass::OnActivate();
}

// $BarnyardWiiJP: FUNCTION 80154ff4
void ACameraOptionsState::OnDeactivate()
{
#  ifdef TOSHI_SKU_WII
	m_oControllerAnim.Hide();
#  endif

	BaseClass::OnDeactivate();
}

// $BarnyardWiiJP: FUNCTION 801553d4
void ACameraOptionsState::OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem )
{
	ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
	ABYardMenuState::OnButtonActivated( TSTATICCAST( AGUI2Button, a_pMenuItem ) );
}

// $BarnyardWiiJP: FUNCTION 80155420
void ACameraOptionsState::OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus )
{
	m_iFocusedItemId = TSTATICCAST( AGUI2Button, &a_rNewFocus )->GetId();
}

// $BarnyardWiiJP: FUNCTION 801554d8
void ACameraOptionsState::OnMenuClose()
{
	if ( m_iFocusedItemId == -1 )
	{
		// Cancelled, restore the speed
		AOptions* pOptions = AOptions::GetSingleton();

		if ( m_iCameraSpeed != m_iOriginalCameraSpeed )
		{
			pOptions->SetCameraSpeed( m_iOriginalCameraSpeed );
			pOptions->ApplySettings();
		}

		// NOTE: not in the original, also restore the toggles
		for ( TINT i = 0; i < NUM_TOGGLES; i++ )
		{
			if ( m_abToggles[ i ] != m_abOriginalToggles[ i ] )
			{
				SetToggle( CAMERAOPTIONS_WIISTYLE + i, m_abOriginalToggles[ i ] );
				pOptions->ApplySettings();
			}
		}

		AOptionsState::ms_bSettingsChanged = m_bSettingsChangedOnInsertion;

		// NOTE: not in the original, write the restored settings to OpenBarnyard.cfg
		pOptions->SaveExtraSettings();
	}

	BaseClass::Remove();
}

// $BarnyardWiiJP: FUNCTION 8015547c
void ACameraOptionsState::OnMenuGridItemFocused( AGUI2MenuGridItem& a_rOldFocus, AGUI2MenuGridItem& a_rNewFocus )
{
	// NOTE: the toggle sliders can't be focused, they're disabled like m_oSlider
	m_iFocusedItemId = ( &a_rNewFocus == &m_oSlider ) ? m_oSlider.GetId() : TSTATICCAST( AGUI2MenuGridTextItem, &a_rNewFocus )->GetId();
}

#endif // BARNYARD_LATEST
