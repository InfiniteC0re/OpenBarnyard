#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIScene.h"
#  include "AGUI2XUIVisual.h"
#  include "AGUI2XUIResource.h"
#  include "GUI/AGUI2.h"

#  include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIScene );

// $BarnyardWiiJP: FUNCTION 8041ae00
AGUI2XUIScene::AGUI2XUIScene()
    : m_oTimelinePlayer( &m_oTimelineControl )
{
	m_pDefaultFocus   = TNULL;
	m_pTransFrom      = TNULL;
	m_pTransTo        = TNULL;
	m_pTransBackFrom  = TNULL;
	m_pTransBackTo    = TNULL;
	m_uiUnk1          = 0;
	m_bIgnorePresses  = TFALSE;
	m_pFocusedControl = TNULL;
}

// $BarnyardWiiJP: FUNCTION 8041ae7c
// $BarnyardWiiJP: FUNCTION 8041b574
AGUI2XUIScene::~AGUI2XUIScene()
{
	if ( m_pTransFrom )
		delete m_pTransFrom;

	if ( m_pTransTo )
		delete m_pTransTo;

	if ( m_pTransBackFrom )
		delete m_pTransBackFrom;

	if ( m_pTransBackTo )
		delete m_pTransBackTo;
}

// $BarnyardWiiJP: FUNCTION 8041af88
TBOOL AGUI2XUIScene::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	XURXUISceneData* pData = TSTATICCAST( XURXUISceneData, a_pData );

	AGUI2XUIControl::Create( a_rResource, a_pData, TTRUE );
	m_oTimelineControl.Create( this, a_rResource, a_pData );

	m_uiUnk1         = pData->m_uiUnk1;
	m_bIgnorePresses = pData->m_bIgnorePresses;

	AGUI2XUI& rXUI = AGUI2::GetSingleton()->GetXUI();

	if ( pData->m_usTransFrom != 0 )
		m_pTransFrom = rXUI.CreateVisual( a_rResource.GetString( pData->m_usTransFrom ) );

	if ( pData->m_usTransTo != 0 )
		m_pTransTo = rXUI.CreateVisual( a_rResource.GetString( pData->m_usTransTo ) );

	if ( pData->m_usTransBackFrom != 0 )
		m_pTransBackFrom = rXUI.CreateVisual( a_rResource.GetString( pData->m_usTransBackFrom ) );

	if ( pData->m_usTransBackTo != 0 )
		m_pTransBackTo = rXUI.CreateVisual( a_rResource.GetString( pData->m_usTransBackTo ) );

	if ( pData->m_usDefaultFocus != 0 )
	{
		AGUI2XUIElement* pDefaultFocus = FindChildElement( a_rResource.GetString( pData->m_usDefaultFocus ) );

		if ( pDefaultFocus )
			m_pDefaultFocus = pDefaultFocus;
	}

	if ( pData->m_usClassOverride != 0 )
		m_wszClassOverride = a_rResource.GetString( pData->m_usClassOverride );

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( pChild->GetClass()->IsA( &TGetClass( AGUI2XUIControl ) ) )
			TSTATICCAST( AGUI2XUIControl, pChild )->ResolveNavigation( this );
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041b148
void AGUI2XUIScene::Render()
{
	AGUI2XUIControl::Render();
}

// $BarnyardWiiJP: FUNCTION 8041b14c
void AGUI2XUIScene::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() )
		m_oTimelinePlayer.Tick( a_fDeltaTime );

	AGUI2XUIControl::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 8041b1c8
TBOOL AGUI2XUIScene::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	TBOOL bHandled = TFALSE;

	if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
		return bHandled;

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL && it->IsFocused(); it = GetPrevChild( it ) )
	{
		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( !pChild->GetClass()->IsA( &TGetClass( AGUI2XUIControl ) ) )
			continue;

		if ( m_pFocusedControl )
		{
			AGUI2XUIControl* pNewFocus = TNULL;

			switch ( a_pEvent->GetDoodad() )
			{
				case TInputDeviceKeyboard::KEY_UP:
					pNewFocus = m_pFocusedControl->GetNavigation( XURXUIControlData::NAV_UP );
					break;
				case TInputDeviceKeyboard::KEY_DOWN:
					pNewFocus = m_pFocusedControl->GetNavigation( XURXUIControlData::NAV_DOWN );
					break;
				case TInputDeviceKeyboard::KEY_LEFT:
					pNewFocus = m_pFocusedControl->GetNavigation( XURXUIControlData::NAV_LEFT );
					break;
				case TInputDeviceKeyboard::KEY_RIGHT:
					pNewFocus = m_pFocusedControl->GetNavigation( XURXUIControlData::NAV_RIGHT );
					break;
				case TInputDeviceKeyboard::KEY_RETURN:
					m_pFocusedControl->Press();
					bHandled = TTRUE;
					break;
			}

			if ( pNewFocus && m_pFocusedControl->GetState() == STATE_FOCUSED )
			{
				m_pFocusedControl->SetFocus( TFALSE );
				pNewFocus->SetFocus( TTRUE );
				m_pFocusedControl = pNewFocus;
				bHandled          = TTRUE;
			}
		}

		if ( bHandled == TTRUE )
			break;
	}

	return bHandled;
}

// $BarnyardWiiJP: FUNCTION 8041b334
void AGUI2XUIScene::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelinePlayer.PlayAnimation( a_wszStartFrame, a_wszEndFrame );
	AGUI2XUIControl::PlayAnimation( a_wszStartFrame, a_wszEndFrame );
}

// $BarnyardWiiJP: FUNCTION 8041b38c
void AGUI2XUIScene::PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_oTimelinePlayer.PlayTimelineRange( a_fStart, a_fEnd );
	AGUI2XUIControl::PlayTimelineRange( a_fStart, a_fEnd );
}

// $BarnyardWiiJP: FUNCTION 8041b3e4
void AGUI2XUIScene::StopAnimation()
{
	m_oTimelinePlayer.Stop();
	AGUI2XUIElement::StopAnimation();
}

// $BarnyardWiiJP: FUNCTION 8041b41c
void AGUI2XUIScene::StopAndResetAnimation()
{
	m_oTimelinePlayer.StopAndReset();
}

// $BarnyardWiiJP: FUNCTION 8041b424
void AGUI2XUIScene::SetFocus( TBOOL a_bFocused )
{
	if ( m_pDefaultFocus )
		m_pDefaultFocus->SetFocus( a_bFocused );

	m_pFocusedControl = TSTATICCAST( AGUI2XUIControl, m_pDefaultFocus );
}

// $BarnyardWiiJP: FUNCTION 8041b474
TFLOAT AGUI2XUIScene::GetNamedFrameTime( const TWCHAR* a_wszName )
{
	AGUI2XUITimelineControl::NamedFrame* pFrame = m_oTimelineControl.FindNamedFrame( a_wszName );

	if ( pFrame )
		return XUI_FRAME_TIME * TFLOAT( pFrame->m_uiTime );

	return -1.0f;
}

// $BarnyardWiiJP: FUNCTION 8041b4cc
void AGUI2XUIScene::SetTimelineTime( TFLOAT a_fTime )
{
	if ( a_fTime != m_oTimelineControl.GetTime() )
		m_oTimelineControl.SetTime( TTRUE, a_fTime );
}

#endif // BARNYARD_LATEST
