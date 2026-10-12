#include "pch.h"
#include "AGUI2MessageBoxState.h"
#include "AGUI2.h"
#include "Input/AInputMapManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( DialogInputMap );

// NOTE: GetClass 006406e0, CreateObject 00641270, CreateObjectInPlace 006412d0
TDEFINE_CLASS( AGUI2MessageBoxState );

// $Barnyard: FUNCTION 00640c80
AGUI2MessageBoxState::AGUI2MessageBoxState()
{
	m_fPaddingX     = 0.0f;
	m_fPaddingY     = 0.0f;
	m_bAutoSize     = TFALSE;
	m_fMenuX        = 0.0f;
	m_fMenuY        = 0.0f;
	m_pResult       = TNULL;
	m_pRootElement  = TNULL;
	m_iNumButtons   = 0;
	m_bUseDialog    = TFALSE;
	m_pTextFont     = TNULL;
	m_iCancelButton = -1;
	m_pButtonFont   = TNULL;
	m_bDarkenScreen = TFALSE;

	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( DialogInputMap ) );

	m_oMenu.SetFocusSound( soundbank::UI_MENUCLICK );
	m_oMenu.SetSelectSound( soundbank::UI_MENUOK );
	m_oMenu.SetBackSound( soundbank::UI_MENUBACK );
	m_oMenu.SetNegativeSound( soundbank::UI_NEGATIVE );
}

// $Barnyard: FUNCTION 006406f0
// $Barnyard: FUNCTION 00640dd0
AGUI2MessageBoxState::~AGUI2MessageBoxState()
{
	m_oMenu.RemoveAllItems();

	if ( m_bUseDialog == TTRUE )
	{
		m_oDialog.Destroy();
	}
	else
	{
		m_oBackground.RemoveSelf();
		m_oText.RemoveSelf();
	}

	T2GUIFontManager::ReleaseFont( m_pTextFont );
	T2GUIFontManager::ReleaseFont( m_pButtonFont );
}

// $Barnyard: FUNCTION 006409e0
TBOOL AGUI2MessageBoxState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_oMenu.ProcessInputCommand( a_eInputCommand, a_pInputEvent ) == TTRUE )
		return TTRUE;

	return AGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $Barnyard: FUNCTION 00640a20
TBOOL AGUI2MessageBoxState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_oMenu.Update( a_fDeltaTime );
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00640f60
void AGUI2MessageBoxState::OnInsertion()
{
	m_oMenu.ReflowChildrenVertically();

	if ( m_bAutoSize == TTRUE )
	{
		if ( m_bUseDialog == TTRUE )
		{
			TFLOAT fMenuWidth, fMenuHeight;
			m_oMenu.GetDimensions( fMenuWidth, fMenuHeight );

			TFLOAT fTextWidth, fTextHeight;
			m_oText.GetDimensions( fTextWidth, fTextHeight );

			m_fWidth = fTextWidth;

			if ( fMenuWidth > fTextWidth )
				m_fWidth = fMenuWidth;

			// NOTE: only the height of the menu is used here
			m_fHeight = fMenuHeight;
			m_oDialog.SetDialogSize( m_fPaddingX + m_fWidth, fMenuHeight + m_fPaddingY );
			m_oText.SetTransform( 0.0f, 15.0f, 0.0f );
		}
		else
		{
			TFLOAT fMenuWidth, fMenuHeight;
			m_oMenu.GetDimensions( fMenuWidth, fMenuHeight );

			TFLOAT fTextWidth, fTextHeight;
			m_oText.GetDimensions( fTextWidth, fTextHeight );

			TFLOAT fMaxWidth = ( fMenuWidth > fTextWidth ) ? fMenuWidth : fTextWidth;
			TFLOAT fHeight   = m_oText.GetLineHeight() + fTextHeight + fMenuHeight;

			m_fWidth  = m_fPaddingX + m_fPaddingX + fMaxWidth;
			m_fHeight = m_fPaddingY + m_fPaddingY + fHeight;

			m_oText.SetTransform( 0.0f, m_fPaddingY, 0.0f );
			m_oMenu.SetTransform( m_fMenuX, m_fMenuY - m_fPaddingY, 0.0f );
			m_oMenu.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
			m_oBackground.SetDimensions( m_fWidth, m_fHeight );
		}
	}
	else if ( m_bUseDialog == TTRUE )
	{
		m_oDialog.SetDialogSize( m_fWidth, m_fHeight, m_fPaddingX );
	}
	else
	{
		m_oText.SetTransform( 0.0f, m_fPaddingY, 0.0f );
		m_oText.SetWidth( m_oText.GetWidth() - ( m_fPaddingX + m_fPaddingX ) );

		m_oMenu.SetTransform( 0.0f, -m_fPaddingY, 0.0f );
		TFLOAT fMenuHeight = m_oMenu.GetHeight();
		m_oMenu.SetDimensions( m_oMenu.GetWidth() - ( m_fPaddingX + m_fPaddingX ), fMenuHeight );

		m_oBackground.SetDimensions( m_fPaddingX + m_fPaddingX + m_fWidth, m_fPaddingY + m_fPaddingY + m_fHeight );
	}

	m_oMenu.SetDefaultFocusElement( &m_aButtons[ 0 ] );

	if ( m_iCancelButton > -1 )
		m_oMenu.SetCancelItem( &m_aButtons[ m_iCancelButton ] );

	m_oMenu.SetItemActivationCallback( OnButtonActivated );
	m_oMenu.SetCallbackUserData( this );

	for ( TINT i = 0; i < m_iNumButtons; i++ )
		m_aButtons[ i ].OnFocusLost();

	m_oMenu.SetFocusAt( m_aButtons[ 0 ] );
	AGUI2::GetRootElement()->AddChildTail( *m_pRootElement );

	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 00640a50
void AGUI2MessageBoxState::OnRemoval()
{
	if ( m_bDarkenScreen )
		m_oScreenRect.RemoveSelf();

	m_pRootElement->RemoveSelf();

	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 00640df0
void AGUI2MessageBoxState::Create( TFLOAT a_fWidth, TFLOAT a_fHeight, const TCHAR* a_szTextFont, const TCHAR* a_szButtonFont, const TCHAR* a_szTopLeft, const TCHAR* a_szTopCenter, const TCHAR* a_szTopRight, const TCHAR* a_szMiddleLeft, const TCHAR* a_szMiddleCenter, const TCHAR* a_szMiddleRight, const TCHAR* a_szBottomLeft, const TCHAR* a_szBottomCenter, const TCHAR* a_szBottomRight, TBOOL a_bDarkenScreen )
{
	m_bDarkenScreen = a_bDarkenScreen;

	if ( a_bDarkenScreen )
	{
		TFLOAT fScreenWidth, fScreenHeight;
		AGUI2::GetSingleton()->GetDimensions( fScreenWidth, fScreenHeight );

		m_oScreenRect.SetDimensions( fScreenWidth, fScreenHeight );
		m_oScreenRect.SetColour( 0xff000000 );
		AGUI2::GetRootElement()->AddChildTail( m_oScreenRect );
	}

	m_fWidth       = a_fWidth;
	m_fHeight      = a_fHeight;
	m_bUseDialog   = TTRUE;
	m_pRootElement = &m_oDialog;

	m_oDialog.Create( a_szTopLeft, a_szTopCenter, a_szTopRight, a_szMiddleLeft, a_szMiddleCenter, a_szMiddleRight, a_szBottomLeft, a_szBottomCenter, a_szBottomRight );
	m_oDialog.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oDialog.SetDialogSize( a_fWidth + 128.0f, a_fHeight );

	m_pTextFont   = T2GUIFontManager::FindFont( a_szTextFont );
	m_pButtonFont = T2GUIFontManager::FindFont( a_szButtonFont );

	TFLOAT fContentWidth, fContentHeight;
	m_oDialog.GetContentDimensions( fContentWidth, fContentHeight );

	m_oText.Create( m_pTextFont, fContentWidth );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oText.SetColour( 0xffffffff );
	m_oDialog.AddChildTail( m_oText );

	m_oMenu.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oDialog.AddChildToRectangle( &m_oMenu, T2GUIATTACHMENT_MIDDLECENTER );
}

// $Barnyard: FUNCTION 00640850
TBOOL AGUI2MessageBoxState::AddButton( const TWCHAR* a_wszText )
{
	if ( m_iNumButtons == MAX_NUM_BUTTONS )
		return TFALSE;

	TFLOAT fWidth, fHeight;

	if ( m_bUseDialog == TTRUE )
		m_oDialog.GetContentDimensions( fWidth, fHeight );
	else
		m_oBackground.GetDimensions( fWidth, fHeight );

	m_aButtons[ m_iNumButtons ].Create( m_pButtonFont, fWidth, a_wszText, 0.0f );
	m_oMenu.AddMenuItem( m_aButtons[ m_iNumButtons ] );
	m_iNumButtons++;

	return TTRUE;
}

// $Barnyard: FUNCTION 00640820
void AGUI2MessageBoxState::SetText( const TWCHAR* a_wszText )
{
	m_oText.SetText( a_wszText );
}

// $Barnyard: FUNCTION 00640840
void AGUI2MessageBoxState::SetTextColour( TUINT32 a_uiColour )
{
	m_oText.SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 00640810
void AGUI2MessageBoxState::SetDialogColour( TUINT32 a_uiColour )
{
	m_pRootElement->SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 00640900
void AGUI2MessageBoxState::SetButtonColours( TUINT32 a_uiFocusedColour, TUINT32 a_uiEnabledColour )
{
	for ( TINT i = 0; i < m_iNumButtons; i++ )
	{
		m_aButtons[ i ].SetFocusedColour( a_uiFocusedColour );
		m_aButtons[ i ].SetEnabledColour( a_uiEnabledColour );
	}
}

// $Barnyard: FUNCTION 00640950
void AGUI2MessageBoxState::SetButtonScales( TFLOAT a_fFocusedScale, TFLOAT a_fScale )
{
	for ( TINT i = 0; i < m_iNumButtons; i++ )
	{
		m_aButtons[ i ].SetFocusedScale( a_fFocusedScale );
		m_aButtons[ i ].SetScale( a_fScale );
	}
}

// $Barnyard: FUNCTION 00640b40
void AGUI2MessageBoxState::SetTextShadowEnabled( TBOOL a_bEnabled )
{
	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		m_aButtons[ i ].GetTextBox().SetShadowEnabled( a_bEnabled );

	m_oText.SetShadowEnabled( a_bEnabled );
}

// $Barnyard: FUNCTION 00640b80
void AGUI2MessageBoxState::SetTextShadowColour( TUINT32 a_uiColour )
{
	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		m_aButtons[ i ].GetTextBox().SetShadowColour( a_uiColour );

	m_oText.SetShadowColour( a_uiColour );
}

// $Barnyard: FUNCTION 00640bc0
void AGUI2MessageBoxState::SetTextShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY )
{
	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		m_aButtons[ i ].GetTextBox().SetShadowOffset( a_fOffsetX, a_fOffsetY );

	m_oText.SetShadowOffset( a_fOffsetX, a_fOffsetY );
}

// $Barnyard: FUNCTION 00640c30
void AGUI2MessageBoxState::SetTextShadowAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < MAX_NUM_BUTTONS; i++ )
		m_aButtons[ i ].GetTextBox().SetShadowAlpha( a_fAlpha );

	m_oText.SetShadowAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 006409a0
TINT AGUI2MessageBoxState::GetFocusedButton()
{
	for ( TINT i = 0; i < m_iNumButtons; i++ )
	{
		if ( m_aButtons[ i ].IsFocused() == TTRUE )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00640a90
void AGUI2MessageBoxState::OnButtonActivated( void* a_pUserData, AGUI2MenuItem* a_pItem )
{
	AGUI2MessageBoxState* pState = TSTATICCAST( AGUI2MessageBoxState, a_pUserData );

	if ( ( ( pState->m_iCancelButton >= 0 && a_pItem != &pState->m_aButtons[ pState->m_iCancelButton ] ) || pState->m_iCancelButton == -1 ) && pState->m_pResult )
		*pState->m_pResult = pState->GetFocusedButton();

	pState->Remove();
}
