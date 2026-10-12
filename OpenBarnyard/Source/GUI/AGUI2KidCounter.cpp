#include "pch.h"
#include "AGUI2KidCounter.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Pop curves: they overshoot 1.0 and come back to 1.0 at the end
static TFLOAT s_fStateAngle = TMath::PI * 0.75f;                  // flt_7b5038
static TFLOAT s_fStateScale = 1.0f / TMath::Sin( s_fStateAngle ); // flt_7b5030
static TFLOAT s_fSizeAngle  = TMath::PI * 0.6f;                   // flt_7b503c
static TFLOAT s_fSizeScale  = 1.0f / TMath::Sin( s_fSizeAngle );  // flt_7b5034

static constexpr TFLOAT ICON_SIZE    = 32.0f;  // flt_7797ec
static constexpr TFLOAT GRID_WIDTH   = 256.0f; // flt_7797f0
static constexpr TFLOAT GRID_PADDING = 8.0f;   // flt_7797f4

// $Barnyard: FUNCTION 00635c00
AGUI2KidCounter::Icon::Icon()
{
}

// $Barnyard: FUNCTION 00635c50
AGUI2KidCounter::Icon::~Icon()
{
}

// $Barnyard: FUNCTION 00635d40
void AGUI2KidCounter::Icon::SetState( TFLOAT a_fState )
{
	if ( a_fState != fState )
	{
		fPrevState = oOn.GetWidth() / fSize;
		fStateTime = 0.3f;
		fState     = a_fState;
		oOn.Show();
	}
}

// $Barnyard: FUNCTION 00635d90
void AGUI2KidCounter::Icon::UpdatePosition( MODE a_eMode, TINT a_iNumIcons )
{
	TFLOAT fX;
	TFLOAT fY;

	if ( a_eMode == MODE_SEEKING )
	{
		fX = 0.0f;
		fY = oOff.GetWidth() * -0.1f;
	}
	else
	{
		TFLOAT fSpacing = oOff.GetWidth() * 0.7f;

		fY = fRowY;
		fX = ( TFLOAT( iIndex - 15 ) - 2.0f ) * fSpacing - ( fSize - 32.0f ) * 0.0f;

		if ( a_iNumIcons > 10 )
			fY = oOff.GetWidth() * TFLOAT( a_iNumIcons - 10 ) * 0.1f + fY;

		if ( oOff.GetWidth() > 32.0f )
			fY = fY - ( oOff.GetWidth() - 32.0f );

		if ( iIndex < 15 )
		{
			for ( TUINT i = ( 14 - iIndex ) / 5u + 1; i != 0; i-- )
				fX = fX + fSpacing * 5.0f;
		}
	}

	oOff.SetTransform( fX, fY, 0.0f );
	oOn.SetTransform( fX, fY, 0.0f );
}

// $Barnyard: FUNCTION 00635ed0
void AGUI2KidCounter::Icon::Update( TFLOAT a_fDeltaTime, MODE a_eMode, TINT a_iNumIcons )
{
	// Pop the "off" image
	TFLOAT fOldSizeTime = fSizeTime;
	fSizeTime -= a_fDeltaTime;

	if ( fSizeTime <= 0.0f )
	{
		if ( fOldSizeTime > 0.0f )
		{
			oOff.SetDimensions( fSize, fSize );
			UpdatePosition( a_eMode, a_iNumIcons );
			oOff.SetVisible( fSize > 0.1f );
			oOn.SetVisible( fSize > 0.1f );
		}
	}
	else
	{
		TFLOAT fProgress = ( 0.2f - fSizeTime ) * 5.0f;
		TFLOAT fT;

		if ( fPrevSize <= fSize )
			fT = TMath::Sin( fProgress * s_fSizeAngle ) * s_fSizeScale;
		else
			fT = TMath::Sin( fProgress * TMath::PI * 0.5f );

		if ( fT < 0.0f ) fT = 0.0f;

		TFLOAT fCurrentSize = TMath::LERP( fPrevSize, fSize, fT );
		oOff.SetDimensions( fCurrentSize, fCurrentSize );
		UpdatePosition( a_eMode, a_iNumIcons );
	}

	// Pop the "on" image
	TFLOAT fOldStateTime = fStateTime;
	fStateTime -= a_fDeltaTime;

	if ( fStateTime <= 0.0f )
	{
		if ( fOldStateTime > 0.0f )
		{
			TFLOAT fOnSize = fSize * fState;
			oOn.SetDimensions( fOnSize, fOnSize );
			oOn.SetVisible( fState > 0.1f );
		}
	}
	else
	{
		TFLOAT fProgress = ( 0.3f - fStateTime ) * 3.3333333f;
		if ( fProgress > 1.0f ) fProgress = 1.0f;

		TFLOAT fT;

		if ( fPrevState <= fState )
			fT = TMath::Sin( fProgress * s_fStateAngle ) * s_fStateScale;
		else
			fT = TMath::Sin( fProgress * TMath::PI * 0.5f );

		if ( fT < 0.0f ) fT = 0.0f;

		TFLOAT fOnSize = TMath::LERP( fPrevState, fState, fT ) * fSize;
		oOn.SetDimensions( fOnSize, fOnSize );
	}
}

// $Barnyard: FUNCTION 00636150
AGUI2KidCounter::AGUI2KidCounter()
{
	m_bCreated       = TFALSE;
	m_fCountTime     = 0.0f;
	m_fLastIconState = 1.0f;
	m_iTargetCount   = 0;
	m_iCount         = 0;
	m_eMode          = MODE_0;
	m_iNumIcons      = 10;
}

// $Barnyard: FUNCTION 006361f0
AGUI2KidCounter::~AGUI2KidCounter()
{
	if ( m_bCreated )
	{
		m_bCreated = TFALSE;
		RemoveSelf();
	}
}

// $Barnyard: FUNCTION 00635ca0
void AGUI2KidCounter::SetTargetCount( TINT a_iCount )
{
	if ( m_iTargetCount != a_iCount )
	{
		m_iTargetCount   = a_iCount;
		m_fLastIconState = 1.0f;
	}
}

// $Barnyard: FUNCTION 00635cc0
void AGUI2KidCounter::SetText( const TWCHAR* a_wszText )
{
	TINT iLength = T2String16::Length( a_wszText );

	if ( iLength > 12 )
		m_oText.SetScale( 0.4f );
	else if ( iLength > 9 )
		m_oText.SetScale( 0.6f );
	else
		m_oText.SetScale( 0.7f );

	m_oText.SetTextAndFitWidth( a_wszText );
}

// $Barnyard: FUNCTION 00636280
void AGUI2KidCounter::Create( T2GUIElement* a_pParent )
{
	if ( m_bCreated )
		return;

	m_bCreated = TTRUE;

	SetDimensions( GRID_WIDTH * 0.5f + GRID_PADDING, ICON_SIZE + ICON_SIZE + GRID_PADDING );
	SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	SetTransform( a_pParent->GetWidth() * -0.015625f, 0.0f, 0.0f );
	a_pParent->AddChildTail( *this );

	m_pTexOn                     = T2GUITextureSectionManager::GetTextureSection( "Tommy_on" );
	m_pTexSeeking                = T2GUITextureSectionManager::GetTextureSection( "Tommy_Kid_Seeking" );
	T2GUITextureSection* pTexOff = T2GUITextureSectionManager::GetTextureSection( "Tommy_off" );
	m_fIconSize                  = 32.0f;

	// Create the grid of icons, the last icon is at the top left
	TINT iIndex = MAX_NUM_ICONS;

	for ( TINT iRow = 0; iRow < 4; iRow++ )
	{
		TFLOAT fRowY = ( 0.25f - iRow ) * ICON_SIZE;

		for ( TINT iColumn = 4; iColumn >= 0; iColumn-- )
		{
			Icon& rIcon = m_aIcons[ --iIndex ];

			TFLOAT fX = ( TFLOAT( iColumn - 15 ) + 0.25f ) * 22.4f;

			rIcon.oOff.T2GUIElement::SetDimensions( 32.0f, 32.0f );
			rIcon.oOn.T2GUIElement::SetDimensions( 0.0f, 0.0f );
			rIcon.oOff.SetTextureSection( pTexOff );
			rIcon.oOn.SetTextureSection( m_pTexOn );
			rIcon.oOff.SetTransform( fX, fRowY, 0.0f );
			rIcon.oOn.SetTransform( fX, fRowY, 0.0f );
			AddChildTail( rIcon.oOff );
			AddChildTail( rIcon.oOn );
			rIcon.oOff.Hide();
			rIcon.oOn.Hide();

			rIcon.fStateTime = 0.0f;
			rIcon.fPrevState = 0.0f;
			rIcon.fState     = 0.0f;
			rIcon.fSizeTime  = 0.0f;
			rIcon.fPrevSize  = 0.0f;
			rIcon.fSize      = 0.0f;

			TFLOAT fSize = m_fIconSize;

			if ( fSize != 0.0f )
			{
				rIcon.fPrevSize = rIcon.oOn.GetWidth();
				rIcon.fSizeTime = 0.2f;
				rIcon.fSize     = fSize;
				rIcon.oOff.Show();
			}

			rIcon.iIndex = iIndex;
			rIcon.fRowY  = fRowY;
			rIcon.UpdatePosition( m_eMode, m_iNumIcons );
			rIcon.oOff.SetDimensions( 0.0f, 0.0f );
			rIcon.oOn.SetDimensions( 0.0f, 0.0f );
		}
	}

	// Create the text
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "rekord26" );
	m_oText.Create( pFont, pFont->GetTextWidth( L"", 1.0f ) );
	m_oText.SetText( L"" );
	m_oText.SetScale( 0.7f );
	m_oText.SetColour( 0xffffc800 );
	m_oText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AddChildTail( m_oText );
}

// $Barnyard: FUNCTION 00636540
void AGUI2KidCounter::SetCount( TINT a_iCount )
{
	if ( a_iCount > m_iNumIcons )
		a_iCount = m_iNumIcons;

	if ( m_eMode == MODE_SEEKING )
	{
		// Only the big icon in the middle is shown
		for ( TINT i = 0; i < 13; i++ )
			m_aIcons[ i ].SetState( 0.0f );

		m_aIcons[ 13 ].SetState( 1.0f );

		for ( TINT i = 14; i < MAX_NUM_ICONS; i++ )
			m_aIcons[ i ].SetState( 0.0f );

		m_iCount = 1;
	}
	else
	{
		// The used icons are the last m_iNumIcons ones
		TINT iFirst = ( m_iNumIcons <= 10 ) ? MAX_NUM_ICONS - m_iNumIcons : 0;
		TINT iEnd   = iFirst + a_iCount;
		TINT i      = 0;

		for ( ; i < iFirst; i++ )
			m_aIcons[ i ].SetState( 0.0f );

		for ( ; i < iEnd; i++ )
			m_aIcons[ i ].SetState( 1.0f );

		for ( ; i < MAX_NUM_ICONS; i++ )
			m_aIcons[ i ].SetState( 0.0f );

		m_iCount = a_iCount;
	}
}

// $Barnyard: FUNCTION 00636840
void AGUI2KidCounter::SetIconSize( TFLOAT a_fIconSize )
{
	m_fIconSize = a_fIconSize;

	for ( TINT i = 0; i < MAX_NUM_ICONS; i++ )
		m_aIcons[ i ].SetSize( a_fIconSize );
}

// $Barnyard: FUNCTION 006368b0
void AGUI2KidCounter::Update( TFLOAT a_fDeltaTime )
{
	if ( m_iCount != m_iTargetCount )
	{
		// Count 20 per second towards the target count
		TFLOAT fTime  = a_fDeltaTime + m_fCountTime;
		TINT   iSteps = TINT( fTime * 20.0f );
		m_fCountTime  = TFLOAT( fmod( fTime, 0.05f ) );

		if ( m_iCount >= m_iTargetCount )
		{
			m_iCount -= iSteps;
			if ( m_iCount < m_iTargetCount ) m_iCount = m_iTargetCount;
		}
		else
		{
			m_iCount += iSteps;
			if ( m_iCount > m_iTargetCount ) m_iCount = m_iTargetCount;
		}

		SetCount( m_iCount );
	}
	else if ( m_eMode == MODE_2 )
	{
		TINT iIndex = m_iNumIcons + m_iTargetCount - 21;

		if ( iIndex >= 0 )
			m_aIcons[ iIndex ].SetState( m_fLastIconState );
	}

	for ( TINT i = 0; i < MAX_NUM_ICONS; i++ )
		m_aIcons[ i ].Update( a_fDeltaTime, m_eMode, m_iNumIcons );

	if ( m_eMode == MODE_SEEKING )
	{
		m_aIcons[ 13 ].oOff.Hide();

		TFLOAT fHeight = m_aIcons[ 13 ].oOff.GetWidth() * 0.9f + GRID_PADDING;
		TFLOAT fWidth  = m_aIcons[ 13 ].oOff.GetWidth() * 2.5f;
		SetDimensions( fWidth, fHeight );
	}
	else
	{
		Toshi::T2GUIRectangle& rFirstIcon = m_aIcons[ MAX_NUM_ICONS - m_iNumIcons ].oOff;

		TFLOAT fHeight = rFirstIcon.GetHeight() * TFLOAT( m_iNumIcons / 5 ) + GRID_PADDING;
		TFLOAT fWidth  = rFirstIcon.GetWidth() * 4.5f + GRID_PADDING;
		SetDimensions( fWidth, fHeight );
	}
}

// $Barnyard: FUNCTION 00636a70
void AGUI2KidCounter::SetNumIcons( TINT a_iNumIcons )
{
	m_iNumIcons = a_iNumIcons;

	if ( m_iTargetCount > a_iNumIcons )
		m_iTargetCount = a_iNumIcons;

	if ( m_eMode == MODE_SEEKING )
	{
		// Only the big icon in the middle is shown
		for ( TINT i = 0; i < 13; i++ )
		{
			m_aIcons[ i ].SetState( 0.0f );
			m_aIcons[ i ].SetSize( 0.0f );
		}

		m_aIcons[ 13 ].SetState( 0.0f );
		m_aIcons[ 13 ].SetState( 1.0f );
		m_aIcons[ 13 ].SetSize( 0.0f );
		m_aIcons[ 13 ].SetSize( m_fIconSize );

		for ( TINT i = 14; i < MAX_NUM_ICONS; i++ )
		{
			m_aIcons[ i ].SetState( 0.0f );
			m_aIcons[ i ].SetSize( 0.0f );
		}
	}
	else
	{
		// The used icons are the last a_iNumIcons ones
		TINT iFirst = ( a_iNumIcons <= 10 ) ? MAX_NUM_ICONS - a_iNumIcons : 0;
		TINT iEnd   = iFirst + a_iNumIcons;
		TINT i      = 0;

		for ( ; i < iFirst; i++ )
		{
			m_aIcons[ i ].SetState( 0.0f );
			m_aIcons[ i ].SetSize( 0.0f );
		}

		for ( ; i < iEnd; i++ )
		{
			m_aIcons[ i ].SetSize( m_fIconSize );
			m_aIcons[ i ].SetState( 1.0f );
		}

		for ( ; i < MAX_NUM_ICONS; i++ )
		{
			m_aIcons[ i ].SetState( 0.0f );
			m_aIcons[ i ].SetSize( 0.0f );
		}

		SetCount( 0 );
	}
}

// $Barnyard: FUNCTION 00636f60
void AGUI2KidCounter::SetMode( TINT a_eMode, TINT a_iNumIcons )
{
	m_aIcons[ 13 ].oOn.SetTextureSection( m_pTexOn );

	switch ( a_eMode )
	{
		case MODE_0:
			SetIconSize( 32.0f );
			break;
		case MODE_1:
			SetIconSize( 50.0f );
			break;
		case MODE_2:
			SetIconSize( 31.99f );
			break;
		case MODE_SEEKING:
			SetIconSize( 100.0f );
			a_iNumIcons = 1;
			m_aIcons[ 13 ].oOn.SetTextureSection( m_pTexSeeking );
			break;
	}

	m_eMode = MODE( a_eMode );
	SetNumIcons( a_iNumIcons );
}

// $Barnyard: FUNCTION 00637000
void AGUI2KidCounter::SetModeInstantly( TINT a_eMode, TINT a_iNumIcons )
{
	SetMode( a_eMode, a_iNumIcons );
	Update( 1000.0f );
}
