#pragma once
#include "PDebugRenderer.h"

#ifdef PRIMEDEBUG_DEV
#  include "Console/PCFunc.h"
#  include "Util/PArray.h"

#  include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Prime's debug windows (geDebugWindow): boxes of the last lines
// printed to them, on the screen (the position in fractions of it) or at a
// point in the world (smaller with the distance, hidden past their maximum
// distance), optionally with a "=== name ===" title and a rule below.
// Commands: debug_window.show/hide/print/set_position/list.
//-----------------------------------------------------------------------------
class PDebugWindow
{
public:
	enum ALIGN
	{
		ALIGN_CENTRE = 0,
		ALIGN_START  = 1, // left or top: the position is the corner
		ALIGN_END    = 2,
	};

	static constexpr TINT LINE_LENGTH = 0x80;

	struct Line
	{
		TCHAR szText[ LINE_LENGTH ];
	};

	// The last lines printed, oldest first
	class Text
	{
	public:
		Text( TINT a_iNumLines );
		~Text();

		TINT  GetIndex( TINT a_iLine );
		Line* GetLine( TINT a_iLine );
		void  Add( const TCHAR* a_szText );
		TINT  GetCount() const { return m_iCount; }

	private:
		TINT  m_iStart; // 0x00
		TINT  m_iCount; // 0x04
		Line* m_pLines; // its size is the number kept
		TINT  m_iNumLines;
	};

public:
	PDebugWindow( const TCHAR* a_szName, TINT a_iNumLines );
	~PDebugWindow();

	void GetPosition( Toshi::TVector4* a_pPosition );
	void SetPosition( const Toshi::TVector4* a_pPosition );
	void SetVisible( TBOOL a_bVisible );

	const TCHAR* GetName() const { return m_szName; }

	// Changed by the code that creates the window
	TINT            m_eHorizontalAlign; // ALIGN
	TINT            m_eVerticalAlign;   // ALIGN
	TBOOL           m_bVisible;
	TUINT32         m_uiTextColour;
	TUINT32         m_uiBackColour;
	TBOOL           m_bTitle; // a "=== name ===" line above and a rule below
	TUINT32         m_uiBorderColour;
	TBOOL           m_bInWorld;
	TFLOAT          m_fMaxDistance; // in the world: hidden further away
	Toshi::TVector4 m_vPosition;    // on screen (0..1) or in the world
	TFLOAT          m_fWidth;       // 0 (with the height): fits the text
	TFLOAT          m_fHeight;
	TFLOAT          m_fUnknown88;
	TFLOAT          m_fUnknown8C;
	Text            m_oText;

public:
	static TBOOL         Startup();
	static void          Shutdown();
	static PDebugWindow* Create( const TCHAR* a_szName, TINT a_iNumLines );
	static void          Destroy( PDebugWindow* a_pWindow );
	static void          DestroyAll();
	static PDebugWindow* Find( const TCHAR* a_szName );
	static void          Printf( PDebugWindow* a_pWindow, const TCHAR* a_szFormat, ... );
	static void          Draw( PDebugWindow* a_pWindow, const Toshi::TVector4* a_pEye );
	static void          DrawAll( const Toshi::TVector4* a_pEye );

private:
	static TBOOL SetVisibleCommand( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bVisible );
	static TBOOL HideCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL PrintCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL SetPositionCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ShowCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );

private:
	TCHAR m_szName[ 0x40 ];

	static TBOOL                 ms_bStarted;
	static PArray<PDebugWindow*> ms_oWindows;
	static PCFunc                ms_oShowFunc;
	static PCFunc                ms_oHideFunc;
	static PCFunc                ms_oPrintFunc;
	static PCFunc                ms_oSetPositionFunc;
	static PCFunc                ms_oListFunc;
};

#endif // PRIMEDEBUG_DEV
