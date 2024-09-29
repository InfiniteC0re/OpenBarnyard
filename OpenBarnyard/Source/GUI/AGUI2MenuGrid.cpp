#include "pch.h"
#include "AGUI2MenuGrid.h"
#include "AGUI2.h"

#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#define EAT_ALL_INPUT_EVENTS TFALSE

AGUI2MenuGrid::AGUI2MenuGrid()
{
	m_fItemSpacing         = 0.0f;
	m_pDefaultFocusElement = TNULL;
	m_pCancelItem          = TNULL;
	m_pLastMenuItem        = TNULL;
	m_pFocusedMenuItem     = TNULL;
	m_eFocusSound          = soundbank::UI_MENUCLICK;
	m_eSelectSound         = soundbank::UI_MENUOK;
	m_eBackSound           = soundbank::UI_MENUBACK;
	m_eActionFlags         = ACTIONFLAGS_NONE;
	m_vecMousePos          = TVector2::VEC_ZERO;
	m_fnActivateCallback   = TNULL;
	m_fnFocusCallback      = TNULL;
	m_pCallbackUserData    = TNULL;
	m_fTime                = 0.0f;
	m_iNumRows             = 0;
	m_iNumColumns          = 0;
	m_bFlag1               = TTRUE;
	m_vecMaxItemDimensions = TVector2::VEC_ZERO;
	m_fColumnGap           = 0.0f;
	m_fRowGap              = 0.0f;
	m_pColumnsData         = TNULL;
	m_pRowsData            = TNULL;
	m_fPaddingLeft         = 0.0f;
	m_fMaxItemWidth        = 0.0f;
	m_bBlockActivation     = TFALSE;
	m_pHoveredMenuItem     = TNULL;
	m_eActionFlags         = ACTIONFLAGS_NONE;
}

AGUI2MenuGrid::~AGUI2MenuGrid()
{
	delete[] m_pColumnsData;
	delete[] m_pRowsData;
}

void AGUI2MenuGrid::AddMenuGridItem( AGUI2MenuGridItem& a_rMenuItem )
{
	if ( m_pLastMenuItem )
		a_rMenuItem.LinkMenuItemBefore( *m_pLastMenuItem );
	else
		m_pLastMenuItem = &a_rMenuItem;

	AddChildTail( a_rMenuItem );

	// Update max item dimensions
	TFLOAT fItemWidth, fItemHeight;
	a_rMenuItem.GetDimensions( fItemWidth, fItemHeight );

	if ( m_vecMaxItemDimensions.x < fItemWidth )
		m_vecMaxItemDimensions.x = fItemWidth;

	if ( m_vecMaxItemDimensions.y < fItemHeight )
		m_vecMaxItemDimensions.y = fItemHeight;

	a_rMenuItem.SetAttachment( AGUI2ATTACHMENT_TOPLEFT, AGUI2ATTACHMENT_TOPLEFT );
}

void AGUI2MenuGrid::SetFocusAt( AGUI2MenuGridItem* a_pMenuItem )
{
	if ( m_pFocusedMenuItem )
		m_pFocusedMenuItem->OnFocusLost();

	m_pFocusedMenuItem = a_pMenuItem;

	if ( a_pMenuItem )
		a_pMenuItem->OnFocus();
}

void AGUI2MenuGrid::ReflowChildren()
{
	TASSERT( m_pLastMenuItem != TNULL );

	const TFLOAT fMaxItemWidth  = ( m_fMaxItemWidth <= 0.0f ) ? m_vecMaxItemDimensions.x : m_fMaxItemWidth;
	const TFLOAT fMaxItemHeight = m_vecMaxItemDimensions.y;

	TFLOAT fX      = m_fPaddingLeft;
	TFLOAT fY      = 0.0f;
	TINT   iColumn = 0;
	TINT   iRow    = 0;

	AGUI2MenuGridItem* pMenuItem = m_pLastMenuItem;
	while ( TTRUE )
	{
		// Update position of the menu item
		pMenuItem->SetTranslation( fX, fY );

		// Update size of the item
		TFLOAT fColumnSize = m_pColumnsData[ iColumn++ ];
		pMenuItem->SetDimensions( fMaxItemWidth * fColumnSize, fMaxItemHeight );

		// Offset the coordinate
		fX += fMaxItemWidth * fColumnSize + m_fColumnGap;

		if ( iColumn >= m_iNumColumns )
		{
			// Switch to the next row
			iColumn = 0;

			fX = m_fPaddingLeft;
			fY += fMaxItemHeight * m_pRowsData[ iRow++ ] + m_fRowGap;
		}

		// Go to the next item if it exists
		if ( pMenuItem->m_pPrevMenuGridItem == m_pLastMenuItem ) break;
		pMenuItem = pMenuItem->m_pPrevMenuGridItem;
	}

	// Calculate menu dimensions
	AGUI2Element::SetDimensions( 0.0f, 0.0f );

	// Width
	for ( TINT i = 0; i < m_iNumColumns; i++ )
		AGUI2Element::SetWidth( fMaxItemWidth * m_pColumnsData[ i ] + AGUI2Element::GetWidth() + m_fColumnGap );

	// Height
	for ( TINT i = 0; i < m_iNumRows; i++ )
		AGUI2Element::SetHeight( fMaxItemHeight * m_pRowsData[ i ] + AGUI2Element::GetHeight() + m_fRowGap );

	AGUI2Element::SetWidth( AGUI2Element::GetWidth() - m_fColumnGap );
	AGUI2Element::SetHeight( AGUI2Element::GetHeight() - m_fRowGap );
}

TBOOL AGUI2MenuGrid::TriggerButtonPress( AGUI2MenuGridItem& a_rMenuItem )
{
	m_eActionFlags |= ACTIONFLAGS_OK;
	AGUI2MenuGridItem::COMMANDRESULT eCommandResult = a_rMenuItem.OnInputCommand( m_eActionFlags );
	m_eActionFlags                                  = ACTIONFLAGS_NONE;

	if ( eCommandResult == AGUI2MenuGridItem::COMMANDRESULT_OK )
	{
		if ( m_fnActivateCallback && m_pFocusedMenuItem && !m_bBlockActivation )
		{
			// Can activate
			ASoundManager::GetSingleton()->PlayCue( m_eSelectSound );

			if ( m_fnActivateCallback )
				m_fnActivateCallback( m_pCallbackUserData, m_pFocusedMenuItem );
		}

		return TTRUE;
	}

	return TFALSE;
}

TBOOL AGUI2MenuGrid::CreateGrid( TINT a_iNumRows, TINT a_iNumColumns, TFLOAT a_fColumnGap, TFLOAT a_fRowGap )
{
	m_iNumRows    = a_iNumRows;
	m_iNumColumns = a_iNumColumns;
	m_fColumnGap  = a_fColumnGap;
	m_fRowGap     = a_fRowGap;

	// Create columns data
	m_pColumnsData = new TFLOAT[ a_iNumColumns ];
	TUtil::Fill( m_pColumnsData, m_pColumnsData + a_iNumColumns, 1.0f );

	// Create rows data
	m_pRowsData = new TFLOAT[ a_iNumRows ];
	TUtil::Fill( m_pRowsData, m_pRowsData + a_iNumRows, 1.0f );

	return TTRUE;
}

void AGUI2MenuGrid::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bMouseStateDirty )
	{
		AGUI2Transform oElementInvTransform;

		if ( m_pLastMenuItem != TNULL )
		{
			TVector2 vecCursorPos = AGUI2::GetMouseCursor().GetCursorPos();
			m_pHoveredMenuItem    = TNULL;

			AGUI2MenuGridItem* pItem = m_pLastMenuItem;

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
							ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );

							if ( m_fnFocusCallback )
								m_fnFocusCallback( m_pCallbackUserData, m_pFocusedMenuItem, pItem );

							SetFocusAt( pItem );
						}

						m_pHoveredMenuItem = pItem;
						break;
					}
				}

				pItem = pItem->m_pPrevMenuGridItem;
				if ( pItem == m_pLastMenuItem ) break;
			}
		}

		m_bMouseStateDirty = TFALSE;
	}

	if ( m_pLastMenuItem != TNULL )
	{
		AGUI2MenuGridItem* pItem = m_pLastMenuItem;

		while ( TTRUE )
		{
			pItem->OnUpdate( a_fDeltaTime );

			pItem = pItem->m_pPrevMenuGridItem;
			if ( pItem == m_pLastMenuItem ) break;
		}
	}

	m_fTime += a_fDeltaTime;
}

TBOOL AGUI2MenuGrid::ProcessInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent )
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

TBOOL AGUI2MenuGrid::ProcessInputCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	TBOOL bHandled = TFALSE;

	if ( !TStringManager::String8Compare( a_pEvent->GetSource()->GetClass()->GetName(), "TInputDeviceMouse" ) )
	{
		m_eActionFlags |= ACTIONFLAGS_MOUSEUPDATED;
		m_vecMousePos.x = a_pEvent->GetMagnitudeFloat( 0 );
		m_vecMousePos.y = a_pEvent->GetMagnitudeFloat( 1 );
	}

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

	if ( m_eActionFlags & ACTIONFLAGS_MOUSEUPDATED )
	{
		AGUI2Transform oElementInvTransform;

		AGUI2MenuGridItem* pItem = m_pLastMenuItem;
		while ( TTRUE )
		{
			// Get inverse transform of the current UI element
			pItem->GetInvScreenTransform( oElementInvTransform );

			// Get cursor position that is relative to the UI element
			TVector2 vecElementRelCursorPos;
			oElementInvTransform.Transform( vecElementRelCursorPos, m_vecMousePos );

			if ( pItem->IsPointInside( vecElementRelCursorPos.x, vecElementRelCursorPos.y ) )
			{
				ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );

				if ( m_fnFocusCallback )
					m_fnFocusCallback( m_pCallbackUserData, m_pFocusedMenuItem, pItem );

				SetFocusAt( pItem );
				break;
			}

			// Go to the next item if it exists
			if ( pItem->m_pPrevMenuGridItem == m_pLastMenuItem ) break;
			pItem = pItem->m_pPrevMenuGridItem;
		}
	}

	TASSERT( m_pFocusedMenuItem != TNULL );

	AGUI2MenuGridItem::COMMANDRESULT eCommandResult = m_pFocusedMenuItem->OnInputCommand( m_eActionFlags );

	if ( eCommandResult == AGUI2MenuGridItem::COMMANDRESULT_CANCEL )
	{
		if ( m_pCancelItem )
		{
			// Can cancel, play back sound
			ASoundManager::GetSingleton()->PlayCue( m_eBackSound );

			// Call handler if it's specified
			if ( m_fnActivateCallback )
				m_fnActivateCallback( m_pCallbackUserData, m_pCancelItem );
		}
	}
	else if ( eCommandResult == AGUI2MenuGridItem::COMMANDRESULT_OK )
	{
		if ( m_fnActivateCallback && m_pFocusedMenuItem )
		{
			// Can activate
			ASoundManager::GetSingleton()->PlayCue( m_eSelectSound );

			if ( m_fnActivateCallback )
				m_fnActivateCallback( m_pCallbackUserData, m_pFocusedMenuItem );
		}
	}
	else if ( eCommandResult == AGUI2MenuGridItem::COMMANDRESULT_NONE )
	{
		// If element didn't process the command, check if the menu can handle this

		AGUI2MenuGridItem* pNewFocused;
		if ( m_eActionFlags & ACTIONFLAGS_UP )
		{
			pNewFocused = m_pFocusedMenuItem->m_pTopElement;

			while ( pNewFocused != TNULL && pNewFocused != m_pFocusedMenuItem && !pNewFocused->IsEnabled() )
				pNewFocused = pNewFocused->m_pTopElement;
		}
		else if ( m_eActionFlags & ACTIONFLAGS_DOWN )
		{
			pNewFocused = m_pFocusedMenuItem->m_pBottomElement;

			while ( pNewFocused != TNULL && pNewFocused != m_pFocusedMenuItem && !pNewFocused->IsEnabled() )
				pNewFocused = pNewFocused->m_pBottomElement;
		}
		else if ( m_eActionFlags & ACTIONFLAGS_LEFT )
		{
			pNewFocused = m_pFocusedMenuItem->m_pLeftElement;

			while ( pNewFocused != TNULL && pNewFocused != m_pFocusedMenuItem && !pNewFocused->IsEnabled() )
				pNewFocused = pNewFocused->m_pLeftElement;
		}
		else if ( m_eActionFlags & ACTIONFLAGS_RIGHT )
		{
			pNewFocused = m_pFocusedMenuItem->m_pRightElement;

			while ( pNewFocused != TNULL && pNewFocused != m_pFocusedMenuItem && !pNewFocused->IsEnabled() )
				pNewFocused = pNewFocused->m_pRightElement;
		}
		else
		{
			m_eActionFlags = ACTIONFLAGS_NONE;
			return bHandled;
		}

		// Change focus
		ASoundManager::GetSingleton()->PlayCue( m_eFocusSound );

		if ( m_fnFocusCallback )
			m_fnFocusCallback( m_pCallbackUserData, m_pFocusedMenuItem, pNewFocused );

		SetFocusAt( pNewFocused );
		bHandled = TTRUE;
	}

	m_eActionFlags = ACTIONFLAGS_NONE;
	return bHandled;
}
