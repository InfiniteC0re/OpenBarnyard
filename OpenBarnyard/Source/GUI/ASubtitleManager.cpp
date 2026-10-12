#include "pch.h"
#include "ASubtitleManager.h"
#include "GUI/AGUI2.h"
#include "GUI/T2GUIFontManager.h"
#include "Locale/ALocaleManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0066e340, CreateObject 0066e530, CreateObjectInPlace 0066e590
TDEFINE_CLASS( ASubtitleManager );

// $Barnyard: FUNCTION 0066e1f0
ASubtitle::ASubtitle()
{
	m_pData      = TNULL;
	m_iLine      = 0;
	m_iTime      = 0;
	m_wszText    = TNULL;
	m_fTimeScale = 1.0f;
}

// $Barnyard: FUNCTION 0066e080
// $Barnyard: FUNCTION 0066e230
ASubtitle::~ASubtitle()
{
}

// $Barnyard: FUNCTION 0066e090
void ASubtitle::Reset()
{
	m_pData      = TNULL;
	m_iLine      = 0;
	m_iTime      = 0;
	m_wszText    = TNULL;
	m_fTimeScale = 1.0f;
	m_oText.SetText( TNULL );
	m_oText.RemoveSelf();
}

// $Barnyard: FUNCTION 0066e2e0
void ASubtitle::Start( const Data* a_pData )
{
	Reset();
	m_pData = a_pData;

	// NOTE: the original checks the start of the line after the last one, which is out of the data
	if ( a_pData->iNumLines > 0 && a_pData->aLines[ a_pData->iNumLines ].iStart == 0 )
		m_wszText = a_pData->aLines[ 0 ].wszText;

	Show();
}

// $Barnyard: FUNCTION 0066e0d0
void ASubtitle::SetDuration( TINT a_iDuration )
{
	if ( m_pData )
	{
		TINT iLength = m_pData->aLines[ m_pData->iNumLines - 1 ].iEnd;

		if ( a_iDuration != iLength )
			m_fTimeScale = TFLOAT( a_iDuration ) / TFLOAT( iLength );
	}
}

// $Barnyard: FUNCTION 0066e100
void ASubtitle::SetTime( TINT a_iTime )
{
	if ( !m_pData )
		return;

	// Find the line of the time, from the current one unless the time goes back
	TINT iLine     = ( a_iTime >= m_iTime ) ? m_iLine : 0;
	TINT iLastLine = m_pData->iNumLines - 1;

	while ( iLine < iLastLine && a_iTime >= TINT( m_pData->aLines[ iLine ].iEnd * m_fTimeScale ) )
		iLine++;

	const Line& rLine = m_pData->aLines[ iLine ];
	m_iTime           = a_iTime;
	m_iLine           = iLine;

	if ( a_iTime < TINT( rLine.iStart * m_fTimeScale ) || a_iTime >= TINT( rLine.iEnd * m_fTimeScale ) )
	{
		// Between two lines
		m_wszText = TNULL;
		m_oText.SetText( m_wszText );
	}
	else
	{
		m_wszText = rLine.wszText;
		m_oText.SetText( m_wszText );
	}
}

// $Barnyard: FUNCTION 0066e1c0
TBOOL ASubtitle::Update( TFLOAT a_fDeltaTime )
{
	SetTime( m_iTime - TINT( a_fDeltaTime * -1000.0f ) );
	return m_wszText != TNULL;
}

// $Barnyard: FUNCTION 0066e250
void ASubtitle::Show()
{
	T2GUIElement* pRootElement = AGUI2::GetRootElement();
	T2GUIFont*    pFont        = T2GUIFontManager::FindFont( "Rekord18" );
	TFLOAT        fWidth       = pRootElement->GetWidth() - 24.0f;

	m_oText.RemoveSelf();
	m_oText.Create( pFont, fWidth );
	m_oText.SetTransform( 0.0f, -16.0f );
	m_oText.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Center );
	m_oText.SetText( m_wszText );
	m_oText.Show();
	pRootElement->AddChildTail( m_oText );
}

// $Barnyard: FUNCTION 0066e360
ASubtitleManager::ASubtitleManager()
{
	m_pTRB = TNULL;

	for ( TINT i = 0; i < MAX_NUM_SUBTITLES; i++ )
		m_llFreeSubtitles.PushBack( new ASubtitle() );
}

// $Barnyard: FUNCTION 0066e7c0
// $Barnyard: FUNCTION 0066e930
ASubtitleManager::~ASubtitleManager()
{
	while ( !m_llManagedSubtitles.IsEmpty() )
		StopSubtitle( m_llManagedSubtitles.PopBack() );

	while ( !m_llFreeSubtitles.IsEmpty() )
		delete m_llFreeSubtitles.PopBack().Get();

	UnloadSubtitles();

	m_llFreeSubtitles.Clear();
	m_llSubtitles.Clear();
	m_llManagedSubtitles.Clear();
}

// $Barnyard: FUNCTION 0066e8c0
TBOOL ASubtitleManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_llManagedSubtitles.Begin(); it != m_llManagedSubtitles.End(); )
	{
		ASubtitle* pSubtitle = it;
		it++;

		if ( !pSubtitle->Update( a_fDeltaTime ) )
			StopSubtitle( pSubtitle );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0066e5d0
TBOOL ASubtitleManager::LoadSubtitles( const TCHAR* a_szLibrary )
{
	UnloadSubtitles();

	const TCHAR* szLanguage = ( g_pLocaleManager->GetLanguage() == ALocaleManager::Lang_EnglishUK ) ? "GBR" : ALocaleManager::GetLanguageName( g_pLocaleManager->GetLanguage() );

	TCHAR szFileName[ 256 ];
	TStringManager::String8Format( szFileName, sizeof( szFileName ), "Data/Subs/%s/%s.trb", szLanguage, a_szLibrary );

	m_pTRB = new TTRB();
	m_pTRB->Load( szFileName );
	m_pTRB->GetSymbolAddress( "SubtitleVersion" );

	return TTRUE;
}

// $Barnyard: FUNCTION 0066e420
void ASubtitleManager::UnloadSubtitles()
{
	if ( m_pTRB )
	{
		m_pTRB->Close();
		delete m_pTRB;
		m_pTRB = TNULL;
	}
}

// $Barnyard: FUNCTION 0066e450
ASubtitle* ASubtitleManager::PlaySubtitle( const TCHAR* a_szName, TFLOAT a_fDuration, TBOOL a_bManaged )
{
	if ( !m_pTRB )
		return TNULL;

	const ASubtitle::Data* pData = TSTATICCAST( const ASubtitle::Data, m_pTRB->GetSymbolAddress( a_szName ) );

	if ( !pData || m_llFreeSubtitles.IsEmpty() )
		return TNULL;

	ASubtitle* pSubtitle = m_llFreeSubtitles.PopBack();
	pSubtitle->Start( pData );

	if ( a_fDuration > 0.0f )
		pSubtitle->SetDuration( TINT( a_fDuration * 1000.0f ) );

	if ( a_bManaged )
		m_llManagedSubtitles.PushBack( pSubtitle );
	else
		m_llSubtitles.PushBack( pSubtitle );

	return pSubtitle;
}

// $Barnyard: FUNCTION 0066e6e0
void ASubtitleManager::StopSubtitle( ASubtitle* a_pSubtitle )
{
	a_pSubtitle->Reset();

	if ( !a_pSubtitle->IsLinked() )
		return;

	// Back to the free subtitles
	for ( auto it = m_llManagedSubtitles.Begin(); it != m_llManagedSubtitles.End(); it++ )
	{
		if ( it == a_pSubtitle )
		{
			m_llManagedSubtitles.Transfer( a_pSubtitle, m_llFreeSubtitles );
			return;
		}
	}

	for ( auto it = m_llSubtitles.Begin(); it != m_llSubtitles.End(); it++ )
	{
		if ( it == a_pSubtitle )
		{
			m_llSubtitles.Transfer( a_pSubtitle, m_llFreeSubtitles );
			return;
		}
	}
}
