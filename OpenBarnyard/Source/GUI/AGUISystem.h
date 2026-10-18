#pragma once
#include "Render/ARenderer.h"
#include "AGUIMatlibPicture.h"

#include <Plugins/PGUIRenderer/PGUITRRenderObject.h>
#include <Toshi/TPString8.h>

#ifdef BARNYARD_COMMUNITY_PATCH
#  include <Input/TInputInterface.h>

class PGUITRDisplayContext;
class PGUITRViewportHandler;
class PGUIOwnerDraw;
class AGUISoundFactory;
class AGUIAudioContext;
class AGUIInputContext;
class AGUITGUIDemo;

namespace Toshi
{
class TGUIScreen;
class TGUIWidget;
class TGUIWindow;
struct TGUIWidgetPaintEvent;
} // namespace Toshi
#endif

class AGUISystem
    : public Toshi::TTask
    , public Toshi::TSingleton<AGUISystem>
{
public:
	TDECLARE_CLASS( AGUISystem, Toshi::TTask );
	static constexpr TUINT NUM_RENDER_OBJECTS = 2;

public:
	AGUISystem();
	~AGUISystem();

	virtual TBOOL OnCreate() override;
	virtual void  OnDestroy() override;

	void SetPicture( const Toshi::TPString8& a_rName );
	void ResetPicture();

#ifdef BARNYARD_LATEST
	// Wii: a second picture that stays loaded, drawn instead of the normal one behind the XUI load screens
	void SetBackgroundPicture( const Toshi::TPString8& a_rName );
	void ShowBackgroundPicture( TBOOL a_bShow );
#endif

	// DOES_NOTHING (AGameState::HUDParams::Apply)
	void ApplyHUDParams() {}

	PGUITRRenderObject* GetRenderObject( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_RENDER_OBJECTS );
		return m_pRenderObjects[ a_uiIndex ];
	}

	AGUIMatlibPicture& GetMatlibPicture()
	{
		return m_MatLibPicture;
	}

public:
	static void AllowBackgroundClear( TBOOL a_bAllow );

#ifdef BARNYARD_COMMUNITY_PATCH
	//-----------------------------------------------------------------------------
	// NOTE: Barnyard route added on top: hosts the legacy TOSHI GUI (TGUI/PGUI,
	// as JPOG's AGUISystem did), created only when something uses it. The screen
	// (800x600, AGUI2's root size) is painted in OnGUIRender through the render
	// objects, with AGUI2's renderer, and gets the input of the game states while
	// it shows a widget (AGameStateController::ProcessInput).
	//-----------------------------------------------------------------------------
	static constexpr TINT TGUI_SCREEN_WIDTH  = 800;
	static constexpr TINT TGUI_SCREEN_HEIGHT = 600;
	static constexpr TINT MAX_OWNER_DRAWS    = 16;
	static constexpr TINT TGUI_MEMORY_SIZE   = 8 * 1024 * 1024;

	// The TGUI objects are allocated in a memory block of their own, outside the game's heap (the retail heap has no
	// room for them, the main world leaves about 2 MB free). The functions of the TGUI hosting set it while they run.
	class TGUIMemoryScope
	{
	public:
		TGUIMemoryScope();
		~TGUIMemoryScope();

	private:
		Toshi::TMemory::MemBlock* m_pOldMemBlock;
	};

	using OwnerDrawListener = Toshi::TListener<Toshi::TGUIWidget, Toshi::TGUIWidgetPaintEvent, PGUIOwnerDraw>;

	// Creates the GUI manager, the display context, the factories and the screen
	TBOOL CreateTGUI();
	void  DestroyTGUI();

	// Loads a definition file (e.g. "Data/GUI/main.gui"), TFALSE if it's missing or can't be read
	TBOOL LoadTGUIDefinitionFile( const TCHAR* a_szFileName );

	// Adds a copy of a top-level widget (a [dialog]) of the definition files to the screen, TNULL if there's none
	Toshi::TGUIWidget* ShowTGUIDialog( const Toshi::TPString8& a_rcName );

	// Removes a widget ShowTGUIDialog returned from the screen and deletes it
	void HideTGUIDialog( Toshi::TGUIWidget* a_pDialog );

	// Gives an input event to the screen if it shows a widget, TTRUE if the screen used it
	TBOOL NotifyTGUIInputEvent( const Toshi::TInputInterface::InputEvent* a_pEvent );

	// Pointer mode (the editor states, made for the mouse): the screen only keeps the mouse events over its widgets and
	// the keyboard while a text box has the focus, the rest goes on to the game state
	void  SetTGUIPointerMode( TBOOL a_bPointerMode ) { m_bTGUIPointerMode = a_bPointerMode; }
	TBOOL IsTGUIPointerMode() const { return m_bTGUIPointerMode; }

	// Barnyard's dialogs have empty focus maps (their game code is gone, the editor dialogs were used with the mouse):
	// makes a focus map from the positions of the widgets that take the focus, for the keyboard and the pad
	void BuildTGUIFocusMap( Toshi::TGUIWindow* a_pWindow );

	// The owner draw panels paint themselves when their widget is painted (nothing in the GUI libraries calls it)
	void ConnectOwnerDraws( Toshi::TGUIWidget* a_pWidget );
	void DisconnectOwnerDraws( Toshi::TGUIWidget* a_pWidget );

	// AMaterialLibraryManager calls it before it unloads the textures of a material library and once it loaded one: the
	// textures of the GUI are found again before the screen is painted
	void OnTGUIMaterialLibrariesChanged( TBOOL a_bUnloading );

#  ifndef TOSHI_FINAL
	// Development: shows or hides the dialogs of Data/GUI/main.gui ("-tguidemo", the debug menu)
	void  Dev_ShowTGUIDemo( TBOOL a_bShow );
	TBOOL Dev_IsTGUIDemoShown() const { return m_pTGUIDemo != TNULL; }
#  endif

	TBOOL                 IsTGUICreated() const { return m_pTGUIScreen != TNULL; }
	Toshi::TGUIScreen*    GetTGUIScreen() const { return m_pTGUIScreen; }
	PGUITRDisplayContext* GetTGUIDisplayContext() const { return m_pTGUIDisplayContext; }

private:
	// Paints the screen (in OnGUIRender)
	void RenderTGUI();

	// Maps the screen to AGUI2's root element (the scale of the display context follows the viewport)
	void UpdateTGUIScreenTransform();

	static TBOOL __stdcall OnOwnerDrawPaint( PGUIOwnerDraw* a_pOwnerDraw, Toshi::TGUIWidget* a_pWidget, Toshi::TGUIWidgetPaintEvent* a_pEvent );
	static TBOOL __stdcall FindOwnerDrawCallback( Toshi::TGUIWidget* a_pWidget, void* a_pUserData );
	static TBOOL __stdcall FindFocusableCallback( Toshi::TGUIWidget* a_pWidget, void* a_pUserData );
#endif

private:
	static TBOOL __stdcall OnGUIRender( AGUISystem* a_pGUISystem, ARenderer* a_pRenderer, TINT* a_pUnknown );

private:
	TUINT                                         m_Unk1;
	Toshi::TGenericListener                       m_SomeListener;
	Toshi::TListener<ARenderer, TINT, AGUISystem> m_RenderGUIListener;
	Toshi::TPString8*                             m_pStrings;
	AGUIMatlibPicture                             m_MatLibPicture;
	TUINT                                         m_Unk2;
	TUINT                                         m_Unk3;
	PGUITRRenderObject*                           m_pRenderObjects[ NUM_RENDER_OBJECTS ];

#ifdef BARNYARD_LATEST
	AGUIMatlibPicture m_oBackgroundPicture;     // 0x78 (Wii)
	TBOOL             m_bShowBackgroundPicture; // 0x94 (Wii)
#endif

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Barnyard route (the TGUI hosting, see CreateTGUI)
	PGUITRDisplayContext*  m_pTGUIDisplayContext;
	Toshi::TGUIScreen*     m_pTGUIScreen;
	PGUITRViewportHandler* m_apTGUIViewportHandlers[ NUM_RENDER_OBJECTS ];
	AGUISoundFactory*      m_pTGUISoundFactory;
	AGUIAudioContext*      m_pTGUIAudioContext;
	AGUIInputContext*      m_pTGUIInputContext;
	TBOOL                  m_bTGUIMaterialsDirty;
	TBOOL                  m_bTGUIMatLibLoaded;
	TBOOL                  m_bTGUIPointerMode;
	TINT                   m_iNumOwnerDraws;
	PGUIOwnerDraw*         m_apOwnerDraws[ MAX_OWNER_DRAWS ];
	OwnerDrawListener*     m_apOwnerDrawListeners[ MAX_OWNER_DRAWS ];
#  ifndef TOSHI_FINAL
	AGUITGUIDemo* m_pTGUIDemo; // "-tguidemo"
#  endif
#endif
};

TSINGLETON_DECLARE_ALIAS( AGUISystem, GUISystem );
