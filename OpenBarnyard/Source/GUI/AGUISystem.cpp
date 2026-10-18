#include "pch.h"
#include "AGUISystem.h"
#include "Render/ARenderer.h"
#include "Memory/AMemory.h"
#include "Terrain/ATerrainInterface.h"

#include <GUI/T2GUITextureSectionManager.h>

#ifdef BARNYARD_COMMUNITY_PATCH
#  include "AGUI2.h"
#  include "AGUISoundFactory.h"
#  include "AGUIAudioContext.h"
#  include "AGUIInputContext.h"
#  include "AGUITGUIDemo.h"
#  include "Assets/AAssetLoader.h"
#  include "Assets/AMaterialLibraryManager.h"

#  include <TGUI/TGUIManager.h>
#  include <TGUI/TGUIScreen.h>
#  include <TGUI/TGUIEvents.h>
#  include <TGUI/TGUIMemory.h>
#  include <Plugins/PGUI/PGUICore.h>
#  include <Plugins/PGUI/PGUIOwnerDraw.h>
#  include <Plugins/PGUI/PGUIMenuDialog.h>
#  include <TGUI/TGUIWindow.h>
#  include <Plugins/PGUIRenderer/PGUITRDisplayContext.h>
#  include <Plugins/PGUIRenderer/PGUITRViewportHandler.h>
#  include <File/TFile.h>
#  include <Plugins/PPropertyParser/PProperty.h>
#  include <Input/TInputDeviceController.h>
#  include <Input/TInputDeviceMouse.h>
#  include <Plugins/PGUI/PGUITextBox.h>
#endif

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

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Barnyard route (the TGUI hosting, see CreateTGUI)
	m_pTGUIDisplayContext = TNULL;
	m_pTGUIScreen         = TNULL;
	m_pTGUISoundFactory   = TNULL;
	m_pTGUIAudioContext   = TNULL;
	m_pTGUIInputContext   = TNULL;
	m_bTGUIMaterialsDirty = TFALSE;
	m_bTGUIMatLibLoaded   = TFALSE;
	m_bTGUIPointerMode    = TFALSE;
	m_iNumOwnerDraws      = 0;

	for ( TUINT i = 0; i < NUM_RENDER_OBJECTS; i++ )
		m_apTGUIViewportHandlers[ i ] = TNULL;

#  ifndef TOSHI_FINAL
	m_pTGUIDemo = TNULL;
#  endif
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
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Barnyard route (the TGUI hosting uses the render objects)
	DestroyTGUI();
#endif

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

#ifdef BARNYARD_COMMUNITY_PATCH
#  ifndef TOSHI_FINAL
		// NOTE: "-tguidemo" (development): shows the dialogs of Data/GUI/main.gui
		static TBOOL s_bTGUIDemoStarted = TFALSE;

		if ( !s_bTGUIDemoStarted && AGUITGUIDemo::IsRequested() && AAssetLoader::GetAssetTRB( AAssetType_Startup ) )
		{
			s_bTGUIDemoStarted = TTRUE;
			a_pGUISystem->Dev_ShowTGUIDemo( TTRUE );
		}
#  endif

		// NOTE: Barnyard route added on top: the TGUI screen, above the picture
		if ( a_pGUISystem->m_pTGUIScreen )
			a_pGUISystem->RenderTGUI();
#endif
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

#ifdef BARNYARD_COMMUNITY_PATCH

// NOTE: Barnyard route (never freed: the memory pile of the GUI keeps its blocks)
static TMemory::MemBlock* GetTGUIMemBlock()
{
	static TMemory::MemBlock* s_pMemBlock = TNULL;

	if ( !s_pMemBlock )
	{
		void* pMemory = malloc( AGUISystem::TGUI_MEMORY_SIZE );
		s_pMemBlock   = g_pMemory->CreateMemBlockInPlace( pMemory, AGUISystem::TGUI_MEMORY_SIZE, "TGUI" );
	}

	return s_pMemBlock;
}

AGUISystem::TGUIMemoryScope::TGUIMemoryScope()
{
	m_pOldMemBlock = g_pMemory->SetGlobalBlock( GetTGUIMemBlock() );
}

AGUISystem::TGUIMemoryScope::~TGUIMemoryScope()
{
	g_pMemory->SetGlobalBlock( m_pOldMemBlock );
}

// NOTE: Barnyard route added on top, as in JPOG's AGUISystem::OnCreate: the GUI manager uses the factories of a display
// context drawing with AGUI2's renderer, the screen is painted in the viewport of the render objects
TBOOL AGUISystem::CreateTGUI()
{
	TGUIMemoryScope oMemoryScope;

	if ( m_pTGUIScreen )
		return TTRUE;

	TASSERT( AGUI2::IsSingletonCreated() );

	PGUIInitialise();

	// The game keeps 100 properties without growing (AppBoot), the definition files need many more
	PProperties::SetupFreeList( 0, 64 );
	PProperty::SetupFreeList( 0, 64 );

	m_pTGUIDisplayContext = new PGUITRDisplayContext();
	m_pTGUIDisplayContext->Create( TRenderInterface::GetSingleton() );
	m_pTGUIDisplayContext->SetShader( AGUI2::GetRenderer() );

	m_pTGUISoundFactory = new AGUISoundFactory();
	m_pTGUIAudioContext = new AGUIAudioContext();
	m_pTGUIInputContext = new AGUIInputContext();

	TGUIManager* pManager = TGUIManager::IsSingletonCreated() ? TGUIManager::GetSingleton() : TGUIManager::CreateSingleton();
	pManager->SetTextureFactory( m_pTGUIDisplayContext->GetTextureFactory() );
	pManager->SetFontFactory( m_pTGUIDisplayContext->GetFontFactory() );
	pManager->SetSoundFactory( m_pTGUISoundFactory );

	// The fonts no [font] block defines are the fonts of Data/GUI/fonts.trb (Rekord18, Rekord26)
	TGUIManager::SetFontFallback( PGUITRFontFactory::CreateFontFromT2GUIFont );

	// The default action map has the pad's Context1 (Y) as cancel, Barnyard cancels with Context2 (B)
	pManager->GetActionMap().AddActionMapping( TInputDeviceController::BUTTON_CONTEXT_2, 2 );

	m_pTGUIScreen = new TGUIScreen();
	m_pTGUIScreen->SetDisplayContext( m_pTGUIDisplayContext );
	m_pTGUIScreen->SetAudioContext( m_pTGUIAudioContext );
	m_pTGUIScreen->SetInputContext( m_pTGUIInputContext );
	m_pTGUIScreen->SetFixedSize( TGUI_SCREEN_WIDTH, TGUI_SCREEN_HEIGHT );
	m_pTGUIScreen->SetState( TGUIWidget::State_Visible, TGUIWidget::State_Visible );

	for ( TUINT i = 0; i < NUM_RENDER_OBJECTS; i++ )
	{
		m_apTGUIViewportHandlers[ i ] = new PGUITRViewportHandler( *m_pRenderObjects[ i ] );
		m_apTGUIViewportHandlers[ i ]->SetScreen( m_pTGUIScreen );
	}

	UpdateTGUIScreenTransform();

	TINFO( "[TGUI] created the screen (%dx%d)\n", TGUI_SCREEN_WIDTH, TGUI_SCREEN_HEIGHT );
	return TTRUE;
}

void AGUISystem::DestroyTGUI()
{
	TGUIMemoryScope oMemoryScope;

#  ifndef TOSHI_FINAL
	if ( m_pTGUIDemo )
	{
		delete m_pTGUIDemo;
		m_pTGUIDemo = TNULL;
	}
#  endif

	if ( !m_pTGUIScreen )
		return;

	// The screen doesn't delete its children
	while ( !m_pTGUIScreen->GetChildren().IsEmpty() )
		HideTGUIDialog( m_pTGUIScreen->GetChildren().Head().Get() );

	for ( TUINT i = 0; i < NUM_RENDER_OBJECTS; i++ )
	{
		m_apTGUIViewportHandlers[ i ]->SetScreen( TNULL );
		delete m_apTGUIViewportHandlers[ i ];
		m_apTGUIViewportHandlers[ i ] = TNULL;
	}

	m_pTGUIScreen->SetInputContext( TNULL );
	delete m_pTGUIScreen;
	m_pTGUIScreen = TNULL;

	TGUIManager::SetFontFallback( TNULL );

	if ( TGUIManager::IsSingletonCreated() )
		TGUIManager::DestroySingleton();

	TGUISetUseMemoryPile( TTRUE );

	delete m_pTGUIInputContext;
	delete m_pTGUIAudioContext;
	delete m_pTGUISoundFactory;
	delete m_pTGUIDisplayContext;

	// The materials of the textures are gone with the display context
	if ( m_bTGUIMatLibLoaded )
	{
		AMaterialLibraryManager::GetSingleton()->UnrefLibrary( TPString8( "gui_dialogs" ), TTRUE );
		m_bTGUIMatLibLoaded = TFALSE;
	}

	m_pTGUIInputContext   = TNULL;
	m_pTGUIAudioContext   = TNULL;
	m_pTGUISoundFactory   = TNULL;
	m_pTGUIDisplayContext = TNULL;
}

TBOOL AGUISystem::LoadTGUIDefinitionFile( const TCHAR* a_szFileName )
{
	TGUIMemoryScope oMemoryScope;

	if ( !m_pTGUIScreen )
		return TFALSE;

	// The file is optional (a development file of the PC data)
	TFile* pFile = TFile::Create( a_szFileName, TFILEMODE_READ );

	if ( !pFile )
	{
		TWARN( "[TGUI] no definition file '%s'\n", a_szFileName );
		return TFALSE;
	}

	pFile->Destroy();

	// The textures of the dialogs (the frames of the buttons, Dialogs/frontend2.tga) are in Data/Matlibs/gui_Dialogs.ttl,
	// a material library retail never loads
	if ( !m_bTGUIMatLibLoaded )
	{
		AMaterialLibraryManager::GetSingleton()->LoadLibrary( TPString8( "gui_dialogs" ), TNULL, TTRUE );
		m_bTGUIMatLibLoaded = TTRUE;
	}

	// The definitions are allocated from the memory pile, the copies of the widgets made later aren't
	TGUISetUseMemoryPile( TTRUE );
	TBOOL bLoaded = TGUIManager::GetSingleton()->LoadDefinitionFile( a_szFileName );
	TGUISetUseMemoryPile( TFALSE );

	// The textures of the texture sections and of the fonts are textures of the material libraries (lib_startup)
	PGUITRTextureFactory* pTextureFactory = m_pTGUIDisplayContext->GetTextureFactory();
	pTextureFactory->ResolveMaterials();

	TINT iNumTextures = 0;
	TINT iNumResolved = 0;

	for ( TINT16 i = pTextureFactory->GetFirstTextureID(); i != -1; i = pTextureFactory->GetNextTextureID( i ) )
	{
		if ( !pTextureFactory->IsTextureIDValid( i ) )
			continue;

		iNumTextures++;

		if ( pTextureFactory->GetTexture( i )->IsValidated() )
			iNumResolved++;
		else
			TWARN( "[TGUI] texture '%s' isn't in the material libraries\n", pTextureFactory->GetTextureName( i ).GetString() );
	}

	TINFO( "[TGUI] loaded '%s': %d, %d/%d textures found\n", a_szFileName, bLoaded, iNumResolved, iNumTextures );
	return bLoaded;
}

TGUIWidget* AGUISystem::ShowTGUIDialog( const TPString8& a_rcName )
{
	TGUIMemoryScope oMemoryScope;

	if ( !m_pTGUIScreen )
		return TNULL;

	TGUIWidget* pDialog = TNULL;
	auto&       rList   = TGUIManager::GetSingleton()->GetTopLevelInstanceList();

	for ( auto it = rList.Begin(); it != rList.End(); it++ )
	{
		if ( it->pWidget->GetName() == a_rcName )
		{
			pDialog = it->pWidget->Clone();
			break;
		}
	}

	if ( !pDialog )
		return TNULL;

	// The material libraries loaded since are found
	m_pTGUIDisplayContext->GetTextureFactory()->ResolveMaterials();

	m_pTGUIScreen->AddWidget( pDialog, TFALSE );
	pDialog->Layout( -1, -1 );
	pDialog->Pack();

	if ( pDialog->IsA( &TGetClass( TGUIWindow ) ) && !pDialog->IsA( &TGetClass( PGUIMenuDialog ) ) )
		BuildTGUIFocusMap( TSTATICCAST( TGUIWindow, pDialog ) );

	ConnectOwnerDraws( pDialog );
	m_pTGUIScreen->ChangeWidgetFocus( pDialog, TFALSE );

	return pDialog;
}

void AGUISystem::HideTGUIDialog( TGUIWidget* a_pDialog )
{
	TGUIMemoryScope oMemoryScope;

	if ( !m_pTGUIScreen || !a_pDialog )
		return;

	DisconnectOwnerDraws( a_pDialog );

	if ( m_pTGUIScreen->GetFocusedWidget() && m_pTGUIScreen->GetFocusedWidget() != m_pTGUIScreen )
		m_pTGUIScreen->ChangeWidgetFocus( m_pTGUIScreen, TTRUE );

	m_pTGUIScreen->RemoveWidget( a_pDialog );
	delete a_pDialog;
}

#  ifndef TOSHI_FINAL

void AGUISystem::Dev_ShowTGUIDemo( TBOOL a_bShow )
{
	if ( a_bShow && !m_pTGUIDemo )
	{
		m_pTGUIDemo = new AGUITGUIDemo( this );
	}
	else if ( !a_bShow && m_pTGUIDemo )
	{
		delete m_pTGUIDemo;
		m_pTGUIDemo = TNULL;
	}
}

#  endif

// NOTE: Barnyard route added on top (called by AGameStateController::ProcessInput): the events of the doodads the action
// map knows are kept from the game state while the screen shows a widget
TBOOL AGUISystem::NotifyTGUIInputEvent( const TInputInterface::InputEvent* a_pEvent )
{
	TGUIMemoryScope oMemoryScope;

#  ifndef TOSHI_FINAL
	if ( m_pTGUIDemo && m_pTGUIDemo->ProcessInput( a_pEvent ) )
		return TTRUE;
#  endif

	if ( !m_pTGUIScreen || m_pTGUIScreen->GetChildren().IsEmpty() )
		return TFALSE;

	if ( m_bTGUIPointerMode )
	{
		// The mouse events always update the pointer, the widgets keep the ones they use and the ones over them
		if ( a_pEvent->GetSource() && a_pEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
		{
			TBOOL bUsed = m_pTGUIScreen->NotifyInputEvent( const_cast<TInputInterface::InputEvent*>( a_pEvent ) );

			// The game state sees the moves too
			if ( a_pEvent->GetDoodad() == TInputDeviceMouse::AXIS_CURSOR )
				return TFALSE;

			return bUsed || m_pTGUIScreen->GetWidgetUnderPointer() != m_pTGUIScreen;
		}

		// The keyboard only goes to a text box that has the focus
		TGUIWidget* pFocused = m_pTGUIScreen->GetFocusedWidget();

		if ( !pFocused || !pFocused->IsA( &TGetClass( PGUITextBox ) ) )
			return TFALSE;

		m_pTGUIScreen->NotifyInputEvent( const_cast<TInputInterface::InputEvent*>( a_pEvent ) );
		return TTRUE;
	}

	TBOOL bUsed = m_pTGUIScreen->NotifyInputEvent( const_cast<TInputInterface::InputEvent*>( a_pEvent ) );
	return bUsed || TGUIManager::GetSingleton()->GetActionMap().TranslateDoodadToAction( a_pEvent->GetDoodad() ) != 0;
}

TBOOL __stdcall AGUISystem::OnOwnerDrawPaint( PGUIOwnerDraw* a_pOwnerDraw, TGUIWidget* a_pWidget, TGUIWidgetPaintEvent* a_pEvent )
{
	a_pOwnerDraw->Paint( a_pEvent->pDisplayContext );
	return TTRUE;
}

TBOOL __stdcall AGUISystem::FindOwnerDrawCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	AGUISystem* pGUISystem = TSTATICCAST( AGUISystem, a_pUserData );

	if ( a_pWidget->IsA( &TGetClass( PGUIOwnerDraw ) ) && pGUISystem->m_iNumOwnerDraws < MAX_OWNER_DRAWS )
	{
		PGUIOwnerDraw*     pOwnerDraw = TSTATICCAST( PGUIOwnerDraw, a_pWidget );
		OwnerDrawListener* pListener  = new OwnerDrawListener();
		pListener->Connect( pOwnerDraw->GetPaintEmitter(), pOwnerDraw, OnOwnerDrawPaint, 0 );

		pGUISystem->m_apOwnerDraws[ pGUISystem->m_iNumOwnerDraws ]         = pOwnerDraw;
		pGUISystem->m_apOwnerDrawListeners[ pGUISystem->m_iNumOwnerDraws ] = pListener;
		pGUISystem->m_iNumOwnerDraws++;
	}

	return TTRUE;
}

void AGUISystem::ConnectOwnerDraws( TGUIWidget* a_pWidget )
{
	a_pWidget->RecurseTree( FindOwnerDrawCallback, TNULL, TNULL, this );
}

void AGUISystem::DisconnectOwnerDraws( TGUIWidget* a_pWidget )
{
	for ( TINT i = m_iNumOwnerDraws - 1; i >= 0; i-- )
	{
		TBOOL bInWidget = TFALSE;

		for ( TGUIWidget* pWidget = m_apOwnerDraws[ i ]; pWidget && !bInWidget; pWidget = pWidget->GetParentWidget() )
			bInWidget = ( pWidget == a_pWidget );

		if ( !bInWidget )
			continue;

		m_apOwnerDrawListeners[ i ]->Disconnect();
		delete m_apOwnerDrawListeners[ i ];

		m_iNumOwnerDraws--;
		m_apOwnerDraws[ i ]         = m_apOwnerDraws[ m_iNumOwnerDraws ];
		m_apOwnerDrawListeners[ i ] = m_apOwnerDrawListeners[ m_iNumOwnerDraws ];
	}
}

void AGUISystem::OnTGUIMaterialLibrariesChanged( TBOOL a_bUnloading )
{
	if ( !m_pTGUIScreen )
		return;

	// The materials of the textures that are about to be destroyed mustn't be used anymore
	if ( a_bUnloading )
	{
		PGUITRTextureFactory* pTextureFactory = m_pTGUIDisplayContext->GetTextureFactory();

		for ( TINT16 i = pTextureFactory->GetFirstTextureID(); i != -1; i = pTextureFactory->GetNextTextureID( i ) )
		{
			if ( pTextureFactory->IsTextureIDValid( i ) && !pTextureFactory->GetTexture( i )->GetName().IsEmpty() )
				pTextureFactory->GetTexture( i )->Invalidate();
		}
	}

	m_bTGUIMaterialsDirty = TTRUE;
}

// The widgets that take the focus and their centres (relative to the window)
struct TGUIFocusableList
{
	static constexpr TINT MAX_WIDGETS = 128;

	TGUIWindow* pWindow;
	TINT        iNumWidgets;
	TGUIWidget* apWidgets[ MAX_WIDGETS ];
	TINT        aiX[ MAX_WIDGETS ];
	TINT        aiY[ MAX_WIDGETS ];
};

TBOOL __stdcall AGUISystem::FindFocusableCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	TGUIFocusableList* pList = TSTATICCAST( TGUIFocusableList, a_pUserData );

	if ( a_pWidget == pList->pWindow || pList->iNumWidgets >= TGUIFocusableList::MAX_WIDGETS )
		return TTRUE;

	if ( !a_pWidget->AcceptsFocus() || !a_pWidget->CanFocus() )
		return TTRUE;

	TINT iX = a_pWidget->GetWidth() / 2;
	TINT iY = a_pWidget->GetHeight() / 2;

	for ( TGUIWidget* pWidget = a_pWidget; pWidget && pWidget != pList->pWindow; pWidget = pWidget->GetParentWidget() )
	{
		iX += pWidget->GetXPos();
		iY += pWidget->GetYPos();
	}

	pList->apWidgets[ pList->iNumWidgets ] = a_pWidget;
	pList->aiX[ pList->iNumWidgets ]       = iX;
	pList->aiY[ pList->iNumWidgets ]       = iY;
	pList->iNumWidgets++;

	return TTRUE;
}

// NOTE: Barnyard route added on top (see the declaration)
void AGUISystem::BuildTGUIFocusMap( TGUIWindow* a_pWindow )
{
	if ( a_pWindow->GetFocusMapSize() != 0 )
		return;

	TGUIFocusableList* pList = new TGUIFocusableList;
	pList->pWindow           = a_pWindow;
	pList->iNumWidgets       = 0;
	a_pWindow->RecurseTree( FindFocusableCallback, TNULL, TNULL, pList );

	TINT iFirst = -1;

	for ( TINT i = 0; i < pList->iNumWidgets; i++ )
	{
		// The default focus is the widget at the top (then at the left)
		if ( iFirst < 0 || pList->aiY[ i ] < pList->aiY[ iFirst ] || ( pList->aiY[ i ] == pList->aiY[ iFirst ] && pList->aiX[ i ] < pList->aiX[ iFirst ] ) )
			iFirst = i;

		for ( TINT iDirection = TGUIWidget::NeighbourDirection_North; iDirection <= TGUIWidget::NeighbourDirection_West; iDirection++ )
		{
			TINT iBest      = -1;
			TINT iBestScore = 0;

			for ( TINT j = 0; j < pList->iNumWidgets; j++ )
			{
				if ( j == i )
					continue;

				TINT iDeltaX = pList->aiX[ j ] - pList->aiX[ i ];
				TINT iDeltaY = pList->aiY[ j ] - pList->aiY[ i ];
				TINT iAlong;
				TINT iAcross;

				switch ( iDirection )
				{
					case TGUIWidget::NeighbourDirection_North: iAlong = -iDeltaY, iAcross = iDeltaX; break;
					case TGUIWidget::NeighbourDirection_East: iAlong = iDeltaX, iAcross = iDeltaY; break;
					case TGUIWidget::NeighbourDirection_South: iAlong = iDeltaY, iAcross = iDeltaX; break;
					default: iAlong = -iDeltaX, iAcross = iDeltaY; break;
				}

				// In front of the widget, the closest to the line of the direction first
				if ( iAlong <= 0 )
					continue;

				TINT iScore = iAlong + 3 * TMath::Abs( iAcross );

				if ( iBest < 0 || iScore < iBestScore )
				{
					iBest      = j;
					iBestScore = iScore;
				}
			}

			if ( iBest >= 0 )
				a_pWindow->SetFocusNeighbour( pList->apWidgets[ i ], TGUIWidget::NeighbourDirection( iDirection ), pList->apWidgets[ iBest ] );
		}
	}

	if ( iFirst >= 0 )
		a_pWindow->SetDefaultFocus( pList->apWidgets[ iFirst ] );

	delete pList;
}

// The display context scales the screen to the viewport (as PGUITRViewportHandler::SetScreen does), the screen transform
// maps the viewport to AGUI2's root element (centred, of the root's size)
void AGUISystem::UpdateTGUIScreenTransform()
{
	TViewport* pViewport = m_pRenderObjects[ 0 ]->GetViewport();

	TFLOAT fRootWidth;
	TFLOAT fRootHeight;
	AGUI2::GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

	m_pTGUIDisplayContext->SetScaleX( pViewport->GetWidth() / TFLOAT( m_pTGUIScreen->GetWidth() ) );
	m_pTGUIDisplayContext->SetScaleY( pViewport->GetHeight() / TFLOAT( m_pTGUIScreen->GetHeight() ) );

	T2GUITransform oTransform;
	oTransform.Set(
	    TVector2( fRootWidth / pViewport->GetWidth(), 0.0f ),
	    TVector2( 0.0f, fRootHeight / pViewport->GetHeight() ),
	    TVector2( fRootWidth * -0.5f, fRootHeight * -0.5f )
	);

	m_pTGUIDisplayContext->SetScreenTransform( oTransform );
}

// Paints the screen like the events of the viewport would (the first render object restarts the painting)
void AGUISystem::RenderTGUI()
{
	TGUIMemoryScope oMemoryScope;

#  ifndef TOSHI_FINAL
	if ( m_pTGUIDemo )
		m_pTGUIDemo->Update();
#  endif

	if ( m_pTGUIScreen->GetChildren().IsEmpty() )
		return;

	if ( m_bTGUIMaterialsDirty )
	{
		m_pTGUIDisplayContext->GetTextureFactory()->ResolveMaterials();
		m_bTGUIMaterialsDirty = TFALSE;
	}

	UpdateTGUIScreenTransform();

	PGUITRViewportHandler* pHandler  = m_apTGUIViewportHandlers[ 0 ];
	TViewport*             pViewport = pHandler->GetViewport();

	PGUITRViewportHandler::OnUpdateEvent( pHandler, pViewport, TNULL );
	PGUITRViewportHandler::OnBeginRenderEvent( pHandler, pViewport, TNULL );
	PGUITRViewportHandler::OnRenderEvent( pHandler, pViewport, TNULL );
	PGUITRViewportHandler::OnEndRenderEvent( pHandler, pViewport, TNULL );
}

#endif // BARNYARD_COMMUNITY_PATCH
