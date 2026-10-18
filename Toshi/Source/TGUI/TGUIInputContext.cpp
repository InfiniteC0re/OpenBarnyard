#include "ToshiPCH.h"
#include "TGUIInputContext.h"
#include "TGUIScreen.h"
#include "TGUIManager.h"
#include "TGUIWidgetInfo.h"
#include "TGUIActionMap.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00364fc8)
TDEFINE_CLASS_NORUNTIME( TGUIInputContext );

// $NicktoonsPS2: FUNCTION 00365330
TGUIInputContext::ContextCommandInfo::~ContextCommandInfo()
{
}

// $NicktoonsPS2: FUNCTION 00364fe8
// $JPOG: FUNCTION TGuiInterface.dll+0000f1c0
TGUIInputContext::TGUIInputContext()
    : m_pScreen( TNULL )
    , m_aInputMappings( 10, 0 )
{
}

// $NicktoonsPS2: FUNCTION 00365048
// $JPOG: FUNCTION TGuiInterface.dll+0000f660
TGUIInputContext::~TGUIInputContext()
{
	m_oContextCommands.DeleteAll();
}

// For each context command: the label of the actions of the doodad that the widget class of the focused widget uses,
// then the first enabled action of the focused widget for the doodad
// $NicktoonsPS2: FUNCTION 00365140
// $JPOG: FUNCTION TGuiInterface.dll+0000f2c0
void TGUIInputContext::UpdateStatusDisplay()
{
	if ( !m_pScreen )
		return;

	TGUIWidget* pFocusedWidget = m_pScreen->GetFocusedWidget();

	if ( !pFocusedWidget )
		return;

	TGUIActionMap& rActionMap = TGUIManager::GetSingleton()->GetActionMap();
	TUINT          uiActions  = pFocusedWidget->GetWidgetInfo()->GetInputActions();

	for ( auto it = m_oContextCommands.Begin(); it != m_oContextCommands.End(); it++ )
	{
		TUINT uiCommandActions = rActionMap.TranslateDoodadToAction( it->iCommand );

		if ( uiActions & uiCommandActions )
			SetContextCommand( *it, 0, rActionMap.GetActionLabel( uiActions & uiCommandActions ) );

		TDList<TGUIWidgetAction>& rActions = m_pScreen->GetFocusedWidget()->GetActionList();

		for ( auto itAction = rActions.Begin(); itAction != rActions.End(); itAction++ )
		{
			if ( itAction->GetDoodadID() == it->iCommand && itAction->IsEnabled() )
			{
				SetContextCommand( *it, itAction->GetCommandID(), itAction->GetLabel() );
				break;
			}
		}
	}
}

// DOES_NOTHING
// $NicktoonsPS2: FUNCTION 00365360
// $JPOG: FUNCTION TGuiInterface.dll+0000ff60
void TGUIInputContext::FireWidgetAction( TGUIWidget* a_pWidget, TGUIWidgetAction* a_pAction )
{
}

// DOES_NOTHING
// $NicktoonsPS2: FUNCTION 00365368
void TGUIInputContext::Unknown_0x1C()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f200
void TGUIInputContext::AddContextCommand( TINT a_iCommand )
{
	m_oContextCommands.InsertTail( new ContextCommandInfo( a_iCommand ) );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f260
void TGUIInputContext::RemoveContextCommand( TINT a_iCommand )
{
	for ( auto it = m_oContextCommands.Begin(); it != m_oContextCommands.End(); it++ )
	{
		if ( it->iCommand == a_iCommand )
		{
			m_oContextCommands.Delete( it );
			return;
		}
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f5f0
void TGUIInputContext::AddInputMapping( TINT a_iCommand, TINT a_iDoodad )
{
	// Insert after the mappings of the same doodad
	TINT iLow  = 0;
	TINT iHigh = m_aInputMappings.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = a_iDoodad - m_aInputMappings[ iMiddle ].iDoodad;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else
		{
			iLow = iMiddle + 1;

			if ( iCompare == 0 )
			{
				while ( iLow <= iHigh && m_aInputMappings[ iLow ].iDoodad == a_iDoodad )
					iLow++;

				break;
			}
		}
	}

	InputMapping oMapping;
	oMapping.iDoodad  = a_iDoodad;
	oMapping.iCommand = a_iCommand;
	m_aInputMappings.InsertBefore( TArray<InputMapping>::Iterator( iLow, m_aInputMappings ), oMapping );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f5c0
TINT TGUIInputContext::FindInputMapping( const TInputInterface::InputEvent* a_pEvent )
{
	TINT iLow  = 0;
	TINT iHigh = m_aInputMappings.Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = a_pEvent->GetDoodad() - m_aInputMappings[ iMiddle ].iDoodad;

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			while ( iMiddle - 1 >= iLow && m_aInputMappings[ iMiddle - 1 ].iDoodad == a_pEvent->GetDoodad() )
				iMiddle--;

			return iMiddle;
		}
	}

	return -1;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f4d0
TINT TGUIInputContext::GetNextInputMapping( const TInputInterface::InputEvent* a_pEvent, TINT& a_rIndex )
{
	if ( a_rIndex == -1 )
		return -1;

	if ( m_aInputMappings[ a_rIndex ].iDoodad != a_pEvent->GetDoodad() )
		return -1;

	TINT iCommand = m_aInputMappings[ a_rIndex ].iCommand;

	if ( ++a_rIndex >= m_aInputMappings.Size() )
		a_rIndex = -1;

	return iCommand;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f420
TBOOL TGUIInputContext::IsInput( TINT a_iDoodad, TINT a_iCommand )
{
	TInputInterface::InputEvent oEvent( TNULL, a_iDoodad, TInputInterface::EVENT_TYPE_GONE_DOWN );
	TINT                        iIndex = FindInputMapping( &oEvent );

	while ( iIndex != -1 && m_aInputMappings[ iIndex ].iDoodad == a_iDoodad )
	{
		if ( m_aInputMappings[ iIndex ].iCommand == a_iCommand )
			return TTRUE;

		if ( ++iIndex >= m_aInputMappings.Size() )
			iIndex = -1;
	}

	return TFALSE;
}

TOSHI_NAMESPACE_END
