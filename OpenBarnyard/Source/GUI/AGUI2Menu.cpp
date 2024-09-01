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

TOSHI_NAMESPACE_USING

AGUI2Menu::AGUI2Menu()
{
	m_Unk1              = 0;
	m_pUnk2             = TNULL;
	m_pCancelItem       = TNULL;
	m_pLastMenuItem     = TNULL;
	m_pFocusedMenuItem  = TNULL;
	m_eFocusSound       = soundbank::UI_MENUCLICK;
	m_eSelectSound      = soundbank::UI_MENUOK;
	m_eBackSound        = soundbank::UI_MENUBACK;
	m_eNegativeSound    = soundbank::UI_NEGATIVE;
	m_eActionFlags      = 0;
	m_Unk3              = 0;
	m_Unk4              = 0;
	m_fnActionCallback  = TNULL;
	m_fnFocusCallback   = TNULL;
	m_pCallbackUserData = TNULL;
	m_fTime             = 0.0f;
	m_bVerticalFlow     = TFALSE;
	m_iNumMenuItems     = 0;
	m_bHoverDirty       = TFALSE;
	m_pHoveredMenuItem  = TNULL;
	m_eActionFlags      = 0;
	m_bFlag1            = TTRUE;
}

AGUI2Menu::~AGUI2Menu()
{
}

void AGUI2Menu::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bHoverDirty )
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

						m_pFocusedMenuItem = pItem;
						break;
					}
				}

				pItem = pItem->m_pPrevMenuItem;
				if ( pItem == m_pLastMenuItem ) break;
			}
		}

		m_bHoverDirty = TFALSE;
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

TBOOL AGUI2Menu::ProcessInputCommand( AInputCommand a_eCommand, Toshi::TInputInterface::InputEvent* a_pEvent )
{
	TBOOL bHandled = TFALSE;

	if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN ||
	     a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT ||
	     TFALSE )
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
				bHandled = TFALSE;
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
				if ( m_fnActionCallback )
					m_fnActionCallback( m_pCallbackUserData, m_pCancelItem );
			}
			else
			{
				// Can't cancel, play negative sound
				ASoundManager::GetSingleton()->PlayCue( m_eNegativeSound );
			}
		}
		else if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_OK )
		{
			if ( m_fnActionCallback && m_pFocusedMenuItem )
			{
				// Can submit
				ASoundManager::GetSingleton()->PlayCue( m_eSelectSound );

				if ( m_fnActionCallback )
					m_fnActionCallback( m_pCallbackUserData, m_pFocusedMenuItem );
			}
		}
		else if ( eCommandResult == AGUI2MenuItem::COMMANDRESULT_NONE )
		{
			// If element didn't process the command, check if the menu can handle this

			if ( ( !m_bVerticalFlow && m_eActionFlags & ACTIONFLAGS_UP ) ||
			     ( m_bVerticalFlow && m_eActionFlags & ACTIONFLAGS_LEFT ) )
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
			else if ( ( !m_bVerticalFlow && m_eActionFlags & ACTIONFLAGS_DOWN ) ||
			          ( m_bVerticalFlow && m_eActionFlags & ACTIONFLAGS_RIGHT ) )
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

void AGUI2Menu::SetAlphaRecursive( TFLOAT a_fAlpha, TFLOAT a_fShadowAlpha )
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
