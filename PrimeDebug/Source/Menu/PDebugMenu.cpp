#include "PrimeDebugPCH.h"
#include "PDebugMenu.h"
#include "Console/PConsole.h"
#include "Console/PKeyboard.h"
#include "Draw/PDebugText.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL           PDebugMenu::ms_bStarted = TFALSE;
TBOOL           PDebugMenu::ms_bEnabled = TTRUE;
TBOOL           PDebugMenu::ms_bVisible = TFALSE;
PDebugMenuTree* PDebugMenu::ms_pMenu    = TNULL;
TINT            PDebugMenu::ms_iPad     = 0;
PCVar           PDebugMenu::ms_oTextSize;
PCVar           PDebugMenu::ms_oTransparency;
PCFunc          PDebugMenu::ms_oAddSubmenuFunc;
PCFunc          PDebugMenu::ms_oAddLabelFunc;
PCFunc          PDebugMenu::ms_oAddVarFunc;
PCFunc          PDebugMenu::ms_oAddCommandLineFunc;
PCFunc          PDebugMenu::ms_oRemoveFunc;
PCFunc          PDebugMenu::ms_oResetFunc;
TCHAR           PDebugMenu::ms_szTyped[ MAX_TYPED + 1 ];
TINT            PDebugMenu::ms_iNumTyped = 0;
TFLOAT          PDebugMenu::ms_fHeldTime = 0.0f;

//-----------------------------------------------------------------------------
// PDebugMenuTree
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00808f8e
PDebugMenuTree::PDebugMenuTree( TFLOAT a_fScale, TFLOAT a_fAlpha )
{
	m_fScale           = a_fScale;
	m_pRoot            = TNULL;
	m_iDepth           = 1;
	m_fAlpha           = a_fAlpha;
	m_bShowPath        = TTRUE;
	m_pRoot            = new PDebugMenuNode( PDebugMenu::ROOT );
	m_aiSelection[ 0 ] = -1;
}

// $deBlob2: FUNCTION 008090d8
// $deBlob2: FUNCTION 00807d25
PDebugMenuTree::~PDebugMenuTree()
{
	if ( m_pRoot )
		delete m_pRoot;
}

// $deBlob2: FUNCTION 0080920f
TINT PDebugMenuTree::AddCommandLine( const TCHAR* a_szParent, const TCHAR* a_szName, const TCHAR* a_szCommand )
{
	PDebugMenuNode* pParent = TSTATICCAST( PDebugMenuNode, FindPath( a_szParent, TNULL ) );

	if ( pParent && pParent->GetType() == PDebugNode::TYPE_MENU && !pParent->HasChild( a_szName ) )
	{
		PDebugNode* pNode = new PDebugCommandLineNode( a_szName, a_szCommand );
		pParent->m_oChildren.PushBack( pNode );

		if ( m_aiSelection[ 0 ] == -1 )
			m_aiSelection[ 0 ] = 0;

		return pNode->GetId();
	}

	return -1;
}

// $deBlob2: FUNCTION 0080928b
TINT PDebugMenuTree::AddLabel( const TCHAR* a_szParent, const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData )
{
	PDebugMenuNode* pParent = TSTATICCAST( PDebugMenuNode, FindPath( a_szParent, TNULL ) );

	if ( pParent && pParent->GetType() == PDebugNode::TYPE_MENU && !pParent->HasChild( a_szName ) )
	{
		PDebugNode* pNode = new PDebugLabelNode( a_szName, a_fnCallback, a_pUserData );
		pParent->m_oChildren.PushBack( pNode );

		if ( m_aiSelection[ 0 ] == -1 )
			m_aiSelection[ 0 ] = 0;

		return pNode->GetId();
	}

	return -1;
}

// $deBlob2: FUNCTION 00809307
TINT PDebugMenuTree::AddSubmenu( const TCHAR* a_szParent, const TCHAR* a_szName )
{
	PDebugMenuNode* pParent = TSTATICCAST( PDebugMenuNode, FindPath( a_szParent, TNULL ) );

	if ( pParent && pParent->GetType() == PDebugNode::TYPE_MENU && !pParent->HasChild( a_szName ) )
	{
		PDebugNode* pNode = new PDebugMenuNode( a_szName );
		pParent->m_oChildren.PushBack( pNode );

		if ( m_aiSelection[ 0 ] == -1 )
			m_aiSelection[ 0 ] = 0;

		return pNode->GetId();
	}

	return -1;
}

// The child menu of that name, added if there's none
// $deBlob2: FUNCTION 00809380
PDebugMenuNode* PDebugMenuTree::GetSubmenu( PDebugMenuNode* a_pParent, const TCHAR* a_szName )
{
	PDebugNode* pNode = a_pParent;

	if ( !a_pParent->FindChild( a_szName, &pNode ) )
	{
		pNode = new PDebugMenuNode( a_szName );
		a_pParent->m_oChildren.PushBack( pNode );

		if ( m_aiSelection[ 0 ] == -1 )
			m_aiSelection[ 0 ] = 0;
	}

	return TSTATICCAST( PDebugMenuNode, pNode );
}

// A variable or command found for "name*": put in submenus for the parts of its name after the last '.' of the
// match, at the front
// $deBlob2: FUNCTION 008094f0
void PDebugMenuTree::AddMatch( const TCHAR* a_szName, void* a_pUserData )
{
	AddVars*        pAdd    = TSTATICCAST( AddVars, a_pUserData );
	PDebugMenuNode* pParent = pAdd->pParent;

	if ( pParent->HasChild( a_szName ) )
		return;

	PCVar*  pVar  = PConsole::FindVariable( a_szName );
	PCFunc* pFunc = PConsole::FindCommand( a_szName );

	if ( !pVar && !pFunc )
		return;

	TINT         iDot  = -1;
	const TCHAR* pLast = strrchr( pAdd->szMatch, '.' );

	if ( pLast )
		iDot = TINT( pLast - pAdd->szMatch );

	for ( const TCHAR* pPart = a_szName + iDot; pPart; )
	{
		const TCHAR* pNext = strchr( pPart + 1, '.' );

		if ( pNext )
		{
			TINT  iLength = TINT( pNext - pPart ) - 1;
			TCHAR szPart[ 0x80 ];
			memcpy( szPart, pPart + 1, iLength );
			szPart[ iLength ] = '\0';
			pParent           = pAdd->pMenu->GetSubmenu( pParent, szPart );
		}

		pPart = pNext;
	}

	PDebugNode* pNode;

	if ( pVar )
		pNode = new PDebugVarNode( a_szName, pVar );
	else
		pNode = new PDebugCommandLineNode( a_szName, a_szName );

	pParent->m_oChildren.InsertAt( 0 );
	pParent->m_oChildren[ 0 ] = pNode;
}

// A name ending with '*' adds every variable and command starting with the rest
// $deBlob2: FUNCTION 008093ea
TINT PDebugMenuTree::AddVar( const TCHAR* a_szParent, const TCHAR* a_szName )
{
	PDebugMenuNode* pParent = TSTATICCAST( PDebugMenuNode, FindPath( a_szParent, TNULL ) );

	if ( !pParent || pParent->GetType() != PDebugNode::TYPE_MENU )
		return -1;

	TINT iLength = TINT( strlen( a_szName ) );

	if ( iLength == 0 )
		return -1;

	if ( a_szName[ iLength - 1 ] == '*' )
	{
		TCHAR szMatch[ 0x100 ];
		PString::Copy( szMatch, sizeof( szMatch ), a_szName );
		szMatch[ iLength - 1 ] = '\0';

		AddVars oAdd;
		oAdd.pMenu   = this;
		oAdd.pParent = pParent;
		oAdd.szMatch = szMatch;
		PConsole::Enumerate( szMatch, -1, TTRUE, AddMatch, &oAdd );
		return -1;
	}

	if ( pParent->HasChild( a_szName ) )
		return -1;

	PCVar* pVar = PConsole::FindVariable( a_szName );

	if ( !pVar )
		return -1;

	PDebugNode* pNode = new PDebugVarNode( a_szName, pVar );
	pParent->m_oChildren.PushBack( pNode );

	if ( m_aiSelection[ 0 ] == -1 )
		m_aiSelection[ 0 ] = 0;

	return pNode->GetId();
}

// $deBlob2: FUNCTION 00809ad3
PDebugNode* PDebugMenuTree::FindPath( const TCHAR* a_szPath, PDebugMenuNode** a_ppParent )
{
	if ( a_ppParent )
		*a_ppParent = TNULL;

	return m_pRoot->FindPath( a_szPath, TINT( strlen( a_szPath ) ), a_ppParent );
}

// $deBlob2: FUNCTION 00809b05
PDebugNode* PDebugMenuTree::GetSelected()
{
	TINT iSelection = m_aiSelection[ m_iDepth - 1 ];

	if ( iSelection == -1 )
		return TNULL;

	return GetLevel( m_iDepth - 1 )->m_oChildren[ iSelection ];
}

// $deBlob2: FUNCTION 00809b2c
TINT PDebugMenuTree::GetSelectedId()
{
	PDebugNode* pNode = GetSelected();

	if ( pNode )
		return pNode->GetId();

	return -1;
}

// $deBlob2: FUNCTION 00809b3d
TBOOL PDebugMenuTree::GetSelectedLabelData( void** a_ppUserData )
{
	PDebugMenuNode* pMenu = m_pRoot;

	for ( TINT i = 1; i < m_iDepth; i++ )
		pMenu = TSTATICCAST( PDebugMenuNode, pMenu->m_oChildren[ m_aiSelection[ i - 1 ] ] );

	if ( m_aiSelection[ m_iDepth - 1 ] == -1 )
		return TFALSE;

	PDebugNode* pNode = pMenu->m_oChildren[ m_aiSelection[ m_iDepth - 1 ] ];

	if ( pNode->GetType() != PDebugNode::TYPE_LABEL )
		return TFALSE;

	*a_ppUserData = TSTATICCAST( PDebugLabelNode, pNode )->GetUserData();
	return TTRUE;
}

// The open menu at a level (0: the root)
// $deBlob2: FUNCTION 00809ba1
PDebugMenuNode* PDebugMenuTree::GetLevel( TINT a_iLevel )
{
	if ( a_iLevel >= m_iDepth )
		return TNULL;

	PDebugMenuNode* pMenu = m_pRoot;

	for ( TINT i = 1; i <= a_iLevel; i++ )
		pMenu = TSTATICCAST( PDebugMenuNode, pMenu->m_oChildren[ m_aiSelection[ i - 1 ] ] );

	return pMenu;
}

// $deBlob2: FUNCTION 00809bd7
TINT PDebugMenuTree::GetCount()
{
	return GetLevel( m_iDepth - 1 )->m_oChildren.Size();
}

// $deBlob2: FUNCTION 00809e18
TBOOL PDebugMenuTree::IsSelectedMenu()
{
	PDebugNode* pNode = GetSelected();
	TBOOL       bMenu = TFALSE;

	if ( pNode )
		bMenu = pNode->GetType() == PDebugNode::TYPE_MENU;

	return bMenu;
}

// $deBlob2: FUNCTION 00809e2c
TBOOL PDebugMenuTree::Next( TBOOL a_bWrap )
{
	TINT iDepth     = m_iDepth;
	TINT iSelection = m_aiSelection[ iDepth - 1 ];

	if ( iSelection == -1 )
		return TFALSE;

	iSelection++;

	if ( iSelection < GetLevel( iDepth - 1 )->m_oChildren.Size() )
	{
		m_aiSelection[ iDepth - 1 ] = iSelection;
	}
	else
	{
		if ( !a_bWrap )
			return TFALSE;

		m_aiSelection[ iDepth - 1 ] = 0;
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 00809e73
TBOOL PDebugMenuTree::Enter()
{
	if ( IsSelectedMenu() && TSTATICCAST( PDebugMenuNode, GetSelected() )->m_oChildren.Size() > 0 )
	{
		m_aiSelection[ m_iDepth ] = 0;
		m_iDepth++;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 00809ea6
TBOOL PDebugMenuTree::Decrease( TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
	TBOOL bAny = GetLevel( m_iDepth - 1 )->m_oChildren.Size() != 0;

	if ( bAny )
		GetSelected()->Activate( TFALSE, a_fHeldTime, a_fDeltaTime );

	return bAny;
}

// $deBlob2: FUNCTION 00809ef2
TBOOL PDebugMenuTree::Back()
{
	if ( m_iDepth > 1 )
	{
		m_iDepth--;
		return TTRUE;
	}

	return TFALSE;
}

// A submenu is entered only with a_bEnter
// $deBlob2: FUNCTION 00809f0a
TBOOL PDebugMenuTree::Increase( TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime, TBOOL a_bEnter )
{
	if ( GetLevel( m_iDepth - 1 )->m_oChildren.Size() != 0 )
	{
		if ( !IsSelectedMenu() )
		{
			GetSelected()->Activate( TTRUE, a_fHeldTime, a_fDeltaTime );
			return TTRUE;
		}

		if ( a_bEnter )
			return Enter();
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 00809f70
TBOOL PDebugMenuTree::Prev( TBOOL a_bBackAtTop, TBOOL a_bWrap )
{
	TINT iDepth     = m_iDepth;
	TINT iSelection = m_aiSelection[ iDepth - 1 ];

	if ( iSelection == -1 )
		return TFALSE;

	if ( iSelection == 0 )
	{
		if ( a_bBackAtTop )
			return Back();

		if ( !a_bWrap )
			return TFALSE;

		iSelection = GetLevel( iDepth - 1 )->m_oChildren.Size();
	}

	m_aiSelection[ iDepth - 1 ] = iSelection - 1;
	return TTRUE;
}

// Back to the root afterwards
// $deBlob2: FUNCTION 0080a02a
TBOOL PDebugMenuTree::Remove( const TCHAR* a_szPath )
{
	PDebugMenuNode* pParent = TNULL;
	PDebugNode*     pNode   = FindPath( a_szPath, &pParent );

	if ( !pNode || !pParent )
		return TFALSE;

	TINT iCount = pParent->m_oChildren.Size();

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( pParent->m_oChildren[ i ] == pNode )
		{
			pParent->m_oChildren[ i ] = pParent->m_oChildren[ iCount - 1 ];
			pParent->m_oChildren.SetSize( pParent->m_oChildren.Size() - 1 );
			pNode->Clear();
			delete pNode;
			break;
		}
	}

	m_iDepth           = 1;
	m_aiSelection[ 0 ] = ( m_pRoot->m_oChildren.Size() > 0 ) - 1;
	return TTRUE;
}

// $deBlob2: FUNCTION 0080a1a5
void PDebugMenuTree::Draw( TFLOAT a_fX, TFLOAT a_fY, TFLOAT* a_pHeight )
{
	TFLOAT fHeight = 0.0f;
	*a_pHeight     = 0.0f;

	if ( m_bShowPath )
	{
		DrawPath( a_fX, a_fY, &fHeight );
		*a_pHeight += fHeight;
	}

	TFLOAT fY = fHeight + a_fY;
	GetLevel( m_iDepth - 1 )->DrawChildren( GetSelectedId(), a_fX, fY, m_fScale, m_fAlpha, &fHeight );
	*a_pHeight += fHeight;
}

// The open menus' names, "./debug/"
// $deBlob2: FUNCTION 0080a617
void PDebugMenuTree::DrawPath( TFLOAT a_fX, TFLOAT a_fY, TFLOAT* a_pHeight )
{
	TCHAR           szPath[ 0x100 ];
	PDebugMenuNode* pMenu = m_pRoot;

	memcpy( szPath, pMenu->GetName(), pMenu->GetNameLength() );
	TINT iLength        = pMenu->GetNameLength();
	szPath[ iLength++ ] = '/';

	for ( TINT i = 1; i < m_iDepth; i++ )
	{
		pMenu = TSTATICCAST( PDebugMenuNode, pMenu->m_oChildren[ m_aiSelection[ i - 1 ] ] );

#  ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: Prime writes past the end of the path when the menus are nested deep enough
		if ( iLength + pMenu->GetNameLength() + 2 > TINT( sizeof( szPath ) ) )
			break;
#  endif

		memcpy( szPath + iLength, pMenu->GetName(), pMenu->GetNameLength() );
		iLength += pMenu->GetNameLength();
		szPath[ iLength++ ] = '/';
	}

	szPath[ iLength ] = '\0';

	PDebugNode::DrawBar( PDebugText::GetCharWidth() * 50.0f, PDebugText::GetLineHeight(), a_fX, a_fY, m_fScale, 0xFF000000 );
	PDebugText::Draw( szPath, a_fX, a_fY, 0xFFF5F5F5, m_fScale, 0, TNULL, a_pHeight );
}

// Closes every menu and removes every entry
// $deBlob2: FUNCTION 0080a736
void PDebugMenuTree::Reset()
{
	m_pRoot->Clear();
	m_iDepth           = 1;
	m_aiSelection[ 0 ] = -1;
}

// $deBlob2: FUNCTION 0080a77b
TBOOL PDebugMenuTree::SetTyped( const TCHAR* a_szText )
{
	PDebugNode* pNode = GetSelected();

	if ( !pNode )
		return TFALSE;

	return pNode->SetTyped( a_szText );
}

// $deBlob2: FUNCTION 008083fd
void PDebugMenuTree::SetScale( TFLOAT a_fScale )
{
	m_fScale = a_fScale;
}

// $deBlob2: FUNCTION 00808411
void PDebugMenuTree::SetAlpha( TFLOAT a_fAlpha )
{
	m_fAlpha = a_fAlpha;
}

//-----------------------------------------------------------------------------
// PDebugMenu
//-----------------------------------------------------------------------------

// The variables are removed by the destructor of Prime's menu system object
// $deBlob2: FUNCTION 00807d0c
void PDebugMenu::RemoveVars()
{
	ms_oTransparency.Remove();
	ms_oTextSize.Remove();
}

// $deBlob2: FUNCTION 00807dd1
TBOOL PDebugMenu::PushTyped( TCHAR a_cChar )
{
	TINT iCount = ms_iNumTyped;

	if ( iCount < MAX_TYPED )
	{
		ms_szTyped[ iCount ] = a_cChar;
		ms_iNumTyped++;
	}

	return iCount < MAX_TYPED;
}

// $deBlob2: FUNCTION 008083d1
void PDebugMenu::ClearTyped()
{
	memset( ms_szTyped, 0, sizeof( ms_szTyped ) );
	ms_iNumTyped = 0;
}

// DOES_NOTHING
// $deBlob2: FUNCTION 0080a9b5
void PDebugMenu::OnVisibilityChanged()
{
}

// $deBlob2: FUNCTION 00807d44
TBOOL PDebugMenu::SetVisible( TBOOL a_bVisible )
{
	if ( !ms_bEnabled )
		return TFALSE;

	ms_bVisible = a_bVisible;
	ClearTyped();
	OnVisibilityChanged();
	return TTRUE;
}

// $deBlob2: FUNCTION 00807d70
TINT PDebugMenu::AddCommandLine( const TCHAR* a_szParent, const TCHAR* a_szName, const TCHAR* a_szCommand )
{
	return ms_pMenu->AddCommandLine( a_szParent, a_szName, a_szCommand );
}

// $deBlob2: FUNCTION 00807d89
TINT PDebugMenu::AddLabel( const TCHAR* a_szParent, const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData )
{
	return ms_pMenu->AddLabel( a_szParent, a_szName, a_fnCallback, a_pUserData );
}

// $deBlob2: FUNCTION 00807da5
TINT PDebugMenu::AddSubmenu( const TCHAR* a_szParent, const TCHAR* a_szName )
{
	return ms_pMenu->AddSubmenu( a_szParent, a_szName );
}

// $deBlob2: FUNCTION 00807dbb
TINT PDebugMenu::AddVar( const TCHAR* a_szParent, const TCHAR* a_szName )
{
	return ms_pMenu->AddVar( a_szParent, a_szName );
}

// NOTE: Prime returns the id as the command's result, so adding to the root's first id (0, never given) or a
// failure (-1) both count as success; kept
// $deBlob2: FUNCTION 00807dfa
TBOOL PDebugMenu::AddCommandLineCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 4 )
		return TFALSE;

	return AddCommandLine( a_ppArgv[ 1 ], a_ppArgv[ 2 ], a_ppArgv[ 3 ] ) != 0;
}

// $deBlob2: FUNCTION 00807e1d
TBOOL PDebugMenu::AddLabelCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 3 )
		return TFALSE;

	return ms_pMenu->AddLabel( a_ppArgv[ 1 ], a_ppArgv[ 2 ], TNULL, TNULL ) != 0;
}

// $deBlob2: FUNCTION 00807e44
TBOOL PDebugMenu::AddSubmenuCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 3 )
		return TFALSE;

	return AddSubmenu( a_ppArgv[ 1 ], a_ppArgv[ 2 ] ) != 0;
}

// $deBlob2: FUNCTION 00807e63
TBOOL PDebugMenu::AddVarCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 3 )
		return TFALSE;

	return AddVar( a_ppArgv[ 1 ], a_ppArgv[ 2 ] ) != 0;
}

// $deBlob2: FUNCTION 00807e82
TBOOL PDebugMenu::RemoveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 2 )
		return TFALSE;

	return Remove( a_ppArgv[ 1 ] );
}

// $deBlob2: FUNCTION 00807e9d
TBOOL PDebugMenu::ResetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 1 )
		return TFALSE;

	Reset( TTRUE );
	return TTRUE;
}

// $deBlob2: FUNCTION 00807eb7
void PDebugMenu::OnVarChanged( PCVar* a_pVar )
{
	if ( a_pVar == &ms_oTextSize )
	{
		ms_pMenu->SetScale( ms_oTextSize.GetFloat() );
		return;
	}

	if ( a_pVar == &ms_oTransparency )
		ms_pMenu->SetAlpha( ms_oTransparency.GetFloat() );
}

// $deBlob2: FUNCTION 00807eef
void PDebugMenu::AddDefaults()
{
	AddSubmenu( ROOT, "debug" );
	AddVar( "./debug", "debug.text_size" );
	AddVar( "./debug", "debug.transparency" );
}

// NOTE: Prime draws the box with the white texture through geDebugDraw
// $deBlob2: FUNCTION 00807f2e
void PDebugMenu::DrawBox( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fX, TFLOAT a_fY, TUINT32 a_uiColour )
{
	if ( PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer() )
		pRenderer->DrawRect( a_fX, a_fY, a_fWidth, a_fHeight, a_uiColour );
}

// $deBlob2: FUNCTION 00807fa3
void PDebugMenu::SetEnabled( TBOOL a_bEnabled )
{
	if ( !a_bEnabled )
		SetVisible( TFALSE );

	ms_bEnabled = a_bEnabled;
}

// $deBlob2: FUNCTION 00807fc9
TBOOL PDebugMenu::GetSelectedLabelData( void** a_ppUserData )
{
	return ms_pMenu->GetSelectedLabelData( a_ppUserData );
}

// Up/down: the previous or next entry, escape: back, return: in; left/right (or return held): change the entry's
// value. Returns whether anything was done.
// $deBlob2: FUNCTION 00807fe8
TBOOL PDebugMenu::HandleInput( TFLOAT a_fDeltaTime )
{
	PDebugMenuTree* pMenu = ms_pMenu;

	if ( PKeyboard::IsPressed( PKeyboard::KEY_UP ) )
	{
		pMenu->Prev( TFALSE, TTRUE );
	}
	else if ( PKeyboard::IsPressed( PKeyboard::KEY_DOWN ) )
	{
		pMenu->Next( TTRUE );
	}
	else if ( PKeyboard::IsReleased( PKeyboard::KEY_ESCAPE ) )
	{
		pMenu->Back();
	}
	else if ( PKeyboard::IsReleased( PKeyboard::KEY_RETURN ) )
	{
		pMenu->Enter();
	}
	else if ( !PKeyboard::IsDown( PKeyboard::KEY_RETURN ) && !PKeyboard::IsDown( PKeyboard::KEY_RIGHT ) )
	{
		if ( !PKeyboard::IsDown( PKeyboard::KEY_LEFT ) )
		{
			ms_fHeldTime = 0.0f;
			return TFALSE;
		}

		ms_fHeldTime = ms_fHeldTime + a_fDeltaTime;
		pMenu->Decrease( ms_fHeldTime, a_fDeltaTime );
	}
	else
	{
		ms_fHeldTime = ms_fHeldTime + a_fDeltaTime;
		pMenu->Increase( ms_fHeldTime, a_fDeltaTime, TFALSE );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 00808172
TBOOL PDebugMenu::Remove( const TCHAR* a_szPath )
{
	return ms_pMenu->Remove( a_szPath );
}

// $deBlob2: FUNCTION 00808185
void PDebugMenu::Draw()
{
	if ( !ms_bVisible )
		return;

	TFLOAT fScale      = ms_oTextSize.GetFloat();
	TFLOAT fCharWidth  = PDebugText::GetCharWidth() * fScale;
	TFLOAT fLineHeight = PDebugText::GetLineHeight() * fScale;

	TUINT  uiLines  = TUINT( ms_pMenu->GetCount() + 3 );
	TFLOAT fHeight  = TFLOAT( double( uiLines ) ) * fLineHeight + fLineHeight * 2.0f;
	TFLOAT fWidth   = fCharWidth * 54.0f;
	TFLOAT fBorder  = fCharWidth * 0.25f;
	TFLOAT fShading = 1.0f;

#  ifdef BARNYARD_COMMUNITY_PATCH
	TUINT32 uiAlpha = TUINT32( TINT( ms_oTransparency.GetFloat() * 255.0f ) ) << 24;
	TFLOAT  fPixelX = 1.0f / 640.0f;
	TFLOAT  fPixelY = 1.0f / 480.0f;

	if ( PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer() )
	{
		fPixelX = 1.0f / TFLOAT( pRenderer->GetScreenWidth() );
		fPixelY = 1.0f / TFLOAT( pRenderer->GetScreenHeight() );
	}

	TFLOAT fBorderX = TMath::Max( fBorder * 0.5f * fScale, fPixelX );
	TFLOAT fBorderY = TMath::Max( fBorder * 0.5f * fScale * ( fPixelY / fPixelX ), fPixelY );

	TFLOAT fLeft = PDebugText::GetCharWidth() * 5.0f;
	TFLOAT fTop  = PDebugText::GetLineHeight();
	DrawBox( fWidth, fBorderY, fLeft, fTop, uiAlpha | 0xC0C0C0 );
	DrawBox( fWidth, fBorderY, fLeft, fTop + fHeight - fBorderY, uiAlpha | 0xC0C0C0 );
	DrawBox( fBorderX, fHeight - fBorderY * 2.0f, fLeft, fTop + fBorderY, uiAlpha | 0xC0C0C0 );
	DrawBox( fBorderX, fHeight - fBorderY * 2.0f, fLeft + fWidth - fBorderX, fTop + fBorderY, uiAlpha | 0xC0C0C0 );
	DrawBox( fWidth - fBorderX * 2.0f, fHeight - fBorderY * 2.0f, fLeft + fBorderX, fTop + fBorderY, uiAlpha | 0x696969 );
#  else
	DrawBox( fWidth, fHeight, PDebugText::GetCharWidth() * 5.0f, PDebugText::GetLineHeight(), 0xFFC0C0C0 );

	fShading  = fShading * fBorder;
	TFLOAT fY = PDebugText::GetLineHeight() + fShading * 0.5f * fScale;
	TFLOAT fX = PDebugText::GetCharWidth() * 5.0f + fBorder * 0.5f * fScale;
	DrawBox( fWidth - fBorder, fHeight - fShading, fX, fY, 0xFF696969 );
#  endif

	PDebugText::SetCursorX( PDebugText::GetCharWidth() * 7.0f );
	PDebugText::SetCursorY( PDebugText::GetLineHeight() * 3.0f );
	PDebugText::Print( "Prime Debug Menu", 0xFFF5F5F5, fScale );
	PDebugText::SetTabSize( 40 );

	TFLOAT fMenuHeight = 0.0f;
	ms_pMenu->Draw( PDebugText::GetCursorX(), PDebugText::GetCursorY(), &fMenuHeight );
	PDebugText::SetCursorY( PDebugText::GetCursorY() + fMenuHeight );
	PDebugText::SetTabSize( 7 );
}

// $deBlob2: FUNCTION 008083ab
void PDebugMenu::Reset( TBOOL a_bDefaults )
{
	ms_pMenu->Reset();
	ClearTyped();

	if ( a_bDefaults )
		AddDefaults();
}

// $deBlob2: FUNCTION 008083f0
void PDebugMenu::SetPad( TINT a_iPad )
{
	ms_iPad = a_iPad;
}

// $deBlob2: FUNCTION 00808425
void PDebugMenu::Shutdown()
{
	if ( !ms_bStarted )
		return;

	Reset( TFALSE );
	ms_oAddSubmenuFunc.Remove();
	ms_oAddLabelFunc.Remove();
	ms_oAddVarFunc.Remove();
	ms_oAddCommandLineFunc.Remove();
	ms_oRemoveFunc.Remove();
	ms_oResetFunc.Remove();

	if ( ms_pMenu )
		delete ms_pMenu;

	ms_pMenu    = TNULL;
	ms_bStarted = TFALSE;

	RemoveVars();
}

// $deBlob2: FUNCTION 008084a5
TBOOL PDebugMenu::Startup()
{
	ms_oTextSize.CreateFloatRange( "debug.text_size", "Text size", PCVar::FLAGS_VALIDATERANGE, 1.0f, OnVarChanged, TNULL, 0.3f, 1.0f, 0.1f );
	ms_oTransparency.CreateFloatRange( "debug.transparency", "Debug background transparency", PCVar::FLAGS_VALIDATERANGE, 0.7f, OnVarChanged, TNULL, 0.3f, 1.0f, 0.1f );

	ms_pMenu    = new PDebugMenuTree( ms_oTextSize.GetFloat(), ms_oTransparency.GetFloat() );
	ms_bVisible = TFALSE;
	ms_bStarted = TTRUE;
	ms_bEnabled = TTRUE;
	ms_iPad     = 0;
	AddDefaults();

	ms_oAddSubmenuFunc.Create( "debug.menu.add_submenu", "Adds submenu to debug menu.", "debug_menu_add_submenu parent_name name", AddSubmenuCommand, 2, TNULL );
	ms_oAddLabelFunc.Create( "debug.menu.add_label", "Adds label to debug menu.", "debug_menu_add_label parent_name name", AddLabelCommand, 2, TNULL );
	ms_oAddVarFunc.Create( "debug.menu.add_var", "Adds existing variable to debug menu.", "debug_menu_add_var parent_name name", AddVarCommand, 2, TNULL );
	ms_oAddCommandLineFunc.Create( "debug.menu.add_command_line", "Adds command line to debug menu.", "debug_menu_add_command_line parent_name name command_line", AddCommandLineCommand, 3, TNULL );
	ms_oRemoveFunc.Create( "debug.menu.remove", "Removes element from debug menu.", "debug_menu_remove name", RemoveCommand, 1, TNULL );
	ms_oResetFunc.Create( "debug.menu.reset", "Resets debug menu to its initial state (removes all elements).", "debug_menu_reset", ResetCommand, 0, TNULL );
	return TTRUE;
}

// The digit keys (and the numpad's) and '.' typed this frame; only the last one counts
// $deBlob2: FUNCTION 008086fe
void PDebugMenu::ReadTyped()
{
	if ( ms_iNumTyped >= MAX_TYPED )
		return;

	TCHAR cChar = '\0';

	for ( TINT i = PKeyboard::KEY_0; i < PKeyboard::KEY_A; i++ )
	{
		if ( PKeyboard::IsPressed( i ) )
			cChar = TCHAR( '0' + i );
	}

	for ( TINT i = PKeyboard::KEY_NUMPAD0; i < PKeyboard::KEY_ESCAPE; i++ )
	{
		if ( PKeyboard::IsPressed( i ) )
			cChar = TCHAR( i - 3 );
	}

	if ( PKeyboard::IsPressed( PKeyboard::KEY_PERIOD ) || PKeyboard::IsPressed( PKeyboard::KEY_DECIMAL ) )
		cChar = '.';

	if ( cChar != '\0' )
		PushTyped( cChar );
}

// Home: shows or hides the menu
// $deBlob2: FUNCTION 0080867b
void PDebugMenu::Update( TFLOAT a_fDeltaTime )
{
	if ( !ms_bStarted )
		return;

	if ( PKeyboard::IsPressed( PKeyboard::KEY_HOME ) )
	{
		SetVisible( !ms_bVisible );
		return;
	}

	if ( !ms_bVisible )
		return;

	ReadTyped();

	if ( ms_iNumTyped > 0 )
		ms_pMenu->SetTyped( ms_szTyped );

	if ( HandleInput( a_fDeltaTime ) )
		ClearTyped();
}

#endif // PRIMEDEBUG_DEV
