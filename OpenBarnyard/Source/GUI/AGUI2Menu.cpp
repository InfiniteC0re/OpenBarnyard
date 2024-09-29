#include "pch.h"
#include "AGUI2Menu.h"
#include "AGUI2.h"

#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

#define EAT_ALL_INPUT_EVENTS TFALSE

TOSHI_NAMESPACE_USING

AGUI2Menu::AGUI2Menu()
{
	m_fItemSpacing         = 0.0f;
	m_pDefaultFocusElement = TNULL;
	m_pCancelItem          = TNULL;
	m_pLastMenuItem        = TNULL;
	m_pFocusedMenuItem     = TNULL;
	m_eFocusSound          = soundbank::UI_MENUCLICK;
	m_eSelectSound         = soundbank::UI_MENUOK;
	m_eBackSound           = soundbank::UI_MENUBACK;
	m_eNegativeSound       = soundbank::UI_NEGATIVE;
	m_eActionFlags         = 0;
	m_Unk3                 = 0;
	m_Unk4                 = 0;
	m_fnActivateCallback   = TNULL;
	m_fnFocusCallback      = TNULL;
	m_pCallbackUserData    = TNULL;
	m_fTime                = 0.0f;
	m_bHorizontalFlow      = TFALSE;
	m_iNumMenuItems        = 0;
	m_bMouseStateDirty     = TFALSE;
	m_pHoveredMenuItem     = TNULL;
	m_eActionFlags         = 0;
	m_bFlag1               = TTRUE;
}

AGUI2Menu::~AGUI2Menu()
{
}

void AGUI2Menu::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseStateDirty )
	{
		AGUI2Transform oElementInvTransform;

		if ( m_pLastMenuItem != TNULL )
		{
			TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
			m_pHoveredMenuItem    = TNULL;

			AGUI2MenuItem* pItem = m_pLastMenuItem;

			while ( TTRUE )
			{
				if ( pItem->IsVisible() )
				{
					// Get inverse transform of the current UI element
					pItem->GetInvScreenTransform( oElementInvTransform );

					// Get cursor position that is relative to the UI element
					TVector2 vecElementRelCursorPos;
					oElementInvTransform.Transform( vecElementRelCursorPos, vecCursorPos );

					if ( pItem->IsPointInside( vecElementRelCursorPos.x, vecElementRelCursorPos.y ) )
					{
						if ( m_pFocusedMenuItem != pItem )
						{
							SetFocusAt( *pItem );
							ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );
						}

						m_pHoveredMenuItem = pItem;
						break;
					}
				}

				pItem = pItem->m_pPrevMenuItem;
				if ( pItem == m_pLastMenuItem ) break;
			}
		}

		m_bMouseStateDirty = TFALSE;
	}

	if ( m_pLastMenuItem != TNULL )
	{
		AGUI2MenuItem* pItem = m_pLastMenuItem;

		while ( TTRUE )
		{
			if ( pItem->IsVisible() )
			{
				pItem->OnUpdate( a_fDeltaTime );
			}

			pItem = pItem->m_pPrevMenuItem;
			if ( pItem == m_pLastMenuItem ) break;
		}
	}

	m_fTime += a_fDeltaTime;
}

TBOOL AGUI2Menu::ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		// Make sure menu updates mouse state
		SetMouseStateDirty();
	}
	else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN && a_pEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 && GetHoveredMenuItem() != TNULL )
	{
		// LMB was pressed and the menu has a hovered item
		return TriggerButtonPress( *GetHoveredMenuItem() );
	}

	return TFALSE;
}

TBOOL AGUI2Menu::ProcessInputCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	TBOOL bHandled = TFALSE;

	if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ||
	     a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT ||
	     EAT_ALL_INPUT_EVENTS )
	{
		bHandled = TTRUE;

		switch ( a_eCommand )
		{
			case AInputCommand_OK:
			case AInputCommand_Shoot:
				m_eActionFlags |= ACTIONFLAGS_OK;
				break;
			case AInputCommand_Cancel:
				m_eActionFlags |= ACTIONFLAGS_CANCEL;
				break;
			case AInputCommand_Up:
			case AInputCommand_Forward:
				m_eActionFlags |= ACTIONFLAGS_UP;
				a_pEvent->StartRepeat( 0.5f, 0.1f );
				break;
			case AInputCommand_Down:
			case AInputCommand_Backward:
				m_eActionFlags |= ACTIONFLAGS_DOWN;
				a_pEvent->StartRepeat( 0.5f, 0.1f );
				break;
			case AInputCommand_Left:
				m_eActionFlags |= ACTIONFLAGS_LEFT;
				a_pEvent->StartRepeat( 0.5f, 0.1f );
				break;
			case AInputCommand_Right:
				m_eActionFlags |= ACTIONFLAGS_RIGHT;
				a_pEvent->StartRepeat( 0.5f, 0.1f );
				break;
			default:
				bHandled = EAT_ALL_INPUT_EVENTS;
				break;
		}
	}

	if ( m_pFocusedMenuItem )
	{
		AGUI2MenuItem::COMMANDRESULT eCommandResult = m_pFocusedMenuItem->OnInputCommand( m_eActionFlags );

		if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_CANCEL )
		{
			if ( m_pCancelItem )
			{
				// Can cancel, play back sound
				ASoundManager::GetSingleton()->PlayCue( m_eBackSound );

				// Call handler if it's specified
				if ( m_fnActivateCallback )
					m_fnActivateCallback( m_pCallbackUserData, m_pCancelItem );
			}
			else
			{
				// Can't cancel, play negative sound
				ASoundManager::GetSingleton()->PlayCue( m_eNegativeSound );
			}
		}
		else if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_OK )
		{
			ActivateFocusedButton();
		}
		else if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_NONE )
		{
			// If element didn't process the command, check if the menu can handle this

			if ( ( !m_bHorizontalFlow && m_eActionFlags & ACTIONFLAGS_UP ) ||
			     ( m_bHorizontalFlow && m_eActionFlags & ACTIONFLAGS_LEFT ) )
			{
				// Focus on next element
				AGUI2MenuItem* pNewFocused = m_pFocusedMenuItem->m_pNextMenuItem;

				// Skip elements that cannot be focused/activated
				while ( pNewFocused->m_pNextMenuItem && !pNewFocused->CanFocus() )
					pNewFocused = pNewFocused->m_pNextMenuItem;

				if ( pNewFocused != m_pFocusedMenuItem )
				{
					// Focus changed, play audio and call handler

					ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );

					if ( m_fnFocusCallback )
						m_fnFocusCallback( m_pCallbackUserData, m_pFocusedMenuItem, pNewFocused );

					SetFocusAt( *pNewFocused );
					bHandled = TTRUE;
				}
			}
			else if ( ( !m_bHorizontalFlow && m_eActionFlags & ACTIONFLAGS_DOWN ) || ( m_bHorizontalFlow && m_eActionFlags & ACTIONFLAGS_RIGHT ) )
			{
				// Focus on previous element
				AGUI2MenuItem* pNewFocused = m_pFocusedMenuItem->m_pPrevMenuItem;

				// Skip elements that cannot be focused/activated
				while ( pNewFocused->m_pPrevMenuItem && !pNewFocused->CanFocus() )
					pNewFocused = pNewFocused->m_pPrevMenuItem;

				if ( pNewFocused != m_pFocusedMenuItem )
				{
					// Focus changed, play audio and call handler

					ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );

					if ( m_fnFocusCallback )
						m_fnFocusCallback( m_pCallbackUserData, m_pFocusedMenuItem, pNewFocused );

					SetFocusAt( *pNewFocused );
					bHandled = TTRUE;
				}
			}
		}
	}

	m_eActionFlags = ACTIONFLAGS_NONE;
	return bHandled;
}

void AGUI2Menu::AddMenuItem( AGUI2MenuItem& a_rMenuItem )
{
	if ( m_pLastMenuItem )
		a_rMenuItem.LinkMenuItemBefore( *m_pLastMenuItem );
	else
		m_pLastMenuItem = &a_rMenuItem;

	AddChildTail( a_rMenuItem );
	m_iNumMenuItems++;
}

void AGUI2Menu::SetFocusAt( AGUI2MenuItem& a_rMenuItem )
{
	if ( a_rMenuItem.IsVisible() )
	{
		if ( m_pFocusedMenuItem )
			m_pFocusedMenuItem->OnFocusLost();

		m_pFocusedMenuItem = &a_rMenuItem;
		a_rMenuItem.OnFocus();
	}
}

void AGUI2Menu::ReflowChildrenVertically()
{
	m_fWidth  = 0.0f;
	m_fHeight = 0.0f;

	TFLOAT         fOffsetY  = 0.0f;
	AGUI2MenuItem* pMenuItem = m_pLastMenuItem;

	if ( pMenuItem )
	{
		while ( TTRUE )
		{
			if ( pMenuItem->IsVisible() )
			{
				TFLOAT fItemWidth, fItemHeight;
				pMenuItem->GetDimensions( fItemWidth, fItemHeight );

				TFLOAT fItemPosY = fItemHeight * 0.5f + pMenuItem->GetFlowVisualOffset() + fOffsetY;
				pMenuItem->GetTransform().SetTranslation( 0.0f, fItemPosY );
				pMenuItem->SetAttachment( AGUI2ATTACHMENT_TOPCENTER, AGUI2ATTACHMENT_MIDDLECENTER );

				m_fWidth = TMath::Max( m_fWidth, fItemWidth );
				fOffsetY = fItemHeight * 0.5f + pMenuItem->GetFlowOffset() + fItemPosY + m_fItemSpacing;
			}

			pMenuItem = pMenuItem->m_pPrevMenuItem;
			if ( pMenuItem == m_pLastMenuItem ) break;
		}

		// Set focus at the last added item
		SetFocusAt( *m_pLastMenuItem );
	}

	// Update height
	m_fHeight = fOffsetY;

	// Set focus at default item if it's specified
	if ( m_pFocusedMenuItem )
		SetFocusAt( *m_pFocusedMenuItem );

	m_bHorizontalFlow = TFALSE;
}

TBOOL AGUI2Menu::TriggerButtonPress( AGUI2MenuItem& a_rMenuItem )
{
	m_eActionFlags |= ACTIONFLAGS_OK;
	AGUI2MenuItem::COMMANDRESULT eCommandResult = a_rMenuItem.OnInputCommand( m_eActionFlags );
	m_eActionFlags                              = ACTIONFLAGS_NONE;

	if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_OK )
	{
		ActivateFocusedButton();
		return TTRUE;
	}

	return TFALSE;
}

void AGUI2Menu::SetMenuAlpha( TFLOAT a_fAlpha, TFLOAT a_fShadowAlpha /* = -1.0f*/ )
{
	if ( m_pLastMenuItem != TNULL )
	{
		AGUI2MenuItem* pItem = m_pLastMenuItem;

		while ( TTRUE )
		{
			if ( pItem->IsVisible() )
			{
				pItem->SetAlpha( a_fAlpha );
				pItem->SetShadowAlpha( ( a_fShadowAlpha == -1.0f ) ? a_fAlpha : a_fShadowAlpha );
			}

			pItem = pItem->m_pPrevMenuItem;
			if ( pItem == m_pLastMenuItem ) break;
		}
	}

	AGUI2Element::SetAlpha( a_fAlpha );
}

void AGUI2Menu::ActivateFocusedButton()
{
	if ( m_fnActivateCallback && m_pFocusedMenuItem )
	{
		// Can activate
		ASoundManager::GetSingleton()->PlayCue( m_eSelectSound );

		if ( m_fnActivateCallback )
			m_fnActivateCallback( m_pCallbackUserData, m_pFocusedMenuItem );
	}
}
