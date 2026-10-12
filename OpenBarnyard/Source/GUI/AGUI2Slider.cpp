#include "pch.h"
#include "AGUI2Slider.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUITextureSectionManager.h"
#include "Input/AInputHandler.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0063c690
AGUI2Slider::AGUI2Slider()
{
	m_iNumBars               = 0;
	m_pBars                  = TNULL;
	m_bMouseDirty            = TFALSE;
	m_iLastPressedArrowIndex = -1;
}

// $Barnyard: FUNCTION 0063c710
// $Barnyard: FUNCTION 0063cb40
AGUI2Slider::~AGUI2Slider()
{
	if ( m_pBars )
	{
		delete[] m_pBars;
		m_pBars = TNULL;
	}
}

// $Barnyard: FUNCTION 0063c7a0
TBOOL AGUI2Slider::Create( const TCHAR* a_szLeftArrow, const TCHAR* a_szRightArrow, const TCHAR* a_szBarIcon, TINT a_iNumBars )
{
	TFLOAT fMaxArrowSize = 0.0f;

	// Setup left arrow (the sections are square)
	T2GUITextureSection* pLeftArrow = T2GUITextureSectionManager::GetTextureSection( a_szLeftArrow );
	TFLOAT               fLeftSize  = pLeftArrow->GetWidth();

	m_aArrows[ 0 ].SetDimensions( fLeftSize, fLeftSize );
	m_aArrows[ 0 ].SetTextureSection( pLeftArrow );
	m_aArrows[ 0 ].SetTransform( 0.0f, 0.0f );
	m_aArrows[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 0 ] );

	TFLOAT fX = fLeftSize;

	if ( fLeftSize > 0.0f )
		fMaxArrowSize = fLeftSize;

	// Setup the bars
	T2GUITextureSection* pBarIcon = T2GUITextureSectionManager::GetTextureSection( a_szBarIcon );
	TFLOAT               fBarSize = pBarIcon->GetWidth();

	m_iNumBars = a_iNumBars;
	m_pBars    = new T2GUIRectangle[ a_iNumBars ];

	for ( TINT i = 0; i < m_iNumBars; i++ )
	{
		m_pBars[ i ].SetDimensions( fBarSize, fBarSize );
		m_pBars[ i ].SetTextureSection( pBarIcon );
		m_pBars[ i ].SetTransform( fX, 0.0f );
		m_pBars[ i ].GetTransform().Scale( ms_fBarScale, ms_fBarScale );
		m_pBars[ i ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		AddChildTail( m_pBars[ i ] );

		fX += ms_fBarScale * fBarSize;
	}

	// Setup right arrow
	T2GUITextureSection* pRightArrow = T2GUITextureSectionManager::GetTextureSection( a_szRightArrow );
	TFLOAT               fRightSize  = pRightArrow->GetWidth();

	m_aArrows[ 1 ].SetDimensions( fRightSize, fRightSize );
	m_aArrows[ 1 ].SetTextureSection( pRightArrow );
	m_aArrows[ 1 ].SetTransform( fX, 0.0f );
	m_aArrows[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	AddChildTail( m_aArrows[ 1 ] );

	// NOTE: only the left arrow is used for the height
	SetDimensions( fX + fRightSize, ms_fBarScale * fMaxArrowSize );
	return TTRUE;
}

// $Barnyard: FUNCTION 0063ca60
void AGUI2Slider::RemoveElements()
{
	for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		m_aArrows[ i ].RemoveSelf();

	if ( m_pBars )
	{
		for ( TINT i = 0; i < m_iNumBars; i++ )
			m_pBars[ i ].RemoveSelf();

		delete[] m_pBars;
		m_pBars = TNULL;
	}
}

// $Barnyard: FUNCTION 0063cdd0
TBOOL AGUI2Slider::ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		// Arrows are hit-tested in Tick
		m_bMouseDirty = TTRUE;
		return TFALSE;
	}

	if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN ||
	     a_pInputEvent->GetDoodad() != TInputDeviceMouse::BUTTON_1 ||
	     m_iLastPressedArrowIndex == -1 )
		return TFALSE;

	// Clicking an arrow emulates a press of the Left/Right key
	TInputDeviceKeyboard* pKeyboard = AInputHandler::GetSingleton()->GetKeyboardDevice();
	TINT                  iKey      = ( m_iLastPressedArrowIndex != 0 ) ? TInputDeviceKeyboard::KEY_RIGHT : TInputDeviceKeyboard::KEY_LEFT;

	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TInputInterface::GetSingleton()->SendEvent( pKeyboard, iKey, TInputInterface::EVENT_TYPE_GONE_UP );
	return TTRUE;
}

// $Barnyard: FUNCTION 0063cb60
void AGUI2Slider::Tick( TFLOAT a_fDeltaTime )
{
	// The value is spread over the bars, every bar holds a part of it in the 0-1 range
	TFLOAT fFill = m_iNumBars * m_fValue;

	for ( TINT i = 0; i < m_iNumBars; i++ )
	{
		T2GUIRectangle& rBar = m_pBars[ i ];

		// NOTE: the original keeps the channels in global variables (007b5058-007b5064)
		TUINT32 uiColour  = rBar.GetColour();
		TINT    aRGB[ 3 ] = {
            TINT( ( uiColour >> 16 ) & 0xFF ),
            TINT( ( uiColour >> 8 ) & 0xFF ),
            TINT( uiColour & 0xFF )
		};

		TFLOAT fTarget;

		if ( fFill > 1.0f )
		{
			fTarget = 255.0f;
			fFill -= 1.0f;
		}
		else if ( fFill > 0.0f )
		{
			fTarget = fFill * 255.0f;
			fFill   = 0.0f;
		}
		else
		{
			fTarget = 0.0f;
		}

		// Fade the RGB components to the target, alpha is kept
		for ( TINT k = 0; k < 3; k++ )
			aRGB[ k ] = TINT( TMath::LERP( TFLOAT( aRGB[ k ] ), fTarget, ms_fBarFadeSpeed * a_fDeltaTime ) );

		rBar.SetColour( ( uiColour & 0xFF000000 ) | ( aRGB[ 0 ] << 16 ) | ( aRGB[ 1 ] << 8 ) | aRGB[ 2 ] );
	}

	if ( m_bMouseDirty == TTRUE )
	{
		// Check if any of the arrow rectangles is hovered
		m_iLastPressedArrowIndex = -1;
		T2GUITransform oElementInvTransform;

		TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();

		for ( TINT i = 0; i < TARRAYSIZE( m_aArrows ); i++ )
		{
			// Get cursor position that is relative to the arrow
			m_aArrows[ i ].GetInvScreenTransform( oElementInvTransform );

			TVector2 vecElementRelCursorPos;
			oElementInvTransform.Transform( vecElementRelCursorPos, vecCursorPos );

			if ( m_aArrows[ i ].IsPointInside( vecElementRelCursorPos.x, vecElementRelCursorPos.y ) )
			{
				m_fnArrowFocusCallback( m_pCallbackUserData, m_iID );
				m_iLastPressedArrowIndex = i;
			}
		}

		m_bMouseDirty = TFALSE;
	}
}

// NOTE: folded with AGUI2MenuItem::SetAlpha (0063cb00)
void AGUI2Slider::SetAlpha( TFLOAT a_fAlpha )
{
	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
		it->SetAlpha( a_fAlpha );

	T2GUIElement::SetAlpha( a_fAlpha );
}
