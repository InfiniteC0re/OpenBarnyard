#include "PrimeDebugPCH.h"
#include "PDebugMenuNode.h"
#include "Console/PConsole.h"
#include "Draw/PDebugText.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT PDebugNode::ms_iLastNodeId;

// NOTE: Prime draws the bar with the white texture through geDebugDraw
// $deBlob2: FUNCTION 00809920
void PDebugNode::DrawBar( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TUINT32 a_uiColour )
{
	TFLOAT fGap      = PDebugText::GetLineHeight() * 0.0625f;
	TFLOAT fHalfChar = PDebugText::GetCharWidth() * 0.5f;
	TFLOAT fHalfLine = PDebugText::GetLineHeight() * 0.5f;

	if ( PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer() )
		pRenderer->DrawRect( a_fX - fHalfChar * a_fScale, a_fY - fHalfLine * a_fScale, a_fWidth * a_fScale, ( a_fHeight - fGap ) * a_fScale, a_uiColour );
}

//-----------------------------------------------------------------------------
// PDebugNode
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00809024
PDebugNode::PDebugNode( TYPE a_eType, const TCHAR* a_szName )
{
	m_eType = a_eType;
	m_iId   = ++ms_iLastNodeId;

	TINT iLength  = TINT( strlen( a_szName ) );
	m_iNameLength = MAX_NAME_LENGTH;

	if ( iLength < MAX_NAME_LENGTH )
		m_iNameLength = iLength;

	// NOTE: Prime copies a longer name cut to MAX_NAME_LENGTH characters without its terminator
	TCHAR szName[ MAX_NAME_LENGTH + 1 ];
	memcpy( szName, a_szName, m_iNameLength );
	szName[ m_iNameLength ] = '\0';
	m_strName               = szName;
}

// $deBlob2: FUNCTION 008091bd
PDebugNode::~PDebugNode()
{
}

// $deBlob2: FUNCTION 00809d2a
TBOOL PDebugNode::Find( TINT a_iId, PDebugNode** a_ppNode )
{
	TBOOL bFound = a_iId == m_iId;

	if ( bFound )
		*a_ppNode = this;

	return bFound;
}

// $deBlob2: FUNCTION 0080a793
TBOOL PDebugNode::SetTyped( const TCHAR* a_szText )
{
	return TFALSE;
}

// $deBlob2: FUNCTION 008098e6
void PDebugNode::Clear()
{
}

// $deBlob2: FUNCTION 0080a342
void PDebugNode::Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight )
{
	*a_pHeight = 0.0f;
	DrawBar( PDebugText::GetCharWidth() * 50.0f, PDebugText::GetLineHeight(), a_fX, a_fY, a_fScale, GetColour( a_iSelectedId, a_fAlpha ) );
	PDebugText::Draw( m_strName.GetString(), a_fX, a_fY, PDebugText::COLOUR_BLACK, a_fScale, 0, TNULL, a_pHeight );
}

// $deBlob2: FUNCTION 00809c2b
void PDebugNode::Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
}

// $deBlob2: FUNCTION 008098e7
TUINT32 PDebugNode::GetColour( TINT a_iSelectedId, TFLOAT a_fAlpha )
{
	TUINT32 uiColour = ( a_iSelectedId == m_iId ) ? SELECTED_COLOUR : COLOUR;

	if ( a_iSelectedId == m_iId )
		a_fAlpha = 1.0f;

	return TUINT32( TINT( a_fAlpha * 255.0f ) ) << 24 | ( uiColour & 0xFFFFFF );
}

//-----------------------------------------------------------------------------
// PDebugMenuNode
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00808ff5
PDebugMenuNode::PDebugMenuNode( const TCHAR* a_szName )
    : PDebugNode( TYPE_MENU, a_szName )
{
}

// $deBlob2: FUNCTION 008090e5
// $deBlob2: FUNCTION 0080919e
PDebugMenuNode::~PDebugMenuNode()
{
	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
		delete m_oChildren[ i ];
}

// $deBlob2: FUNCTION 00809ce2
TBOOL PDebugMenuNode::Find( TINT a_iId, PDebugNode** a_ppNode )
{
	if ( m_iId == a_iId )
	{
		*a_ppNode = this;
		return TTRUE;
	}

	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		if ( m_oChildren[ i ]->Find( a_iId, a_ppNode ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 008098b0
void PDebugMenuNode::Clear()
{
	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		m_oChildren[ i ]->Clear();
		delete m_oChildren[ i ];
	}

	m_oChildren.Free();
}

// $deBlob2: FUNCTION 0080a257
void PDebugMenuNode::Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight )
{
	*a_pHeight = 0.0f;

	TCHAR szText[ 0xC0 ];
	PString::Format( szText, sizeof( szText ), "[%2d] %s", m_oChildren.Size(), m_strName.GetString() );

	DrawBar( PDebugText::GetCharWidth() * 50.0f, PDebugText::GetLineHeight(), a_fX, a_fY, a_fScale, GetColour( a_iSelectedId, a_fAlpha ) );
	PDebugText::Draw( szText, a_fX, a_fY, PDebugText::COLOUR_BLACK, a_fScale, 0, TNULL, a_pHeight );
}

// $deBlob2: FUNCTION 00809a0e
PDebugNode* PDebugMenuNode::FindPath( const TCHAR* a_szPath, TINT a_iLength, PDebugMenuNode** a_ppParent )
{
	if ( strcoll( m_strName.GetString(), a_szPath ) == 0 )
		return this;

	TINT iLength = m_iNameLength;

	if ( strncmp( m_strName.GetString(), a_szPath, iLength ) != 0 || iLength >= a_iLength || a_szPath[ iLength ] != '/' )
		return TNULL;

	const TCHAR* szRest = a_szPath + iLength + 1;

	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		PDebugNode* pChild = m_oChildren[ i ];

		if ( pChild->GetType() == TYPE_MENU )
		{
			PDebugNode* pFound = TSTATICCAST( PDebugMenuNode, pChild )->FindPath( szRest, ( a_iLength - iLength ) - 1, a_ppParent );

			if ( pFound )
			{
				if ( a_ppParent && !*a_ppParent )
					*a_ppParent = this;

				return pFound;
			}
		}
		else if ( TUINT( pChild->GetType() - 1 ) < 3 && strcoll( szRest, pChild->GetName() ) == 0 )
		{
			if ( a_ppParent )
				*a_ppParent = this;

			return m_oChildren[ i ];
		}
	}

	return TNULL;
}

// $deBlob2: FUNCTION 0080982a
TBOOL PDebugMenuNode::HasChild( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		if ( strcoll( a_szName, m_oChildren[ i ]->GetName() ) == 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 00809865
TBOOL PDebugMenuNode::FindChild( const TCHAR* a_szName, PDebugNode** a_ppChild )
{
	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		if ( strcoll( a_szName, m_oChildren[ i ]->GetName() ) == 0 )
		{
			if ( a_ppChild )
				*a_ppChild = m_oChildren[ i ];

			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080a593
void PDebugMenuNode::DrawChildren( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight )
{
	TFLOAT fHeight = 0.0f;
	*a_pHeight     = 0.0f;

	for ( TINT i = 0; i < m_oChildren.Size(); i++ )
	{
		TFLOAT fLine = 0.0f;
		m_oChildren[ i ]->Draw( a_iSelectedId, a_fX, fHeight + a_fY, a_fScale, a_fAlpha, &fLine );
		fHeight = fLine + fHeight;
	}

	*a_pHeight = fHeight;
}

//-----------------------------------------------------------------------------
// PDebugLabelNode
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00808f65
PDebugLabelNode::PDebugLabelNode( const TCHAR* a_szName, PDebugLabelCallback a_fnCallback, void* a_pUserData )
    : PDebugNode( TYPE_LABEL, a_szName )
{
	m_fnCallback = a_fnCallback;
	m_pUserData  = a_pUserData;
}

// $deBlob2: FUNCTION 00809175
PDebugLabelNode::~PDebugLabelNode()
{
}

// $deBlob2: FUNCTION 00809c0d
void PDebugLabelNode::Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
#  ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Prime calls back on every frame Return or Right is held; here only on the first one
	if ( a_fHeldTime != a_fDeltaTime )
		return;
#  endif

	if ( a_bIncrease && m_fnCallback )
		m_fnCallback( m_iId, m_pUserData );
}

//-----------------------------------------------------------------------------
// PDebugVarNode
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00809080
PDebugVarNode::PDebugVarNode( const TCHAR* a_szName, PCVar* a_pVar )
    : PDebugNode( TYPE_VAR, a_szName )
{
	m_fValue  = 0.0f;
	m_szTyped = TNULL;
	m_pVar    = a_pVar;
}

// $deBlob2: FUNCTION 008091e6
PDebugVarNode::~PDebugVarNode()
{
}

// $deBlob2: FUNCTION 0080a798
TBOOL PDebugVarNode::SetTyped( const TCHAR* a_szText )
{
	m_szTyped = a_szText;
	return TTRUE;
}

// The last part of the name (after the last '.') and the value, or what is being typed
// $deBlob2: FUNCTION 0080a402
void PDebugVarNode::Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight )
{
	*a_pHeight = 0.0f;

	TCHAR szName[ 0x80 ];
	PString::CopyToLower( szName, sizeof( szName ), m_strName.GetString() );

	TCHAR* szLast = szName;
	TCHAR* pPart  = szName;
	TINT   iSize  = TINT( strlen( m_strName.GetString() ) );

	if ( iSize > 0 )
	{
		do
		{
			szLast = PString::Tokenize( pPart, "." );

			if ( !szLast )
			{
				szLast = pPart;
				break;
			}

			pPart += strlen( szLast ) + 1;
		} while ( pPart - szName < iSize );
	}

	TCHAR szText[ 0xC0 ];

	if ( !m_szTyped )
	{
		TCHAR szValue[ 0x80 ];
		m_pVar->FormatValue( szValue, sizeof( szValue ) );
		PString::Format( szText, sizeof( szText ), "%s \t[c3]%s", szLast, szValue );
	}
	else
	{
		PString::Format( szText, sizeof( szText ), "%s \t[c3]%s", szLast, m_szTyped );
	}

	m_szTyped = TNULL;

	DrawBar( PDebugText::GetCharWidth() * 50.0f, PDebugText::GetLineHeight(), a_fX, a_fY, a_fScale, GetColour( a_iSelectedId, a_fAlpha ) );
	PDebugText::Draw( szText, a_fX, a_fY, PDebugText::COLOUR_BLACK, a_fScale, 0, TNULL, a_pHeight );
}

// $deBlob2: FUNCTION 00809c2e
void PDebugVarNode::Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
	if ( m_szTyped )
	{
		ApplyTyped();
		return;
	}

	switch ( m_pVar->GetType() )
	{
		case PCVar::TYPE_INT:
			if ( a_fHeldTime == a_fDeltaTime )
				m_pVar->StepInt( a_bIncrease, TNULL );
			break;
		case PCVar::TYPE_FLOAT:
			m_pVar->SetFloat( GetHeldValue( a_bIncrease, a_fHeldTime, a_fDeltaTime ), TNULL );
			break;
		case PCVar::TYPE_FLOATSPLAT:
			m_pVar->SetFloatSplat( GetHeldValue( a_bIncrease, a_fHeldTime, a_fDeltaTime ), TNULL );
			break;
		default:
			break;
	}
}

// The first frame moves one step; while held the value speeds up (with the square of the time held, over the whole
// range in three seconds) and is set in whole steps
// $deBlob2: FUNCTION 0080963d
TFLOAT PDebugVarNode::GetHeldValue( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
	TFLOAT fMin   = m_pVar->GetRange().fMin;
	TFLOAT fMax   = m_pVar->GetRange().fMax;
	TFLOAT fStep  = m_pVar->GetRange().fStep;
	TFLOAT fValue = m_pVar->GetFloat();

	if ( a_fHeldTime == a_fDeltaTime )
	{
		TFLOAT fNew = ( a_bIncrease ? 1.0f : -1.0f ) * fStep + fValue;
		m_fValue    = fNew;

		if ( fMax <= fNew )
			fNew = fMax;

		if ( fNew <= fMin )
			fNew = fMin;

		m_fValue = fNew;
		return fNew;
	}

	TFLOAT fBefore = ( a_fHeldTime - a_fDeltaTime ) / 3.0f;
	TFLOAT fNow    = a_fHeldTime / 3.0f;
	TFLOAT fNew    = ( a_bIncrease ? 1.0f : -1.0f ) * ( ( fNow * fNow * ( fMax - fMin ) + fMin ) - ( fBefore * fBefore * ( fMax - fMin ) + fMin ) ) + m_fValue;

	TFLOAT fClamped = fMax;

	if ( fNew < fMax )
		fClamped = fNew;

	if ( fClamped <= fMin )
		fClamped = fMin;

	m_fValue = fClamped;

	if ( fClamped != fNew )
		return fClamped;

	if ( !a_bIncrease )
	{
		if ( fClamped <= fValue - fStep )
			return fValue - fStep;
	}
	else if ( fValue + fStep <= fClamped )
	{
		return fValue + fStep;
	}

	return fValue;
}

// NOTE: the end of the parse isn't checked, so any text sets the variable (to 0 if it isn't a number)
// $deBlob2: FUNCTION 00809794
TBOOL PDebugVarNode::ApplyTyped()
{
	TBOOL bSet = TFALSE;

	switch ( m_pVar->GetType() )
	{
		case PCVar::TYPE_INT:
		{
			TCHAR* pEnd   = TNULL;
			TINT   iValue = PString::ToInt( m_szTyped, &pEnd, 0 );

			if ( pEnd )
			{
				m_pVar->SetInt( iValue, TNULL );
				bSet = TTRUE;
			}

			break;
		}
		case PCVar::TYPE_FLOAT:
		case PCVar::TYPE_FLOATSPLAT:
		{
			TCHAR* pEnd   = TNULL;
			TFLOAT fValue = PString::ToFloat( m_szTyped, &pEnd );

			if ( pEnd )
			{
				if ( m_pVar->GetType() == PCVar::TYPE_FLOAT )
					m_pVar->SetFloat( fValue, TNULL );
				else
					m_pVar->SetFloatSplat( fValue, TNULL );

#  ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: Prime doesn't update the held value, so while Return stays down after the typed value is set,
				// the next frames move on from the value the variable had before it (usually to the minimum)
				m_fValue = m_pVar->GetFloat();
#  endif

				bSet = TTRUE;
			}

			break;
		}
		default:
			break;
	}

	m_szTyped = TNULL;
	return bSet;
}

//-----------------------------------------------------------------------------
// PDebugCommandLineNode
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00808f3a
PDebugCommandLineNode::PDebugCommandLineNode( const TCHAR* a_szName, const TCHAR* a_szCommand )
    : PDebugNode( TYPE_COMMANDLINE, a_szName )
    , m_strCommand( a_szCommand )
{
}

// $deBlob2: FUNCTION 008090ac
// $deBlob2: FUNCTION 00809156
PDebugCommandLineNode::~PDebugCommandLineNode()
{
}

// $deBlob2: FUNCTION 0080a0bd
void PDebugCommandLineNode::Draw( TINT a_iSelectedId, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fScale, TFLOAT a_fAlpha, TFLOAT* a_pHeight )
{
	*a_pHeight = 0.0f;

	TCHAR szText[ 0xC0 ];
	PString::Format( szText, sizeof( szText ), "[%s]", m_strName.GetString() );

	DrawBar( PDebugText::GetCharWidth() * 50.0f, PDebugText::GetLineHeight(), a_fX, a_fY, a_fScale, GetColour( a_iSelectedId, a_fAlpha ) );
	PDebugText::Draw( szText, a_fX, a_fY, PDebugText::COLOUR_BLACK, a_fScale, 0, TNULL, a_pHeight );
}

// $deBlob2: FUNCTION 00809bec
void PDebugCommandLineNode::Activate( TBOOL a_bIncrease, TFLOAT a_fHeldTime, TFLOAT a_fDeltaTime )
{
#  ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: Prime runs the line on every frame Return or Right is held; here only on the first one
	if ( a_fHeldTime != a_fDeltaTime )
		return;
#  endif

	if ( a_bIncrease )
		PConsole::Execute( m_strCommand.GetString() );
}

#endif // PRIMEDEBUG_DEV
