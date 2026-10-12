#include "pch.h"
#include "AGUI2Feathers.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// 0x007797fc, indexed by m_aTextureIndices
static const TCHAR* s_aWingTextures[] = {
	"Wing",
	"Wings_Hot1",
	"Wings_Hot2",
	"Wings_Hot3",
	"Wings_Hot4",
	"Wing_Burnt",
};

static constexpr TINT TEXTURE_WING  = 0;
static constexpr TINT TEXTURE_HOT1  = 1;
static constexpr TINT TEXTURE_BURNT = 5;

static constexpr TFLOAT WING_SPACING     = 0.5f; // flt_779814
static constexpr TFLOAT BURN_FRAME_TIME  = 0.1f; // flt_779818
static constexpr TFLOAT FLARE_START_SIZE = 0.1f;

// $Barnyard: FUNCTION 00639230
AGUI2Feathers::AGUI2Feathers()
    : m_iNumWings( 0 )
    , m_iNumFeathers( 0 )
    , m_iNumTextures( TARRAYSIZE( s_aWingTextures ) )
    , m_ppTextures( TNULL )
    , m_uiBurningMask( 0 )
{
}

// $Barnyard: FUNCTION 00639000
// $Barnyard: FUNCTION 00638f40
AGUI2Feathers::~AGUI2Feathers()
{
}

// $Barnyard: FUNCTION 006392d0
void AGUI2Feathers::Tick( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_iNumWings; i++ )
	{
		T2GUIRectangle& rFlare = m_aFlares[ i ];

		if ( rFlare.IsVisible() )
		{
			// Grow and spin the flare
			TFLOAT fStep = a_fDeltaTime + a_fDeltaTime;

			m_aFlareScales[ i ] += fStep;
			TMath::Clip( m_aFlareScales[ i ], 0.0f, 1.0f );

			T2GUITransform& rTransform   = rFlare.GetTransform();
			TVector2        vTranslation = rTransform.GetTranslation();
			rTransform.SetIdentity();
			rTransform.SetTranslation( vTranslation );

			T2GUITransform oScale;
			oScale.Set( TVector2( m_aFlareScales[ i ], 0.0f ), TVector2( 0.0f, m_aFlareScales[ i ] ), TVector2::VEC_ZERO );
			rTransform.PreMultiply( oScale );

			m_aFlareRotations[ i ] += fStep;
			rTransform.Rotate( m_aFlareRotations[ i ] );

			m_aTimers[ i ] += a_fDeltaTime;

			if ( m_aTimers[ i ] >= 1.0f )
				rFlare.Hide();
		}

		TUINT8 uiBit = TUINT8( 1 << i );

		if ( m_uiBurningMask & uiBit )
		{
			// Play the burning animation
			m_aTimers[ i ] += a_fDeltaTime;

			if ( m_aTimers[ i ] >= BURN_FRAME_TIME )
			{
				m_aTimers[ i ] = 0.0f;
				m_aTextureIndices[ i ]++;

				if ( m_aTextureIndices[ i ] >= TEXTURE_BURNT )
					m_aTextureIndices[ i ] = TEXTURE_BURNT;

				m_aWings[ i ].SetTextureSection( m_ppTextures[ m_aTextureIndices[ i ] ] );

				if ( m_aTextureIndices[ i ] == TEXTURE_BURNT )
					m_uiBurningMask &= ~uiBit;
			}
		}
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00639020
TBOOL AGUI2Feathers::Create( TINT a_iNumFeathers )
{
	m_uiBurningMask = 0;
	m_ppTextures    = new T2GUITextureSection*[ m_iNumTextures ];

	TFLOAT fMaxWidth  = 0.0f;
	TFLOAT fMaxHeight = 0.0f;

	for ( TINT i = 0; i < m_iNumTextures; i++ )
	{
		m_ppTextures[ i ] = T2GUITextureSectionManager::GetTextureSection( s_aWingTextures[ i ] );

		T2GUITextureSection* pSection = m_ppTextures[ i ];
		TFLOAT               fWidth   = TMath::Abs( TFLOAT( pSection->GetPoint2X() - pSection->GetPoint1X() ) );
		if ( fWidth > fMaxWidth ) fMaxWidth = fWidth;

		TFLOAT fHeight = TMath::Abs( TFLOAT( pSection->GetPoint2Y() - pSection->GetPoint1Y() ) );
		if ( fHeight > fMaxHeight ) fMaxHeight = fHeight;
	}

	T2GUITextureSection* pFlareSection = T2GUITextureSectionManager::GetTextureSection( "Flare" );

	for ( TINT i = 0; i < a_iNumFeathers; i++ )
	{
		T2GUIRectangle& rWing  = m_aWings[ i ];
		T2GUIRectangle& rFlare = m_aFlares[ i ];

		m_aTextureIndices[ i ] = TEXTURE_WING;
		rWing.SetTextureSection( *m_ppTextures[ TEXTURE_WING ] );
		rWing.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
		rWing.SetTransform( TFLOAT( i ) * WING_SPACING * fMaxWidth, 0.0f, 0.0f );
		AddChildTail( rWing );

		rFlare.SetDimensions( fMaxWidth, fMaxWidth );
		rFlare.SetTextureSection( pFlareSection );
		rFlare.Hide();
		rWing.AddChildTail( rFlare );
	}

	// NOTE: uses the number of textures (6) rather than the number of wings
	SetDimensions( TFLOAT( m_iNumTextures ) * WING_SPACING * fMaxWidth, fMaxHeight );

	T2GUITransform oScale;
	oScale.Set( TVector2( 0.5f, 0.0f ), TVector2( 0.0f, 0.5f ), TVector2::VEC_ZERO );
	m_oTransform.PreMultiply( oScale );

	m_iNumWings    = a_iNumFeathers;
	m_iNumFeathers = 0;

	return TTRUE;
}

// $Barnyard: FUNCTION 00638fc0
void AGUI2Feathers::Destroy()
{
	for ( TINT i = 0; i < m_iNumWings; i++ )
		m_aWings[ i ].RemoveSelf();

	delete[] m_ppTextures;
}

// $Barnyard: FUNCTION 00639490
void AGUI2Feathers::SetNumFeathers( TINT a_iNumFeathers )
{
	for ( TINT i = 0; i < m_iNumWings; i++ )
	{
		TUINT8 uiBit = TUINT8( 1 << i );

		if ( i < a_iNumFeathers )
		{
			m_aTextureIndices[ i ] = TEXTURE_WING;
			m_aWings[ i ].SetTextureSection( m_ppTextures[ TEXTURE_WING ] );

			if ( a_iNumFeathers > m_iNumFeathers && i >= m_iNumFeathers )
			{
				// The wing was regained, pop the flare in
				T2GUIRectangle& rFlare = m_aFlares[ i ];
				rFlare.Show();

				m_aFlareScales[ i ] = FLARE_START_SIZE;

				T2GUITransform& rTransform   = rFlare.GetTransform();
				TVector2        vTranslation = rTransform.GetTranslation();
				rTransform.SetIdentity();
				rTransform.SetTranslation( vTranslation );
				rTransform.Scale( m_aFlareScales[ i ], m_aFlareScales[ i ] );

				m_aTimers[ i ]         = 0.0f;
				m_aFlareRotations[ i ] = 0.0f;
				m_uiBurningMask &= ~uiBit;
			}
		}
		else
		{
			if ( a_iNumFeathers < m_iNumFeathers && i < m_iNumFeathers )
			{
				// The wing was just lost, start burning it
				m_aTextureIndices[ i ] = TEXTURE_HOT1;
				m_aTimers[ i ]         = 0.0f;
				m_uiBurningMask |= uiBit;
				m_aFlares[ i ].Hide();
			}
			else
			{
				m_aTextureIndices[ i ] = TEXTURE_BURNT;
				m_uiBurningMask &= ~uiBit;
			}

			m_aWings[ i ].SetTextureSection( m_ppTextures[ m_aTextureIndices[ i ] ] );
		}
	}

	m_iNumFeathers = a_iNumFeathers;
}
