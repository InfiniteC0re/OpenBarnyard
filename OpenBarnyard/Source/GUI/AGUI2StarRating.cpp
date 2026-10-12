#include "pch.h"
#include "AGUI2StarRating.h"
#include "AGUI2.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00637a70
AGUI2StarRating::AGUI2StarRating()
    : m_vPosition( 0.0f, 0.0f ) // NOTE: retail copies two zero floats of .bss (0x007d3018 / 0x007d301c)
    , m_iNumStars( 0 )
    , m_bVisible( TFALSE )
    , m_bMoving( TFALSE )
{
}

// $Barnyard: FUNCTION 00637380
AGUI2StarRating::~AGUI2StarRating()
{
	m_oLabel.RemoveSelf();
	m_oLabelRoot.RemoveSelf();

	for ( TINT i = NUM_STARS - 1; i >= 0; i-- )
		m_aStars[ i ].RemoveSelf();

	m_oPlank.RemoveSelf();
}

// $Barnyard: FUNCTION 00637b10
TBOOL AGUI2StarRating::Create( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fStarSize )
{
	m_vPosition.x = AGUI2::GetRootElement()->GetWidth() * a_fX;
	m_vPosition.y = AGUI2::GetRootElement()->GetHeight() * a_fY;

	T2GUIElement* pRoot       = AGUI2::GetRootElement();
	TFLOAT        fRootWidth  = pRoot->GetWidth();
	TFLOAT        fScreenSize = pRoot->GetHeight();
	if ( fScreenSize >= fRootWidth ) fScreenSize = AGUI2::GetRootElement()->GetWidth();
	else fScreenSize = AGUI2::GetRootElement()->GetHeight();

	TFLOAT fStarSize = fScreenSize * a_fStarSize;

	m_vStarStart.y = m_vPosition.y;
	TFLOAT fStarX  = fStarSize + fStarSize + m_vPosition.x;
	m_vStarStart.x = AGUI2::GetRootElement()->GetWidth() + fStarSize + 32.0f;

	// The plank behind the stars
	T2GUITextureSection* pPlankTexture = T2GUITextureSectionManager::GetTextureSection( "Wood_Thin" );
	m_oPlank.SetDimensions( fStarSize * 5.0f, fStarSize );
	m_oPlank.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLECENTER );
	m_oPlank.SetTextureSection( pPlankTexture );
	AGUI2::GetRootElement()->AddChildTail( m_oPlank );
	m_oPlank.SetTranslation( m_vPosition.x, m_vPosition.y );
	m_oPlank.SetAlpha( 0.5f );

	// The stars wait off screen, the last one is the rightmost
	T2GUITextureSection* pStarTexture = T2GUITextureSectionManager::GetTextureSection( "Statue_GoldStar" );
	for ( TINT i = NUM_STARS - 1; i >= 0; i-- )
	{
		m_aStarTargets[ i ].x = fStarX;
		m_aStarTargets[ i ].y = m_vPosition.y;
		fStarX -= fStarSize;

		m_aStarPositions[ i ] = m_vStarStart;

		m_aStars[ i ].SetDimensions( fStarSize, fStarSize );
		m_aStars[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLECENTER );
		m_aStars[ i ].SetTextureSection( pStarTexture );
		AGUI2::GetRootElement()->AddChildTail( m_aStars[ i ] );
		m_aStars[ i ].SetTranslation( m_aStarPositions[ i ] );
	}

	m_apLabels[ 0 ] = g_pLocaleManager->GetString( 700 );
	m_apLabels[ 1 ] = g_pLocaleManager->GetString( 701 );
	m_apLabels[ 2 ] = g_pLocaleManager->GetString( 702 );
	m_apLabels[ 3 ] = g_pLocaleManager->GetString( 703 );
	m_apLabels[ 4 ] = g_pLocaleManager->GetString( 326 );

	// The label under the stars
	const TWCHAR* wszLabel = m_apLabels[ 0 ];
	T2GUIFont*    pFont    = T2GUIFontManager::FindFont( "Rekord26" );
	m_oLabel.Create( pFont, pFont->GetTextWidth( wszLabel, 1.0f ) );
	m_oLabel.SetText( wszLabel );
	m_oLabel.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	m_oLabelRoot.SetDimensions( 10.0f, 10.0f );
	m_oLabelRoot.SetAlpha( 0.0f );
	m_oLabelRoot.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLECENTER );
	m_oLabelRoot.AddChildTail( m_oLabel );
	AGUI2::GetRootElement()->AddChildTail( m_oLabelRoot );
	m_oLabelRoot.SetTranslation( m_vPosition.x, ( pFont->GetLineHeight() + fStarSize ) * 0.5f + m_vPosition.y );

	SetVisible( TFALSE );
	return TTRUE;
}

// $Barnyard: FUNCTION 00637840
TBOOL AGUI2StarRating::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bVisible )
		return TFALSE;

	m_bMoving = TFALSE;

	// The stars of the rating fly to their positions one by one
	for ( TINT i = 0; i < m_iNumStars; i++ )
	{
		if ( m_aStarPositions[ i ] == m_aStarTargets[ i ] )
			continue;

		m_bMoving = TTRUE;

		TVector2 vDir   = m_aStarTargets[ i ] - m_aStarPositions[ i ];
		TFLOAT   fSpeed = a_fDeltaTime * 1500.0f;

		if ( vDir.NormalizeMag() >= fSpeed )
		{
			m_aStarPositions[ i ].x = vDir.x * fSpeed + m_aStarPositions[ i ].x;
			m_aStarPositions[ i ].y = vDir.y * fSpeed + m_aStarPositions[ i ].y;
		}
		else
		{
			m_aStarPositions[ i ] = m_aStarTargets[ i ];
		}

		m_aStars[ i ].SetTranslation( m_aStarPositions[ i ] );
		break;
	}

	// The others fly back off screen
	for ( TINT i = m_iNumStars; i < NUM_STARS; i++ )
	{
		if ( m_aStarPositions[ i ] == m_vStarStart )
			continue;

		TVector2 vDir   = m_vStarStart - m_aStarPositions[ i ];
		TFLOAT   fSpeed = a_fDeltaTime * 1600.0f;

		if ( vDir.NormalizeMag() >= fSpeed )
		{
			m_aStarPositions[ i ].x = vDir.x * fSpeed + m_aStarPositions[ i ].x;
			m_aStarPositions[ i ].y = vDir.y * fSpeed + m_aStarPositions[ i ].y;
		}
		else
		{
			m_aStarPositions[ i ] = m_vStarStart;
		}

		m_aStars[ i ].SetTranslation( m_aStarPositions[ i ] );
	}

	m_oPlank.SetVisible( m_bMoving || m_iNumStars > 0 );
	m_oLabel.SetVisible( !m_bMoving && m_iNumStars > 0 );

	return m_bMoving;
}

// $Barnyard: FUNCTION 00637460
void AGUI2StarRating::SetVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;

	for ( TINT i = NUM_STARS - 1; i >= 0; i-- )
		m_aStars[ i ].SetVisible( a_bVisible == TTRUE );

	// The plank is shown by Update
	m_oPlank.Hide();
	m_oLabel.SetVisible( m_iNumStars != 0 && m_bVisible == TTRUE );
	m_oLabelRoot.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00637500
TBOOL AGUI2StarRating::SetNumStars( TINT a_iNumStars )
{
	if ( m_iNumStars == a_iNumStars )
		return TFALSE;

	m_iNumStars = a_iNumStars;

	const TWCHAR* wszLabel = m_apLabels[ 0 ];

	switch ( a_iNumStars )
	{
		case 2: wszLabel = m_apLabels[ 1 ]; break;
		case 3: wszLabel = m_apLabels[ 2 ]; break;
		case 4: wszLabel = m_apLabels[ 3 ]; break;
		case 5: wszLabel = m_apLabels[ 4 ]; break;
	}

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oLabel.Create( pFont, pFont->GetTextWidth( wszLabel, 1.0f ) );
	m_oLabel.SetText( wszLabel );
	m_oLabel.SetVisible( m_iNumStars != 0 && m_bVisible == TTRUE );

	return TTRUE;
}

// $Barnyard: FUNCTION 006375d0
TBOOL AGUI2StarRating::SetDistance( TFLOAT a_fDistance, TFLOAT& a_rfRecord )
{
	if ( a_fDistance > a_rfRecord )
	{
		a_rfRecord = a_fDistance;
		SetNumStars( 5 );
		return TTRUE;
	}

	// NOTE: SetNumStars is inlined in the original, it returns TFALSE either way
	if ( a_fDistance > 45.0f ) SetNumStars( 4 );
	else if ( a_fDistance > 40.0f ) SetNumStars( 3 );
	else if ( a_fDistance > 35.0f ) SetNumStars( 2 );
	else if ( a_fDistance > 30.0f ) SetNumStars( 1 );
	else SetNumStars( 0 );

	return TFALSE;
}
