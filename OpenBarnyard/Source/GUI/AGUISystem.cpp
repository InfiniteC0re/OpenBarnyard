#include "pch.h"
#include "AGUISystem.h"
#include "Render/ARenderer.h"
#include "Memory/AMemory.h"
#include "Terrain/ATerrainInterface.h"

#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0059de70, CreateObject 0059e3c0, CreateObjectInPlace 0059e420
TDEFINE_CLASS( AGUISystem );

TPSTRING8_DECLARE( Start );
TPSTRING8_DECLARE( ParkManager );

// $Barnyard: FUNCTION 0059e0b0
AGUISystem::AGUISystem()
{
	m_Unk1          = 0;
	m_Unk2          = 0;
	m_Unk3          = 0;
	m_pStrings      = new Toshi::TPString8[ 2 ];
	m_pStrings[ 0 ] = TPS8( ParkManager );
	m_pStrings[ 1 ] = TPS8( Start );

#ifdef BARNYARD_LATEST
	m_bShowBackgroundPicture = TFALSE;

	// NOTE: the background picture is Wii data (bkg_Background)
	m_oBackgroundPicture.SetWiiLayout( TTRUE );
#endif
}

// $Barnyard: FUNCTION 0059e460
// $Barnyard: FUNCTION 0059e1d0
AGUISystem::~AGUISystem()
{
	delete[] m_pStrings;
}

// $Barnyard: FUNCTION 0059df20
TBOOL AGUISystem::OnCreate()
{
	m_RenderGUIListener.Connect(
	    ARenderer::GetSingleton()->GetRenderGUIEmitter(),
	    this,
	    OnGUIRender,
	    0xffff8000
	);

	auto pDisplayParams = Toshi::TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	for ( TUINT i = 0; i < NUM_RENDER_OBJECTS; i++ )
	{
		auto pViewport = new Toshi::TViewport( TTRUE );
		pViewport->SetX( 0.0f );
		pViewport->SetY( 0.0f );
		pViewport->SetWidth( TFLOAT( pDisplayParams->uiWidth ) );
		pViewport->SetHeight( TFLOAT( pDisplayParams->uiHeight ) );
		pViewport->SetMinZ( 0.0f );
		pViewport->SetMaxZ( 1.0f );
		pViewport->SetBackgroundColor( 0, 0, 0, 0 );
		pViewport->AllowBackgroundClear( TFALSE );
		pViewport->AllowDepthClear( TTRUE );
		pViewport->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

		m_pRenderObjects[ i ] = new PGUITRRenderObject( i );
		m_pRenderObjects[ i ]->SetViewport( pViewport );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0059deb0
void AGUISystem::OnDestroy()
{
	for ( TUINT i = 0; i < NUM_RENDER_OBJECTS; i++ )
	{
		delete m_pRenderObjects[ i ];
	}

	m_SomeListener.Disconnect();
}

// $Barnyard: FUNCTION 0059de80
// $BarnyardWiiJP: FUNCTION 80320d3c
TBOOL __stdcall AGUISystem::OnGUIRender( AGUISystem* a_pGUISystem, ARenderer* a_pRenderer, TINT* a_pUnknown )
{
	if ( *a_pUnknown == 0 )
	{
#ifdef BARNYARD_LATEST
		// NOTE: the Wii build also skips both pictures while a Wii-only flag (806b502c) is set
		if ( a_pGUISystem->m_bShowBackgroundPicture )
			a_pGUISystem->m_oBackgroundPicture.Render();
		else
#endif
			a_pGUISystem->m_MatLibPicture.Render();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0059def0
void AGUISystem::AllowBackgroundClear( TBOOL a_bAllow )
{
	if ( ms_pSingleton->m_pRenderObjects[ 0 ] != TNULL )
	{
		auto pViewport = ms_pSingleton->m_pRenderObjects[ 0 ]->GetViewport();

		if ( pViewport )
		{
			pViewport->AllowBackgroundClear( a_bAllow );
		}
	}
}

// $Barnyard: FUNCTION 0059e270
void AGUISystem::SetPicture( const Toshi::TPString8& a_rName )
{
	TBOOL bShowPicture = TFALSE;

	if ( !a_rName.IsEmpty() && a_rName.GetString8().Length() != 0 )
	{
		bShowPicture = ( a_rName != m_MatLibPicture.GetName() );
	}

	if ( bShowPicture )
	{
		if ( ATerrainInterface::IsSingletonCreated() )
		{
			ATerrainInterface::GetSingleton()->WaitUntilLoaded();
		}

		m_MatLibPicture.Create( a_rName );
		m_MatLibPicture.Flush();
		m_MatLibPicture.Cache();
		m_MatLibPicture.SetVisible();
	}
	else if ( a_rName.IsEmpty() || a_rName.GetString8().Length() == 0 )
	{
		m_MatLibPicture.Flush();
		m_MatLibPicture.Create( a_rName );
		m_MatLibPicture.SetHidden();
	}
}

// $Barnyard: FUNCTION 0059e350
void AGUISystem::ResetPicture()
{
	m_MatLibPicture.Flush();
	m_MatLibPicture.Create( TPString8() );
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 80321028
void AGUISystem::SetBackgroundPicture( const TPString8& a_rName )
{
	TBOOL bChanged = TFALSE;

	if ( !a_rName.IsEmpty() && a_rName.GetString8().Length() != 0 )
		bChanged = ( a_rName != m_oBackgroundPicture.GetName() );

	if ( bChanged )
	{
		if ( ATerrainInterface::IsSingletonCreated() )
			ATerrainInterface::GetSingleton()->WaitUntilLoaded();

		m_oBackgroundPicture.Create( a_rName );
		m_oBackgroundPicture.Flush();
		m_oBackgroundPicture.Cache();
	}
	else if ( a_rName.IsEmpty() || a_rName.GetString8().Length() == 0 )
	{
		m_oBackgroundPicture.Flush();
		m_oBackgroundPicture.Create( a_rName );
	}

	m_bShowBackgroundPicture = TFALSE;
}

// $BarnyardWiiJP: FUNCTION 803211b8
void AGUISystem::ShowBackgroundPicture( TBOOL a_bShow )
{
	m_bShowBackgroundPicture = a_bShow;

	if ( a_bShow )
		m_MatLibPicture.SetHidden();
}

#endif // BARNYARD_LATEST
