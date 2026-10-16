#include "pch.h"
#include "AGUI2.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Cameras/ACameraManager.h"
#include "Sound/ASoundManager.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "GameInterface/AHackedQuickSave.h"

#include <Toshi/TSystem.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/DX8/AGUI2Renderer_DX8.h"
#endif // TOSHI_SKU_WINDOWS

#ifdef BARNYARD_LATEST
#  include "GUI/XUI/AGUI2XUIResource.h"
#  include <Platform/Windows/TNativeFile_Win.h>
#  include "GUI/XUI/AGUI2XUIScene.h"
#  include "GUI/XUI/AGUI2XUICanvas.h"
#  include "GUI/XUI/AGUI2XUIText.h"
#  include "GUI/XUI/AGUI2XUIShapeCache.h"
#endif // BARNYARD_LATEST

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00635350, CreateObject 00635b40, CreateObjectInPlace 00635ba0
TDEFINE_CLASS( AGUI2 );

// $Barnyard: FUNCTION 00635440
AGUI2::AGUI2()
{
	m_pRootElement     = TNULL;
	m_bShowMouseCursor = TFALSE;

	// NOTE: retail initialises the four debug overlay flags to TFALSE
	m_bShowMemStatsInfo = TTRUE;
	m_bShowFPSInfo      = TTRUE;
	m_bShowPlayerInfo   = TTRUE;
	m_bShowTexturesInfo = TTRUE;

#ifdef BARNYARD_LATEST
	// Wii: 803ea648
	m_pFocusedXUIScene = TNULL;
	m_pXUIUnk1         = TNULL;
	m_pXUIUnk2         = TNULL;
	m_pXUIUnk3         = TNULL;
	m_pXUICanvas       = TNULL;
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 00635500
// $Barnyard: FUNCTION 00635be0
AGUI2::~AGUI2()
{
}

// $Barnyard: FUNCTION 006355a0
TBOOL AGUI2::OnCreate()
{
	T2GUI::CreateContext()->SetRenderer( new AGUI2RendererDX8 );

	m_pRootElement = T2GUI::GetContext()->GetRootElement();
	m_pRootElement->SetDimensions( 800.0f, 600.0f );
	m_pRootElement->SetPostRenderCallback( MainPostRenderCallback );

	T2GUIFontManager::Open( "data/gui/fonts.trb" );
	T2GUITextureSectionManager::Open( "data/gui/texsec.trb", AAssetLoader::GetAssetTRB( AAssetType_Startup ) );
	T2GUITextureSectionManager::CreateMaterials();

	auto pDebugCanvas = T2GUI::GetContext()->GetDebugCanvas();
	auto pFont        = T2GUIFontManager::FindFont( "Rekord18" );

	// FPS stats
	m_oFPS.Create( pFont, 600.0f );
	m_oFPS.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oFPS.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oFPS.SetTransform( 0.0f, 0.0f );

	if ( m_bShowFPSInfo ) m_oFPS.Show();
	else m_oFPS.Hide();
	pDebugCanvas->AddChildTail( m_oFPS );

	// Player stats
	m_oPlayerInfo.Create( pFont, 600.0f );
	m_oPlayerInfo.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oPlayerInfo.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oPlayerInfo.SetTransform( 0.0f, 24.0f );

	if ( m_bShowPlayerInfo ) m_oPlayerInfo.Show();
	else m_oPlayerInfo.Hide();
	pDebugCanvas->AddChildTail( m_oPlayerInfo );

	// Textures stats
	m_oTexturesInfo.Create( pFont, 600.0f );
	m_oTexturesInfo.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oTexturesInfo.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oTexturesInfo.SetTransform( 0.0f, 48.0f );

	if ( m_bShowTexturesInfo ) m_oTexturesInfo.Show();
	else m_oTexturesInfo.Hide();
	pDebugCanvas->AddChildTail( m_oTexturesInfo );

	// Memory stats
	m_oMemStats.Create( pFont, 600.0f );
	m_oMemStats.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oMemStats.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oMemStats.SetTransform( 0.0f, 72.0f );

	if ( m_bShowMemStatsInfo ) m_oMemStats.Show();
	else m_oMemStats.Hide();
	pDebugCanvas->AddChildTail( m_oMemStats );

	// Mouse cursor
	m_oMouseCursor.Create( "Pointer_Up", "Pointer_Down", 0.5f );

#ifdef BARNYARD_LATEST
	// Wii: end of 803ea88c
	new AGUI2XUIShapeCache();

	// Wii: the layers (set up at the start of 803ea88c, right after the root element)
	TFLOAT fRootWidth  = m_pRootElement->GetWidth();
	TFLOAT fRootHeight = m_pRootElement->GetHeight();
	m_oLayer1.SetDimensions( fRootWidth, fRootHeight );
	m_oSafeLayer.SetDimensions( fRootWidth - 32.0f, fRootHeight - 32.0f );
	m_oLayer3.SetDimensions( fRootWidth, fRootHeight );
	m_pRootElement->AddChildTail( m_oLayer1 );
	m_pRootElement->AddChildTail( m_oSafeLayer );
	m_pRootElement->AddChildTail( m_oLayer3 );

	// The XUI canvas covers the screen: as big as the third layer, in the safe layer and moved back by its border
	m_pXUICanvas = new AGUI2XUICanvas();
	m_pXUICanvas->SetDimensions( m_oLayer3.GetWidth(), m_oLayer3.GetHeight() );
	m_oSafeLayer.AddChildTail( *m_pXUICanvas );
	m_pXUICanvas->SetTransform( -16.0f, -16.0f, 0.0f );

	// NOTE: the Wii root element is as big as the screen, 640x480, and the XUI scenes are made for
	// that size. The PC root element is 800x600, so the canvas keeps the Wii size and is scaled up.
	T2GUITransform oCanvasTransform = m_pXUICanvas->GetTransform();
	oCanvasTransform.SetScale( fRootWidth / XUI_SCREEN_WIDTH, fRootHeight / XUI_SCREEN_HEIGHT );
	m_pXUICanvas->SetDimensions( XUI_SCREEN_WIDTH, XUI_SCREEN_HEIGHT );
	m_pXUICanvas->SetTransform( oCanvasTransform );

	T2String8::Copy( AGUI2XUIText::ms_szDefaultFont, "Arial_14" );
#endif // BARNYARD_LATEST

	return TTRUE;
}

// $Barnyard: FUNCTION 006357d0
TBOOL AGUI2::OnUpdate( TFLOAT a_fDeltaTime )
{
#if defined( BARNYARD_LATEST ) && defined( BARNYARD_COMMUNITY_PATCH )
	// Development: "-devxui <scene>" loads Data/XUI/<language>/<scene>.trb 5 seconds after the start,
	// creates its first scene on the XUI canvas and logs the element tree (lines tagged [devxui])
	static TFLOAT s_fDevXUITimer = 0.0f;

	if ( s_fDevXUITimer >= 0.0f && TStringManager::String8FindString( GetCommandLineA(), "-devxui " ) )
	{
		s_fDevXUITimer += a_fDeltaTime;

		if ( s_fDevXUITimer >= 5.0f )
		{
			s_fDevXUITimer = -1.0f;
			DevLoadXUIScene( TStringManager::String8FindString( GetCommandLineA(), "-devxui " ) + 8 );
		}
	}
#endif

	if ( m_bShowMemStatsInfo )
	{
		// NOTE: retail walks the heap every frame. Here it's refreshed once a second, since the overlay
		// is enabled by default (see the constructor) and the walk is slow in debug builds.
		static TFLOAT s_UpdateTimer = 10.0f;
		s_UpdateTimer += a_fDeltaTime;

		if ( s_UpdateTimer >= 1.0f )
		{
			TMemory::MemInfo memInfo;
			g_pMemory->GetMemInfo( memInfo, TNULL );

			ASoundManager::MemoryStats oSoundStats;
			ASoundManager::GetSingleton()->GetMemoryStats( &oSoundStats );

			// Usage of the quick save buffers
			TUINT uiTransientUsed = 0;
			TUINT uiTransientSize = 0;
			TUINT uiNormalUsed    = 0;
			TUINT uiNormalSize    = 0;

			if ( AHackedQuickSave* pQuickSave = AHackedQuickSave::GetSingleton() )
			{
				AHackedQuickSave::Buffer* pNormal    = pQuickSave->GetBuffer( AHackedQuickSave::BUFFER_WORLD );
				AHackedQuickSave::Buffer* pTransient = pQuickSave->GetBuffer( AHackedQuickSave::BUFFER_PROFILE );

				uiTransientUsed = pTransient->GetCursor();
				uiTransientSize = pTransient->GetSize();
				uiNormalUsed    = pNormal->GetCursor();
				uiNormalSize    = pNormal->GetSize();
			}

			// NOTE: retail prints uiUnknown (always 0) as the second sound value
			TStringManager::String16Format(
			    m_wszMemStats,
			    TARRAYSIZE( m_wszMemStats ),
			    L"Mem Used: %u, Free: %u\nHoles: %u, Largest: %u\nSound: Sys: %u / %u\nSave: Transient Used: %u, Free: %u\nSave: Normal Used: %u, Free: %u",
			    memInfo.m_uiLogicTotalUsed,
			    memInfo.m_uiLogicTotalFree,
			    memInfo.m_iNumHoles,
			    memInfo.m_uiLargestHole,
			    oSoundStats.uiCurrentAlloced,
			    oSoundStats.uiUnknown,
			    uiTransientUsed,
			    uiTransientSize - uiTransientUsed,
			    uiNormalUsed,
			    uiNormalSize - uiNormalUsed
			);

			m_oMemStats.SetText( m_wszMemStats );
			s_UpdateTimer = 0.0f;
		}
	}

	if ( m_bShowFPSInfo )
	{
		static TFLOAT s_FPS         = 0.0f;
		static TFLOAT s_UpdateTimer = 0.0f;
		s_UpdateTimer += a_fDeltaTime;

		if ( s_UpdateTimer > 0.1f )
		{
			s_FPS         = g_oSystemManager.GetFPS();
			s_UpdateTimer = 0.0f;
		}

		TStringManager::String16Format(
		    m_wszFPS,
		    TARRAYSIZE( m_wszFPS ),
		    L"FPS: %0.02f",
		    s_FPS
		);

		m_oFPS.SetText( m_wszFPS );

		if ( s_FPS >= 15.0f )
		{
			m_oFPS.SetColour( s_FPS < 20.0f ? 0xffffff00 : 0xffffffff );
		}
		else
		{
			m_oFPS.SetColour( 0xffff0000 );
		}
	}

	if ( m_bShowPlayerInfo )
	{
		TBOOL bHasPlayerInfo = TFALSE;

		TWCHAR wszPlayerInfo[ 128 ];
		TStringManager::String16Format( wszPlayerInfo, TARRAYSIZE( wszPlayerInfo ), L"" );

		if ( APlayerManager::GetSingleton() )
		{
			AUnit* pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

			if ( pPlayerUnit )
			{
				TVector4 vPlayerPos = *pPlayerUnit->GetSteer()->GetPosition();
				bHasPlayerInfo      = TTRUE;

				TStringManager::String16Format(
				    wszPlayerInfo,
				    TARRAYSIZE( wszPlayerInfo ),
				    L"%s: (%0.02f, %0.02f, %0.02f)",
				    L"Plyr",
				    vPlayerPos.x,
				    vPlayerPos.y,
				    vPlayerPos.z
				);
			}
		}

		TWCHAR wszCameraInfo[ 128 ];
		TStringManager::String16Format( wszCameraInfo, TARRAYSIZE( wszCameraInfo ), L"" );

		auto pCameraManager = ACameraManager::GetSingleton();

		if ( TNULL != pCameraManager )
		{
			auto pCamera = pCameraManager->GetCurrentCamera();

			if ( pCamera )
			{
				auto vCamTranslation = pCamera->GetMatrix().GetTranslation();

				TStringManager::String16Format(
				    wszCameraInfo,
				    TARRAYSIZE( wszCameraInfo ),
				    L"%s%s: (%0.02f, %0.02f, %0.02f)",
				    bHasPlayerInfo ? L"   " : L"",
				    L"Cam",
				    vCamTranslation.x,
				    vCamTranslation.y,
				    vCamTranslation.z
				);
			}
		}

		TStringManager::String16Format(
		    m_wszPlayerInfo,
		    TARRAYSIZE( m_wszPlayerInfo ),
		    L"%s%s",
		    wszPlayerInfo,
		    wszCameraInfo
		);

		m_oPlayerInfo.SetText( m_wszPlayerInfo );
	}

	if ( m_bShowTexturesInfo )
	{
		auto iNumFreeTextures = AMaterialLibraryManager::GetSingleton()->GetNumFreeTextures();
		auto iNumUsedTextures = AMaterialLibraryManager::GetSingleton()->GetNumUsedTextures();

		TStringManager::String16Format(
		    m_wszTexturesInfo,
		    TARRAYSIZE( m_wszTexturesInfo ),
		    L"Textures: (F:%d,U:%d)/%d",
		    iNumFreeTextures,
		    iNumUsedTextures,
		    AMaterialLibraryManager::MAX_NUM_TEXTURES
		);

		m_oTexturesInfo.SetText( m_wszTexturesInfo );

		if ( iNumFreeTextures >= 20 )
		{
			m_oTexturesInfo.SetColour( iNumFreeTextures < 50 ? 0xffffff00 : 0xffffffff );
		}
		else
		{
			m_oTexturesInfo.SetColour( 0xffff0000 );
		}
	}

	m_oMouseCursor.Update();
	T2GUI::GetContext()->Tick( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 00635360
void AGUI2::OnDestroy()
{
	m_oMouseCursor.RemoveSelf();
	m_oFPS.RemoveSelf();
	m_oMemStats.RemoveSelf();
	m_oPlayerInfo.RemoveSelf();
	m_oTexturesInfo.RemoveSelf();

#ifdef BARNYARD_LATEST
	// Wii: 803eb014
	m_pXUICanvas->RemoveSelf();

	if ( m_pXUICanvas )
		delete m_pXUICanvas;

	DeleteXUIScenes();
	UnloadXUIMaterialLibraries();
	CloseXUIFonts();
	m_pFocusedXUIScene = TNULL;
#endif // BARNYARD_LATEST

	T2GUIFontManager::Close( "data/gui/fonts.trb" );
	T2GUITextureSectionManager::DestroyMaterials();
	T2GUITextureSectionManager::Close();

	T2GUIRenderer* pRenderer = T2GUI::GetRenderer();

	if ( pRenderer )
		delete pRenderer;

	T2GUI::DestroyContext();
	TTask::OnDestroy();
}

// $Barnyard: FUNCTION 006353f0
void AGUI2::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	m_pRootElement->GetDimensions( a_rWidth, a_rHeight );
}

// $Barnyard: FUNCTION 00635410
void AGUI2::MainPostRenderCallback()
{
	if ( AGUI2::GetSingleton()->IsCursorVisible() )
	{
		auto& mouseCursor = AGUI2::GetSingleton()->m_oMouseCursor;

		mouseCursor.PreRender();
		mouseCursor.Render();
		mouseCursor.PostRender();
	}
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 803eb74c
void AGUI2::AddXUIScene( AGUI2XUIScene* a_pScene )
{
	m_oXUI.AddScene( a_pScene );
	m_pXUICanvas->AddXUIChild( a_pScene );
}

// $BarnyardWiiJP: FUNCTION 803eb7a0
void AGUI2::RemoveXUIScene( AGUI2XUIScene* a_pScene )
{
	if ( m_pFocusedXUIScene == a_pScene )
		m_pFocusedXUIScene = TNULL;

	a_pScene->RemoveSelf();
	m_oXUI.DeleteScene( a_pScene );
}

// $BarnyardWiiJP: FUNCTION 803eb804
TBOOL AGUI2::LoadXUIResource( const TCHAR* a_szName )
{
	TIMPLEMENT_D( "Use the asset TRB when there's no language" );

	TTRB* pTRB = TNULL;

#  if 0 // GHIDRA 803eb804
	if ( AGUI2::GetLanguage() == TNULL )
		pTRB = sub_800F4D60( a_szName );
#  endif

	AGUI2XUI::LoadResource( a_szName, &m_oXUI, TFALSE, TTRUE, pTRB );
	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 803eb888
AGUI2XUIScene* AGUI2::CreateXUIScene( AGUI2XUIResource* a_pResource, TUINT32 a_uiIndex )
{
	AGUI2XUIScene*     pScene = TNULL;
	XURXUIElementData* pData  = a_pResource->FindScene( a_uiIndex );

	if ( pData )
	{
		// Scenes are created once
		pScene = m_oXUI.FindScene( a_pResource->GetString( pData->m_usId ) );

		if ( !pScene )
		{
			pScene = a_pResource->CreateScene( a_uiIndex );

			if ( pScene )
				AddXUIScene( pScene );
		}
	}

	return pScene;
}

// $BarnyardWiiJP: FUNCTION 803eb934
void AGUI2::DeleteXUIResources()
{
	m_oXUI.DeleteResources();
}

// $BarnyardWiiJP: FUNCTION 803eb93c
void AGUI2::UnloadXUIMaterialLibraries()
{
	if ( AGUI2XUIText::ms_pIconMaterial )
	{
		T2GUI::GetRenderer()->DestroyMaterial( AGUI2XUIText::ms_pIconMaterial );
		AGUI2XUIText::ms_pIconMaterial = TNULL;
	}

	m_oXUI.UnloadMaterialLibraries();
}

// $BarnyardWiiJP: FUNCTION 803eb9a4
void AGUI2::CloseXUIFonts()
{
	m_oXUI.CloseFonts();
}

// $BarnyardWiiJP: FUNCTION 803eb9ac
void AGUI2::LoadXUIMaterialLibrary( const TCHAR* a_szName )
{
	TCHAR szLibName[ 128 ];
	TStringManager::String8Format( szLibName, 128, "xui/%s", a_szName );
	TStringManager::String8ToLowerCase( szLibName );

	TPString8 strLibName( szLibName );
	AMaterialLibraryManager::GetSingleton()->LoadLibrary( strLibName, TNULL, TTRUE );
	m_oXUI.AddMaterialLibrary( strLibName );
}

// $BarnyardWiiJP: FUNCTION 803eba64
void AGUI2::LoadXUIFont( const TCHAR* a_szName )
{
	// NOTE: the Wii fonts aren't supported, so they aren't loaded and XUI texts use the
	// PC retail fonts (see AGUI2XUIText::Create). The Wii build does:
	//   LoadXUIMaterialLibrary( "<language>/<a_szName>" );  // the font textures
	//   T2GUIFontManager::Open( a_szName, TTRUE );          // Data/XUI/Fonts/<language>/<a_szName>.trb
	//   m_oXUI.AddFont( TPString8( a_szName ) );            // closed by CloseXUIFonts
}

// $BarnyardWiiJP: FUNCTION 803ebb58
void AGUI2::DeleteXUIScenes()
{
	m_oXUI.DeleteScenes();
	m_pFocusedXUIScene = TNULL;
}

// $BarnyardWiiJP: FUNCTION 803ebb90
const TCHAR* AGUI2::GetLanguage()
{
	return "jp";
}

#  ifdef BARNYARD_COMMUNITY_PATCH

// Logs an element and its children
static void DevLogXUIElement( AGUI2XUIElement* a_pElement, TINT a_iDepth )
{
	TCHAR szId[ 128 ] = "";
	if ( a_pElement->GetId() )
		TStringManager::StringUnicodeToChar( szId, a_pElement->GetId(), 127 );

	TVector2 vecPos = a_pElement->GetTransform().GetTranslation();
	TINFO( "[devxui] %*s%s '%s' pos=(%.1f, %.1f) size=(%.1f, %.1f) scale=(%.2f, %.2f) pivot=(%.1f, %.1f) visible=%d\n", a_iDepth * 2, "", a_pElement->GetClass()->GetName(), szId, vecPos.x, vecPos.y, a_pElement->GetWidth(), a_pElement->GetHeight(), a_pElement->GetTransform().GetScaleX(), a_pElement->GetTransform().GetScaleY(), a_pElement->GetPivot().x, a_pElement->GetPivot().y, a_pElement->IsVisible() );

	for ( T2GUIElement* it = a_pElement->ChildRBegin(); it != TNULL; it = a_pElement->GetPrevChild( it ) )
	{
		if ( it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) )
			DevLogXUIElement( TSTATICCAST( AGUI2XUIElement, it ), a_iDepth + 1 );
	}
}

void AGUI2::DevLoadXUIScene( const TCHAR* a_szArgs )
{
	// The scene name is the rest of the argument up to a space
	TCHAR szName[ 128 ];
	TINT  iLength = 0;

	while ( a_szArgs[ iLength ] != '\0' && a_szArgs[ iLength ] != ' ' && iLength < TARRAYSIZE( szName ) - 1 )
	{
		szName[ iLength ] = a_szArgs[ iLength ];
		iLength++;
	}

	szName[ iLength ] = '\0';
	TINFO( "[devxui] loading '%s'\n", szName );

	// What the Wii loading screens load before their scene
	LoadXUIMaterialLibrary( "wii_icons128" );

	if ( !AGUI2XUIText::ms_pIconMaterial )
		AGUI2XUIText::ms_pIconMaterial = T2GUI::GetRenderer()->CreateMaterial( "..\\..\\..\\Texture\\wii_icons128.tga" );

	LoadXUIResource( szName );
	AGUI2XUIResource* pResource = m_oXUI.FindResource( szName );

	if ( !pResource )
	{
		TINFO( "[devxui] failed to load the resource\n" );
		return;
	}

	LoadXUIMaterialLibrary( szName );

	AGUI2XUIScene* pScene = CreateXUIScene( pResource, 0 );
	TINFO( "[devxui] scene 0: %p\n", pScene );

	if ( pScene )
	{
		pScene->SetFocus( TTRUE );
		DevLogXUIElement( pScene, 0 );
	}
}

#  endif // BARNYARD_COMMUNITY_PATCH

#endif // BARNYARD_LATEST
