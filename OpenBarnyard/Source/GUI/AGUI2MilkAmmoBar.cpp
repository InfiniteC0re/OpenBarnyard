#include "pch.h"
#include "AGUI2MilkAmmoBar.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00642500
AGUI2MilkAmmoBar::AGUI2MilkAmmoBar()
    : m_iNumFull( 0 )
    , m_fMinAlpha( 1.0f )
    , m_bCreated( TFALSE )
    , m_bDisabled( TFALSE )
{
}

// $Barnyard: FUNCTION 00642580
// $Barnyard: FUNCTION 006428b0
AGUI2MilkAmmoBar::~AGUI2MilkAmmoBar()
{
	for ( TINT i = 0; i < NUM_BOTTLES; i++ )
		m_aBottles[ i ].RemoveSelf();
}

// $Barnyard: FUNCTION 00642610
void AGUI2MilkAmmoBar::Create( TFLOAT a_fHeight, TFLOAT a_fOffsetY, TFLOAT a_fMinAlpha )
{
	m_fMinAlpha = a_fMinAlpha;

	T2GUITextureSection* pBottle = T2GUITextureSectionManager::GetTextureSection( "Joy_Ride_Bottle" );
	TFLOAT               fWidth  = pBottle->GetWidth() / pBottle->GetHeight() * a_fHeight;
	TFLOAT               fX      = 0.0f;

	for ( TINT i = 0; i < NUM_BOTTLES; i++ )
	{
		T2GUIRectangle& rBottle = m_aBottles[ i ];
		rBottle.SetDimensions( fWidth, a_fHeight );
		rBottle.SetTextureSection( pBottle );
		rBottle.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

		// Every other bottle is a bit lower
		if ( i & 1 )
			rBottle.SetTransform( fX, -a_fOffsetY );
		else
			rBottle.SetTransform( fX, a_fOffsetY );

		fX += fWidth;
		AddChildTail( rBottle );
	}

	SetDimensions( fX, a_fHeight + a_fOffsetY );
	m_bCreated = TTRUE;
}

// $Barnyard: FUNCTION 00642710
void AGUI2MilkAmmoBar::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bDisabled )
	{
		// Fade every bottle out
		for ( TINT i = 0; i < NUM_BOTTLES; i++ )
		{
			TFLOAT fAlpha = m_aBottles[ i ].GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha < m_fMinAlpha ) fAlpha = m_fMinAlpha;

			m_aBottles[ i ].SetAlpha( fAlpha );
		}
	}
	else
	{
		// Fade the full bottles in...
		for ( TINT i = 0; i < m_iNumFull; i++ )
		{
			TFLOAT fAlpha = m_aBottles[ i ].GetAlpha() + ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha > 1.0f ) fAlpha = 1.0f;

			m_aBottles[ i ].SetAlpha( fAlpha );
		}

		// ...and the empty ones out
		for ( TINT i = m_iNumFull; i < NUM_BOTTLES; i++ )
		{
			TFLOAT fAlpha = m_aBottles[ i ].GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );
			if ( fAlpha < m_fMinAlpha ) fAlpha = m_fMinAlpha;

			m_aBottles[ i ].SetAlpha( fAlpha );
		}
	}
}

// $Barnyard: FUNCTION 00642860
void AGUI2MilkAmmoBar::SetAmmo( TINT a_iAmmo, TINT a_iMaxAmmo )
{
	if ( a_iMaxAmmo == NUM_BOTTLES )
		m_iNumFull = a_iAmmo;
	else
		m_iNumFull = TMath::FloorToInt( TFLOAT( a_iAmmo ) / TFLOAT( a_iMaxAmmo ) * NUM_BOTTLES );

	TMath::Clip( m_iNumFull, 0, NUM_BOTTLES );
}
