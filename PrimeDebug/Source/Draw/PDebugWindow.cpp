#include "PrimeDebugPCH.h"
#include "PDebugWindow.h"
#include "PDebugText.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL                 PDebugWindow::ms_bStarted;
PArray<PDebugWindow*> PDebugWindow::ms_oWindows;
PCFunc                PDebugWindow::ms_oShowFunc;
PCFunc                PDebugWindow::ms_oHideFunc;
PCFunc                PDebugWindow::ms_oPrintFunc;
PCFunc                PDebugWindow::ms_oSetPositionFunc;
PCFunc                PDebugWindow::ms_oListFunc;

static const TCHAR s_szRule[] = "===============================";

// $deBlob2: FUNCTION 0084f48c
PDebugWindow::Text::Text( TINT a_iNumLines )
{
	m_iStart    = 0;
	m_iCount    = 0;
	m_iNumLines = a_iNumLines > 0 ? a_iNumLines : 1;
	m_pLines    = new Line[ m_iNumLines ];
}

PDebugWindow::Text::~Text()
{
	delete[] m_pLines;
}

// $deBlob2: FUNCTION 0084f911
TINT PDebugWindow::Text::GetIndex( TINT a_iLine )
{
	return ( m_iStart + a_iLine ) % m_iNumLines;
}

// $deBlob2: FUNCTION 0084f8de
PDebugWindow::Line* PDebugWindow::Text::GetLine( TINT a_iLine )
{
	return &m_pLines[ ( m_iStart + a_iLine ) % m_iNumLines ];
}

// When full, the oldest line goes; longer lines are cut to 127 characters
// $deBlob2: FUNCTION 0084f9c8
void PDebugWindow::Text::Add( const TCHAR* a_szText )
{
	TINT iSize = m_iNumLines;
	TINT iLine = m_iCount;

	if ( iLine == iSize )
	{
		iLine    = iSize - 1;
		m_iStart = ( m_iStart + 1 ) % iSize;
	}
	else
	{
		m_iCount = iLine + 1;
	}

	TINT iLength = TINT( strlen( a_szText ) );

	if ( iLength + 1 > LINE_LENGTH - 1 )
		iLength = LINE_LENGTH - 1;

	TCHAR* pLine = m_pLines[ ( m_iStart + iLine ) % iSize ].szText;
	memcpy( pLine, a_szText, iLength );
	pLine[ iLength ] = '\0';
}

// $deBlob2: FUNCTION 0084f4bd
PDebugWindow::PDebugWindow( const TCHAR* a_szName, TINT a_iNumLines )
    : m_oText( a_iNumLines )
{
	m_uiTextColour     = 0xFFFFFFFF;
	m_bInWorld         = TFALSE;
	m_eHorizontalAlign = ALIGN_START;
	m_eVerticalAlign   = ALIGN_START;
	m_bVisible         = TTRUE;
	m_bTitle           = TTRUE;
	m_uiBackColour     = 0x4C191919;
	m_uiBorderColour   = 0xFF191919;
	m_fMaxDistance     = 10000.0f;
	m_vPosition.x      = 0.5f;
	m_vPosition.y      = 0.5f;
	m_vPosition.z      = 0.0f;
	m_vPosition.w      = 1.0f;
	m_fWidth           = 0.0f;
	m_fHeight          = 0.0f;
	m_fUnknown88       = 1.0f;
	m_fUnknown8C       = 1.0f;
	PString::Copy( m_szName, sizeof( m_szName ), a_szName );
}

// $deBlob2: FUNCTION 0084f5d5
PDebugWindow::~PDebugWindow()
{
}

// $deBlob2: FUNCTION 0084f8fa
void PDebugWindow::GetPosition( TVector4* a_pPosition )
{
	*a_pPosition = m_vPosition;
}

// $deBlob2: FUNCTION 008500ad
void PDebugWindow::SetPosition( const TVector4* a_pPosition )
{
	m_vPosition = *a_pPosition;
}

// $deBlob2: FUNCTION 008500c2
void PDebugWindow::SetVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;
}

// $deBlob2: FUNCTION 0084f7de
PDebugWindow* PDebugWindow::Create( const TCHAR* a_szName, TINT a_iNumLines )
{
	PDebugWindow* pWindow = new PDebugWindow( a_szName, a_iNumLines );
	ms_oWindows.PushBack( pWindow );
	return pWindow;
}

// $deBlob2: FUNCTION 0084f829
void PDebugWindow::DestroyAll()
{
	for ( TINT i = 0; i < ms_oWindows.Size(); i++ )
		delete ms_oWindows[ i ];

	ms_oWindows.SetSize( 0 );
}

// $deBlob2: FUNCTION 0084f86d
void PDebugWindow::Destroy( PDebugWindow* a_pWindow )
{
	TINT iCount = ms_oWindows.Size();

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( ms_oWindows[ i ] == a_pWindow )
		{
			ms_oWindows[ i ] = ms_oWindows[ iCount - 1 ];
			ms_oWindows.SetSize( ms_oWindows.Size() - 1 );

			if ( ms_oWindows.Size() == 0 )
				ms_oWindows.Free();

			delete a_pWindow;
			return;
		}
	}
}

// $deBlob2: FUNCTION 0084f927
PDebugWindow* PDebugWindow::Find( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < ms_oWindows.Size(); i++ )
	{
		if ( strcmp( ms_oWindows[ i ]->m_szName, a_szName ) == 0 )
			return ms_oWindows[ i ];
	}

	return TNULL;
}

// $deBlob2: FUNCTION 0084fa2f
void PDebugWindow::Printf( PDebugWindow* a_pWindow, const TCHAR* a_szFormat, ... )
{
	TCHAR szText[ LINE_LENGTH ];

	va_list args;
	va_start( args, a_szFormat );
	PString::FormatV( szText, sizeof( szText ), a_szFormat, args );
	va_end( args );

	a_pWindow->m_oText.Add( szText );
}

// $deBlob2: FUNCTION 0084f6f7
TBOOL PDebugWindow::SetVisibleCommand( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bVisible )
{
	if ( a_iArgc != 2 )
		return TFALSE;

	PDebugWindow* pWindow = Find( a_ppArgv[ 1 ] );

	if ( pWindow )
		pWindow->m_bVisible = a_bVisible;

	return TTRUE;
}

// $deBlob2: FUNCTION 0084f61c
TBOOL PDebugWindow::HideCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	return SetVisibleCommand( a_iArgc, a_ppArgv, TFALSE );
}

// NOTE: lists nothing in de Blob 2; the names are printed here
// $deBlob2: FUNCTION 0084f631
TBOOL PDebugWindow::ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 1 )
		return TFALSE;

	for ( TINT i = 0; i < ms_oWindows.Size(); i++ )
		TINFO( "%s%s\n", ms_oWindows[ i ]->m_szName, ms_oWindows[ i ]->m_bVisible ? "" : " (hidden)" );

	return TTRUE;
}

// $deBlob2: FUNCTION 0084f63f
TBOOL PDebugWindow::PrintCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 3 )
		return TFALSE;

	PDebugWindow* pWindow = Find( a_ppArgv[ 1 ] );

	if ( pWindow )
	{
#  ifdef BARNYARD_COMMUNITY_PATCH
		Printf( pWindow, "%s", a_ppArgv[ 2 ] );
#  else
		// NOTE: the text is used as the format
		Printf( pWindow, a_ppArgv[ 2 ] );
#  endif
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0084f66e
TBOOL PDebugWindow::SetPositionCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 4 )
		return TFALSE;

	PDebugWindow* pWindow = Find( a_ppArgv[ 1 ] );

	if ( pWindow )
	{
		TVector4 vPosition;
		vPosition.x          = PString::ToFloat( a_ppArgv[ 2 ], TNULL );
		vPosition.y          = PString::ToFloat( a_ppArgv[ 3 ], TNULL );
		vPosition.z          = 0.0f;
		vPosition.w          = 1.0f;
		pWindow->m_vPosition = vPosition;
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0084f6e2
TBOOL PDebugWindow::ShowCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	return SetVisibleCommand( a_iArgc, a_ppArgv, TTRUE );
}

// $deBlob2: FUNCTION 0084fb30
void PDebugWindow::DrawAll( const TVector4* a_pEye )
{
	for ( TINT i = 0; i < ms_oWindows.Size(); i++ )
	{
		PDebugWindow* pWindow = ms_oWindows[ i ];

		if ( pWindow->m_bVisible )
			Draw( pWindow, a_pEye );
	}
}

// NOTE: Prime projects the windows in the world with the view projection matrix it gets; here the renderer does
// $deBlob2: FUNCTION 0084fb6b
void PDebugWindow::Draw( PDebugWindow* a_pWindow, const TVector4* a_pEye )
{
	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();

	if ( !pRenderer )
		return;

	TFLOAT fX, fY, fScale;

	if ( !a_pWindow->m_bInWorld )
	{
		fX     = a_pWindow->m_vPosition.x;
		fY     = a_pWindow->m_vPosition.y;
		fScale = 0.65f;
	}
	else
	{
		if ( !a_pEye )
			return;

		TFLOAT fDX       = a_pEye->x - a_pWindow->m_vPosition.x;
		TFLOAT fDY       = a_pEye->y - a_pWindow->m_vPosition.y;
		TFLOAT fDZ       = a_pEye->z - a_pWindow->m_vPosition.z;
		TFLOAT fDistance = TFLOAT( sqrt( double( fDX * fDX + fDY * fDY + fDZ * fDZ ) ) );

		if ( fDistance > a_pWindow->m_fMaxDistance )
			return;

		if ( !pRenderer->ProjectToScreen( a_pWindow->m_vPosition, fX, fY ) )
			return;

		fScale = ( ( a_pWindow->m_fMaxDistance - fDistance ) / a_pWindow->m_fMaxDistance ) * 0.5f + 0.5f;
	}

	TFLOAT fWidth  = a_pWindow->m_fWidth;
	TFLOAT fHeight = a_pWindow->m_fHeight;

	if ( fHeight == 0.0f && fWidth == 0.0f )
	{
		for ( TINT i = 0; i < a_pWindow->m_oText.GetCount(); i++ )
		{
			TFLOAT fLineWidth  = 0.0f;
			TFLOAT fLineHeight = 0.0f;
			PDebugText::Measure( a_pWindow->m_oText.GetLine( i )->szText, &fLineWidth, &fLineHeight, fScale );

			if ( fWidth <= fLineWidth )
				fWidth = fLineWidth;

			fHeight = fLineHeight + fHeight;
		}

		if ( a_pWindow->m_bTitle )
		{
			TFLOAT fRuleWidth  = 0.0f;
			TFLOAT fRuleHeight = 0.0f;
			PDebugText::Measure( s_szRule, &fRuleWidth, &fRuleHeight, fScale );

			if ( fWidth <= fRuleWidth )
				fWidth = fRuleWidth;

			fHeight = fRuleHeight * 2.0f + fHeight;
		}
	}

	if ( fWidth <= 0.0f || fHeight <= 0.0f )
		return;

	if ( a_pWindow->m_eVerticalAlign != ALIGN_START || a_pWindow->m_eHorizontalAlign != ALIGN_START )
	{
		if ( a_pWindow->m_eVerticalAlign == ALIGN_CENTRE )
			fY -= fHeight * 0.5f;
		else if ( a_pWindow->m_eVerticalAlign == ALIGN_END )
			fY -= fHeight;

		if ( a_pWindow->m_eHorizontalAlign == ALIGN_CENTRE )
			fX -= fWidth * 0.5f;
		else if ( a_pWindow->m_eHorizontalAlign == ALIGN_END )
			fX -= fWidth;
	}

	if ( a_pWindow->m_bTitle )
	{
		TCHAR szTitle[ 0x80 ];
		PString::Format( szTitle, sizeof( szTitle ), "=== %s ===", a_pWindow->m_szName );
		PDebugText::Draw( szTitle, fX, fY, PDebugText::COLOUR_RED, fScale, 0, TNULL, TNULL );
		fY = PDebugText::GetLineHeight() * fScale + fY;
	}

#  ifdef BARNYARD_COMMUNITY_PATCH
	pRenderer->DrawRect( fX - 0.005f, fY - 0.015f, fWidth, fHeight, a_pWindow->m_uiBackColour );
#  else
	// NOTE: Prime passes the size as the position and the centre as the size (with a UV scale of 0)
	pRenderer->DrawRect( fWidth, fHeight, ( fWidth * 0.5f + fX ) - 0.005f, ( fHeight * 0.5f + fY ) - 0.015f, a_pWindow->m_uiBackColour );
#  endif

	for ( TINT i = 0; i < a_pWindow->m_oText.GetCount(); i++ )
	{
		PDebugText::Draw( a_pWindow->m_oText.GetLine( i )->szText, fX, fY, a_pWindow->m_uiTextColour, fScale, 0, TNULL, TNULL );
		fY = PDebugText::GetLineHeight() * fScale + fY;
	}

	if ( a_pWindow->m_bTitle )
		PDebugText::Draw( s_szRule, fX, fY, PDebugText::COLOUR_RED, fScale, 0, TNULL, TNULL );
}

// $deBlob2: FUNCTION 008500cf
void PDebugWindow::Shutdown()
{
	if ( !ms_bStarted )
		return;

	DestroyAll();

	ms_oShowFunc.Remove();
	ms_oHideFunc.Remove();
	ms_oSetPositionFunc.Remove();
	ms_oPrintFunc.Remove();
	ms_oListFunc.Remove();

	ms_oWindows.Free();
	ms_bStarted = TFALSE;
}

// NOTE: Prime's window manager object (its heap, the windows, the "menubackground" texture) is folded into the
// statics of the class
// $deBlob2: FUNCTION 0085014e
TBOOL PDebugWindow::Startup()
{
	ms_oShowFunc.Create( "debug_window.show", "Shows debug window.", "debug_window.show window_name", ShowCommand, 1, TNULL );
	ms_oHideFunc.Create( "debug_window.hide", "Hides debug window.", "debug_window.hide window_name", HideCommand, 1, TNULL );
	ms_oSetPositionFunc.Create( "debug_window.set_position", "Sets position of debug window.", "debug_window.set_position window_name x y", SetPositionCommand, 3, TNULL );
	ms_oPrintFunc.Create( "debug_window.print", "Appends text to debug window.", "debug_window.print window_name text", PrintCommand, 2, TNULL );
	ms_oListFunc.Create( "debug_window.list", "Lists all debug windows.", "debug_window.list", ListCommand, 0, TNULL );

	ms_bStarted = TTRUE;
	return TTRUE;
}

#endif // PRIMEDEBUG_DEV
