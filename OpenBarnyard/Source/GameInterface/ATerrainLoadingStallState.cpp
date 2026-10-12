#include "pch.h"
#include "ATerrainLoadingStallState.h"
#include "AGameStateController.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"
#include "Tasks/ARootTask.h"
#include "Terrain/ATerrainInterface.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00442410, CreateObject 004424f0, CreateObjectInPlace 00442550
TDEFINE_CLASS( ATerrainLoadingStallState );

// $Barnyard: FUNCTION 00442420
ATerrainLoadingStallState::ATerrainLoadingStallState()
{
}

// $Barnyard: FUNCTION 00442480
// $Barnyard: FUNCTION 00442590
ATerrainLoadingStallState::~ATerrainLoadingStallState()
{
}

// $Barnyard: FUNCTION 004426d0
TBOOL ATerrainLoadingStallState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( ATerrainInterface::GetSingleton()->IsLoaded() )
	{
		AGameStateController::GetSingleton()->PopCurrentGameState();
		return TTRUE;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004425b0
void ATerrainLoadingStallState::OnActivate()
{
	m_oText.Create( T2GUIFontManager::FindFont( "rekord18" ), 250.0f );
	m_oText.SetTextNoResize( ALocaleManager::Interface()->GetString( 287 ) );

	TFLOAT fWidth, fHeight;
	m_oText.GetDimensions( fWidth, fHeight );

	m_oBackground.SetDimensions( fWidth + 16.0f, fHeight + 16.0f );
	m_oBackground.AddChildTail( m_oText );
	m_oBackground.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oBackground.SetColour( 0x80000000 );

	ARootTask::GetSingleton()->SetPaused( TTRUE );
	T2GUI::GetContext()->GetRootElement()->AddChildTail( m_oBackground );

	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 00442680
void ATerrainLoadingStallState::OnDeactivate()
{
	m_oBackground.RemoveSelf();
	m_oText.RemoveSelf();
	ARootTask::GetSingleton()->SetPaused( TFALSE );

	AGameState::OnDeactivate();
}
