#include "pch.h"
#include "AGUI2MessagePopupFlare.h"
#include "GUI/AGUI2.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00641ac0
AGUI2MessagePopupFlare::AGUI2MessagePopupFlare()
    : m_wszText( TNULL )
    , m_eState( STATE_NONE )
    , m_pTarget( TNULL )
{
	m_oPopup.m_bCentered     = TFALSE;
	m_oPopup.m_bAutoPosition = TFALSE;
	m_oPopup.m_bWhite        = TFALSE;
	m_oPopup.m_eMode         = AGUI2AnythingPopup::MODE_PULSE;
}

// $Barnyard: FUNCTION 00641d70
// $Barnyard: FUNCTION 00641b60
AGUI2MessagePopupFlare::~AGUI2MessagePopupFlare()
{
}

// $Barnyard: FUNCTION 00641f20
void AGUI2MessagePopupFlare::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_eState == STATE_NONE ) return;

	m_fFlareTime -= a_fDeltaTime;

	if ( m_fFlareTime < 0.0f && m_eState == STATE_FLARES )
	{
		for ( TINT i = 0; i < NUM_FLARES; i++ )
			m_aFlares[ i ].Start( TFALSE, 4.0f, -1.0f );

		m_eState = STATE_FLYING;
	}

	T2GUIElement::Tick( a_fDeltaTime );

	if ( m_pTarget )
	{
		// Keep following the target
		TFLOAT fScaleX, fScaleY;
		GetScreenToGUIScale( fScaleX, fScaleY );

		T2GUITransform oTransform;
		m_pTarget->GetScreenTransform( oTransform );

		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		m_oPopup.m_vEndPos.x = oTransform.GetTranslation().x * fScaleX - fWidth * 0.5f;
		m_oPopup.m_vEndPos.y = oTransform.GetTranslation().y * fScaleY - fHeight * 0.5f;
	}

	m_oPopup.Update( a_fDeltaTime );

	if ( m_pTarget && m_eState == STATE_FLYING && !m_oPopup.m_wszPopupText )
	{
		// The text has reached the target
		m_pTarget->Show();
		m_eState = STATE_NONE;
	}
}

// $Barnyard: FUNCTION 00641cb0
void AGUI2MessagePopupFlare::Render()
{
	if ( m_eState == STATE_NONE ) return;

	T2GUIElement::Render();

	if ( m_aFlares[ 0 ].m_oFlare.IsVisible() )
	{
		m_aFlares[ 0 ].SetAlpha( 1.0f );

		// The flares are kept hidden so they are only rendered here
		for ( TINT i = 0; i < NUM_FLARES; i++ )
		{
			m_aFlares[ i ].Show();
			m_aFlares[ i ].PreRender();
			m_aFlares[ i ].Render();
			m_aFlares[ i ].PostRender();
			m_aFlares[ i ].Hide();
		}
	}
}

// $Barnyard: FUNCTION 00641bd0
void AGUI2MessagePopupFlare::Create()
{
	m_aFlares[ 0 ].Create( "Flare", 0.0f, 0.1f, 0.0f, 0.2f, 0.1f, 0.5f, 5.0f );
	m_aFlares[ 1 ].Create( "Flare", 0.3f, 0.05f, 0.2f, 0.4f, 0.12f, 1.0f, 5.0f );
	m_aFlares[ 2 ].Create( "Flare", 0.9f, -0.2f, 0.5f, 0.3f, 0.13f, 0.6f, 5.0f );

	for ( TINT i = 0; i < NUM_FLARES; i++ )
	{
		AddChildTail( m_aFlares[ i ] );
		m_aFlares[ i ].Hide();
	}

	m_oTransform.SetIdentity();
}

// $Barnyard: FUNCTION 00642080
void AGUI2MessagePopupFlare::Show( const TWCHAR* a_wszText, TFLOAT a_fEndScale, T2GUIElement* a_pTarget, TUINT32 a_uiColour, TUINT32 a_uiEndColour )
{
	for ( TINT i = 0; i < NUM_FLARES; i++ )
		m_aFlares[ i ].Start( TTRUE, 1.6666666f, -1.0f );

	m_wszText     = a_wszText;
	m_uiEndColour = a_uiEndColour;
	m_fFlareTime  = 0.54f;
	m_eState      = STATE_STARTING;
	m_uiColour    = a_uiColour;
	m_fEndScale   = a_fEndScale;
	m_pTarget     = a_pTarget;

	if ( a_pTarget )
		a_pTarget->Hide();

	Start();
}

// $Barnyard: FUNCTION 00641d90
void AGUI2MessagePopupFlare::Start()
{
	TVector2 vEndPos( 0.0f, 0.0f );

	if ( m_pTarget )
	{
		// NOTE: GetScreenToGUIScale is inlined here in the original
		TFLOAT fScaleX, fScaleY;
		GetScreenToGUIScale( fScaleX, fScaleY );

		T2GUITransform oTransform;
		m_pTarget->GetScreenTransform( oTransform );

		TFLOAT fWidth, fHeight;
		AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

		vEndPos.x = oTransform.GetTranslation().x * fScaleX - fWidth * 0.5f;
		vEndPos.y = oTransform.GetTranslation().y * fScaleY - fHeight * 0.5f;
	}

	TVector2 vStartPos( 0.0f, 0.0f );

	m_oPopup.ShowIfHasText();
	m_oPopup.m_eMode       = AGUI2AnythingPopup::MODE_SCALE_AND_MOVE;
	m_oPopup.m_fDelay      = 0.0f;
	m_oPopup.m_fStartScale = 0.0f;
	m_oPopup.m_fScale      = 1.5f;
	m_oPopup.SetStartPosition( vStartPos );
	m_oPopup.m_vEndPos       = vEndPos;
	m_oPopup.m_fPulseSpeed   = 1.0f;
	m_oPopup.m_fMoveDuration = 0.5f;
	m_oPopup.m_fDuration     = 0.6f;
	m_oPopup.SetText( m_wszText, m_uiColour, this );
	m_oPopup.m_fHoldTime   = 1.0f;
	m_oPopup.m_fEndScale   = m_fEndScale;
	m_oPopup.m_uiEndColour = m_uiEndColour;

	m_eState = STATE_FLARES;
}

// $Barnyard: FUNCTION 00641d20
void AGUI2MessagePopupFlare::GetScreenToGUIScale( TFLOAT& a_rScaleX, TFLOAT& a_rScaleY )
{
	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fWidth, fHeight;
	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );

	a_rScaleX = fWidth / TINT( pDisplayParams->uiWidth );
	a_rScaleY = fHeight / TINT( pDisplayParams->uiHeight );
}
