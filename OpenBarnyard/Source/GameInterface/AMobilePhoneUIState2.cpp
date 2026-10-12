#include "pch.h"
#include "AMobilePhoneUIState2.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameTimeManager.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Tasks/ARootTask.h"
#include "Units/AUnit.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitType.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0043ac40, CreateObject 0043afc0, CreateObjectInPlace 0043b020 (the constructor is inlined in both)
TDEFINE_CLASS( AMobilePhoneUIState2 );

TPSTRING8_DECLARE( DialogInputMap );
TPSTRING8_DECLARE( Equip );
TPSTRING8_DECLARE( mobile );
TPSTRING8_DECLARE( miles );
TPSTRING8_DECLARE( ben );
TPSTRING8_DECLARE( pig );
TPSTRING8_DECLARE( duke );
TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( otis );
TPSTRING8_DECLARE( hanna );
TPSTRING8_DECLARE( root );
TPSTRING8_DECLARE( twink );
TPSTRING8_DECLARE( peck );
TPSTRING8_DECLARE( eddie );
TPSTRING8_DECLARE( nb_eddy );
TPSTRING8_DECLARE( bessy );
TPSTRING8_DECLARE( daisy );
TPSTRING8_DECLARE( freddy );
TPSTRING8_DECLARE( joey );
TPSTRING8_DECLARE( end_otis );

// The units that can send messages...
static const TPString8* s_aSenders[ AMobilePhoneUIState2::NUM_SENDERS ] = {
	&TPS8( miles ),
	&TPS8( ben ),
	&TPS8( pig ),
	&TPS8( duke ),
	&TPS8( pip ),
	&TPS8( otis ),
	&TPS8( hanna ),
	&TPS8( root ),
	&TPS8( twink ),
	&TPS8( peck ),
	&TPS8( eddie ),
	&TPS8( nb_eddy ),
	&TPS8( bessy ),
	&TPS8( daisy ),
	&TPS8( freddy ),
	&TPS8( joey ),
	&TPS8( end_otis ),
}; // 0x00773520

// ...and their pictures
static const TCHAR* s_aSenderPictures[ AMobilePhoneUIState2::NUM_SENDERS ] = {
	"Miles_SMS",
	"Ben_SMS",
	"Pig_SMS",
	"Duke_SMS",
	"Pip_SMS",
	"Otis_SMS",
	"Hanna_SMS",
	"Root_SMS",
	"Twink_SMS",
	"Peck_SMS",
	"Jersey_SMS",
	"Jersey_SMS",
	"Bessy_SMS",
	"Daisy_SMS",
	"Freddy_SMS",
	"Joey_SMS",
	"Otis_SMS",
}; // 0x00773568

// Locale ids of the week days (AGameTimeManager::GetWeekDay)
static const TINT s_aWeekDayNames[ 7 ] = { 1169, 1170, 1171, 1172, 1173, 1174, 1168 }; // 0x0071abfc

// $Barnyard: FUNCTION 0043ac60
AMobilePhoneUIState2::AMobilePhoneUIState2()
    : m_iNumMessages( 0 )
    , m_iMessage( 0 )
    , m_fPlayerNameScale( 1.0f )
    , m_bMouseMoved( TFALSE )
    , m_bPromptHovered( TFALSE )
    , m_bLeftArrowHovered( TFALSE )
    , m_bRightArrowHovered( TFALSE )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the font is only created by OnInsertion but OnRemoval deletes it
	m_pMessageFont = TNULL;
#endif
}

// $Barnyard: FUNCTION 0043add0
// $Barnyard: FUNCTION 0043b090
AMobilePhoneUIState2::~AMobilePhoneUIState2()
{
}

// $Barnyard: FUNCTION 0043ac50
const TPString8* AMobilePhoneUIState2::GetSenderName( TINT a_iIndex )
{
	return s_aSenders[ a_iIndex ];
}

// $Barnyard: FUNCTION 0043b060
TINT AMobilePhoneUIState2::FindSender( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < NUM_SENDERS; i++ )
	{
		if ( *s_aSenders[ i ] == a_rcName )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0043af30
TBOOL AMobilePhoneUIState2::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		m_bMouseMoved = TTRUE;
		return TTRUE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 )
		return AGameState::ProcessInput( a_pInputEvent );

	// Clicks on the prompt and on the arrows
	TBOOL bStopEvents;

	if ( m_bPromptHovered == TTRUE )
		return ProcessCommand( AInputCommand_OK, a_pInputEvent, bStopEvents );

	if ( m_bLeftArrowHovered == TTRUE )
		return ProcessCommand( AInputCommand_Left, a_pInputEvent, bStopEvents );

	if ( m_bRightArrowHovered == TTRUE )
		return ProcessCommand( AInputCommand_Right, a_pInputEvent, bStopEvents );

	return TTRUE;
}

// $Barnyard: FUNCTION 0043be60
TBOOL AMobilePhoneUIState2::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	switch ( a_eInputCommand )
	{
		case AInputCommand_Context4:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			return TTRUE;
		case AInputCommand_OK:
		case AInputCommand_Cancel:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUOK );
			Remove();
			return TTRUE;
		case AInputCommand_Left:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			ChangeMessage( -1 );
			return TTRUE;
		case AInputCommand_Right:
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_MENUCLICK );
			ChangeMessage( 1 );
			return TTRUE;
		default:
			return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 0043b160
TBOOL AMobilePhoneUIState2::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseMoved == TTRUE )
	{
		// Update what the mouse is over
		T2GUITransform oTransform;
		TVector2       vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
		TVector2       vecTransformed;

		m_oPrompt.GetInvScreenTransform( oTransform );
		oTransform.Transform( vecTransformed, vecCursorPos );
		m_bPromptHovered = m_oPrompt.IsPointInside( vecTransformed.x, vecTransformed.y );

		if ( !m_bPromptHovered )
		{
			m_aArrows[ 0 ].GetInvScreenTransform( oTransform );
			oTransform.Transform( vecTransformed, vecCursorPos );
			m_bLeftArrowHovered = m_aArrows[ 0 ].IsPointInside( vecTransformed.x, vecTransformed.y );
		}

		if ( !m_bPromptHovered && !m_bLeftArrowHovered )
		{
			m_aArrows[ 1 ].GetInvScreenTransform( oTransform );
			oTransform.Transform( vecTransformed, vecCursorPos );
			m_bRightArrowHovered = m_aArrows[ 1 ].IsPointInside( vecTransformed.x, vecTransformed.y );
		}

		m_bMouseMoved = TFALSE;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0043b4e0
void AMobilePhoneUIState2::OnInsertion()
{
	// Darken the screen
	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	T2GUIElement* pRoot = AGUI2::GetRootElement();
	m_oFade.SetDimensions( fWidth, fHeight );
	m_oFade.SetColour( 0x7F000000 );
	pRoot->AddChildTail( m_oFade );

	ARootTask::GetSingleton()->SetPaused( TTRUE );
	SetInputMap( TPS8( DialogInputMap ) );

	// The phone: the screen texture and its mirror image
	T2GUITextureSection* pScreen       = T2GUITextureSectionManager::GetTextureSection( "Phone_Screen" );
	TFLOAT               fScreenWidth  = pScreen->GetWidth();
	TFLOAT               fScreenHeight = pScreen->GetHeight() * 1.5f;
	T2GUIFont*           pFont         = T2GUIFontManager::FindFont( "Rekord26" );

	m_oScreenRight.SetDimensions( fScreenWidth, fScreenHeight );
	m_oScreenRight.SetTextureSection( pScreen );
	m_oScreenRight.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );

	m_oScreenLeft.SetDimensions( fScreenWidth, fScreenHeight );
	m_oScreenLeft.SetTextureSection( pScreen );
	m_oScreenLeft.FlipU();
	m_oScreenLeft.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

	TFLOAT fPhoneWidth = fScreenWidth + fScreenWidth - 1.0f;
	m_oPhone.SetDimensions( fPhoneWidth, fScreenHeight );

	TFLOAT fScale = AGUI2::GetRootElement()->GetWidth() / fPhoneWidth * 0.8f;
	m_oPhone.GetTransform().Scale( fScale, fScale );

	// What to press at the bottom
	TWCHAR wszPrompt[ 64 ];
	TStringManager::String16Format( wszPrompt, TARRAYSIZE( wszPrompt ), L"%s  %s", L"%c_DialogInputMap[OK]", ALocaleManager::Interface()->GetString( 1081 ) );
	ALocale::FormatInputTags( m_wszPrompt, TARRAYSIZE( m_wszPrompt ), wszPrompt );

	TFLOAT fPromptOffset   = fScreenHeight * 0.03f;
	TFLOAT fPromptWidth    = fPhoneWidth * 0.4f;
	TFLOAT fPromptScale    = fPromptWidth / pFont->GetTextWidth( m_wszPrompt, 1.0f );
	TFLOAT fMaxPromptScale = 1.0f;

	if ( fPromptScale > fMaxPromptScale ) fPromptScale = fMaxPromptScale;

	m_oPrompt.Create( pFont, fPromptWidth );
	m_oPrompt.SetScale( fPromptScale );
	m_oPrompt.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oPrompt.SetText( m_wszPrompt );
	m_oPrompt.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPrompt.SetTransform( 0.0f, -fPromptOffset );

	// "Messages n/m" at the top
	const TWCHAR* wszMessages = ALocaleManager::Interface()->GetString( 1078 );

	m_iNumMessages = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetNumUsedItems();
	m_iMessage     = m_iNumMessages > 0;
	TStringManager::String16Format( m_wszMessageNumber, TARRAYSIZE( m_wszMessageNumber ), L" %d/%d", m_iMessage, m_iNumMessages );

	m_oMessagesLabel.Create( pFont, pFont->GetTextWidth( wszMessages, 1.0f ) );
	m_oMessagesLabel.SetScale( 0.8f );
	m_oMessagesLabel.SetText( wszMessages );
	m_oMessagesLabel.SetColour( 0xFF8F99F0 );
	m_oMessagesLabel.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );

	m_oMessageNumber.Create( pFont, pFont->GetTextWidth( m_wszMessageNumber, 1.0f ) );
	m_oMessageNumber.SetScale( 0.8f );
	m_oMessageNumber.SetText( m_wszMessageNumber );
	m_oMessageNumber.SetColour( 0xFFFFF000 );
	m_oMessageNumber.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );

	TFLOAT fNumberHeight  = m_oMessageNumber.GetHeight();
	TFLOAT fLabelHeight   = m_oMessagesLabel.GetHeight();
	TFLOAT fCounterHeight = ( fLabelHeight >= fNumberHeight ) ? fLabelHeight : fNumberHeight;

	m_oMessageCounter.SetDimensions( m_oMessagesLabel.GetWidth() + m_oMessageNumber.GetWidth(), fCounterHeight );
	m_oMessageCounter.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oMessageCounter.SetTransform( fPhoneWidth * 0.03f, fScreenHeight * 0.19f );
	m_oMessageCounter.AddChildTail( m_oMessagesLabel );
	m_oMessageCounter.AddChildTail( m_oMessageNumber );

	// The arrows that change the message
	TFLOAT fPictureX = fPhoneWidth * 0.18f;
	TFLOAT fArrowY   = fScreenHeight * 0.18f;

	T2GUITextureSection* pLeftArrow = T2GUITextureSectionManager::GetTextureSection( "left_arrow" );
	m_aArrows[ 0 ].SetDimensions( pLeftArrow->GetWidth() * 0.6f, pLeftArrow->GetHeight() * 0.6f );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_aArrows[ 0 ].SetTransform( fPictureX, fArrowY );

	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( "left_arrow2" );
	m_aArrows[ 1 ].SetDimensions( pRightArrow->GetWidth() * 0.6f, pRightArrow->GetHeight() * 0.6f );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_aArrows[ 1 ].SetTransform( -fPictureX, fArrowY );

	// The envelope
	T2GUITextureSection* pMessageIcon = T2GUITextureSectionManager::GetTextureSection( "SMS_Message" );
	m_oMessageIcon.SetDimensions( pMessageIcon->GetWidth(), pMessageIcon->GetHeight() );
	m_oMessageIcon.SetTextureSection( pMessageIcon );
	m_oMessageIcon.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oMessageIcon.SetTransform( fPhoneWidth * 0.23f, fScreenHeight * 0.15f );

	// The week day and AM/PM
	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
	const TWCHAR*     wszDayHalf       = ALocaleManager::Interface()->GetString( ( pGameTimeManager->GetCurrentHour() < 12 ) ? 1176 : 1175 );
	const TWCHAR*     wszWeekDay       = ALocaleManager::Interface()->GetString( s_aWeekDayNames[ pGameTimeManager->GetWeekDay() ] );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original passes 64 as the size of the 32 characters buffer
	TStringManager::String16Format( m_wszDate, TARRAYSIZE( m_wszDate ), L"%s  %s", wszWeekDay, wszDayHalf );
#else
	TStringManager::String16Format( m_wszDate, 64, L"%s  %s", wszWeekDay, wszDayHalf );
#endif

	m_oDate.Create( pFont, pFont->GetTextWidth( m_wszDate, 1.0f ) );
	m_oDate.SetScale( 0.5f );
	m_oDate.SetColour( 0xFF8F99F0 );
	m_oDate.SetText( m_wszDate );
	m_oDate.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oDate.SetTransform( fPhoneWidth * -0.08f, fScreenHeight * 0.28f );

	// The picture of the sender
	TFLOAT fPictureY = fScreenHeight * 0.35f;

	T2GUITextureSection* pSenderPicture = T2GUITextureSectionManager::GetTextureSection( "Player_SMS" );
	m_oSenderPicture.SetDimensions( pSenderPicture->GetWidth(), pSenderPicture->GetHeight() );
	m_oSenderPicture.SetTextureSection( pSenderPicture );
	m_oSenderPicture.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oSenderPicture.SetTransform( fPictureX, fPictureY );

	if ( m_iNumMessages == 0 )
	{
		// No messages, show the player
		const TCHAR*         szHide         = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE );
		T2GUITextureSection* pPlayerPicture = T2GUITextureSectionManager::GetTextureSection( szHide );

		m_oPlayerPicture.SetDimensions( pPlayerPicture->GetWidth(), pPlayerPicture->GetHeight() );
		m_oPlayerPicture.SetTextureSection( pPlayerPicture );
		m_oPlayerPicture.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_oPlayerPicture.SetTransform( fPictureX, fPictureY );
	}

	// The name of the sender
	TFLOAT fNameWidth = fPhoneWidth * 0.5f;

	m_oSenderName.Create( pFont, fNameWidth );
	m_oSenderName.SetScale( 1.4f );
	m_oSenderName.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oSenderName.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oSenderName.SetTransform( fPhoneWidth * 0.33f, fScreenHeight * 0.41f );

	// Make the name of the player fit
	TFLOAT fPlayerNameWidth = pFont->GetTextWidth( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString( 0 ), 1.0f );

	if ( fPlayerNameWidth > fNameWidth )
		m_fPlayerNameScale = fNameWidth / fPlayerNameWidth;

	// The message, with more space between the lines
	m_pMessageFont = T2GUIFontManager::FindFont( "Rekord18" )->Clone();
	m_pMessageFont->GetFontDef()->uiLinesMargin += 8;

	m_oMessage.Create( m_pMessageFont, fPhoneWidth * 0.685f );
	m_oMessage.SetScale( 1.0f );
	m_oMessage.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oMessage.SetColour( 0xFFFFFFFF );
	m_oMessage.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oMessage.SetTransform( 0.0f, fScreenHeight * 0.56f );

	UpdateMessage();

	m_oPhone.AddChildTail( m_oScreenRight );
	m_oPhone.AddChildTail( m_oScreenLeft );
	m_oPhone.AddChildTail( m_aArrows[ 0 ] );
	m_oPhone.AddChildTail( m_aArrows[ 1 ] );
	m_oPhone.AddChildTail( m_oMessageIcon );
	m_oPhone.AddChildTail( m_oMessageCounter );
	m_oPhone.AddChildTail( m_oDate );
	m_oPhone.AddChildTail( m_oSenderPicture );

	if ( m_iNumMessages == 0 )
		m_oPhone.AddChildTail( m_oPlayerPicture );

	m_oPhone.AddChildTail( m_oSenderName );
	m_oPhone.AddChildTail( m_oMessage );
	m_oPhone.AddChildTail( m_oPrompt );
	AGUI2::GetRootElement()->AddChildTail( m_oPhone );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0043b0b0
void AMobilePhoneUIState2::OnRemoval()
{
	m_oFade.RemoveSelf();
	ARootTask::GetSingleton()->SetPaused( TFALSE );

	// Put the phone away
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	AInventoryComponent*      pEquip     = pInventory->GetCategory( TPS8( Equip ) );
	AInventoryItem*           pMobile    = pEquip->FindItemByType( TPS8( mobile ) );

	if ( pMobile )
	{
		// Drop the phone and put it back into the player's inventory
		if ( !AInventoryGroupComponent::CollectUnit( pEquip->DropItem( pMobile ), TNULL ) )
			pInventory->SwapItemCategory( pMobile, 1 );
	}

	if ( m_pMessageFont )
	{
		delete m_pMessageFont;
	}

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0043af10
void AMobilePhoneUIState2::OnActivate()
{
	m_oPhone.Show();
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 0043af20
void AMobilePhoneUIState2::OnDeactivate()
{
	m_oPhone.Hide();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0043b280
void AMobilePhoneUIState2::UpdateMessage()
{
	m_oMessage.SetScale( 1.0f );

	if ( m_iMessage == 0 )
	{
		// No messages
		m_oMessage.SetText( ALocaleManager::Interface()->GetString( 1076 ) );
		m_oSenderName.SetText( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetName().GetString() );
		m_oSenderName.SetScale( m_fPlayerNameScale );
		return;
	}

	m_oSenderName.SetScale( 1.4f );

	TINT      iText = 0;
	TPString8 strSender;
	APlayerProfileManager::GetSingleton()->GetMainProfile()->GetUsedItemInfo( m_iMessage, iText, strSender );

	// NOTE: the item stores the text of the message as an integer
	m_oMessage.SetText( TREINTERPRETCAST( const TWCHAR*, TUINTPTR( iText ) ) );

	// Make long messages smaller
	static TFLOAT s_fMaxHeight = TMath::MAXFLOAT;

	TFLOAT fHeight = m_pMessageFont->GetTextHeightWrapped( m_oMessage.GetText(), m_oMessage.GetWidth(), m_oMessage.GetScale() );

	if ( fHeight >= 135.0f )
		s_fMaxHeight = 106.0f;
	else if ( fHeight >= 108.0f )
		s_fMaxHeight = 85.0f;

	if ( fHeight > s_fMaxHeight )
		m_oMessage.SetScale( s_fMaxHeight / fHeight );

	m_oSenderName.SetText( AUnitManager::GetSingleton()->GetUnitFactory()->Find( strSender )->GetScreenName() );

	TINT iSender = FindSender( strSender );

	if ( iSender >= 0 )
		m_oSenderPicture.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( s_aSenderPictures[ iSender ] ) );
}

// $Barnyard: FUNCTION 0043b490
void AMobilePhoneUIState2::ChangeMessage( TINT a_iDirection )
{
	if ( m_iNumMessages < 2 )
		return;

	m_iMessage = ( m_iNumMessages + a_iDirection + m_iMessage - 1 ) % m_iNumMessages + 1;
	TStringManager::String16Format( m_wszMessageNumber, TARRAYSIZE( m_wszMessageNumber ), L" %d/%d", m_iMessage, m_iNumMessages );

	UpdateMessage();
}
