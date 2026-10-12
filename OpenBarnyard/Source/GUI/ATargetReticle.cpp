#include "pch.h"
#include "ATargetReticle.h"
#include "GUI/AGUI2.h"

#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: inlined TMathInline::MoveTowards, moves a_rfValue towards a_fTarget by a_fStep without going past it
static void MoveTowards( TFLOAT& a_rfValue, TFLOAT a_fTarget, TFLOAT a_fStep )
{
	if ( a_rfValue < a_fTarget )
	{
		a_rfValue += a_fStep;

		if ( a_rfValue > a_fTarget )
			a_rfValue = a_fTarget;
	}
	else
	{
		a_rfValue -= a_fStep;

		if ( a_rfValue < a_fTarget )
			a_rfValue = a_fTarget;
	}
}

// $Barnyard: FUNCTION 006491c0
ATargetReticle::ATargetReticle()
    : m_fWidth( 10.0f )
    , m_fHeight( 10.0f )
    , m_iNumVisible( 6 )
    , m_iMode( MODE_NONE )
    , m_iTexture( 2 )
    , m_fAngle( 0.0f )
    , m_fScale( 1.0f )
    , m_fRadius( 0.0f )
    , m_fTargetRadius( 30.0f )
    , m_fSpinSpeed( TMath::HALF_PI )
    , m_fRadiusSpeed( 20.0f )
    , m_fColourSpeed( 1.5f )
    , m_fColourProgress( 0.0f )
    , m_uiColour( 0xFFFFFFFF )
    , m_bActive( TFALSE )
    , m_bCreated( TFALSE )
{
	for ( TINT i = 0; i < 4; i++ )
	{
		m_afColour[ i ]       = 255.0f;
		m_afTargetColour[ i ] = 255.0f;
	}
}

// $Barnyard: FUNCTION 00648e10
ATargetReticle::~ATargetReticle()
{
	for ( TINT i = NUM_RECTANGLES - 1; i >= 0; i-- )
	{
		m_aRectangles[ i ].RemoveSelf();
	}
}

// $Barnyard: FUNCTION 006498d0
void ATargetReticle::Create( Toshi::T2GUIElement* a_pParent )
{
	ms_apTextures[ 0 ] = T2GUITextureSectionManager::GetTextureSection( "Target_Arrow" );
	ms_apTextures[ 1 ] = T2GUITextureSectionManager::GetTextureSection( "Putt_Yel_Point" );
	ms_apTextures[ 2 ] = T2GUITextureSectionManager::GetTextureSection( "right_arrow" );
	ms_apTextures[ 3 ] = T2GUITextureSectionManager::GetTextureSection( "Target_Circle" );

	T2GUIElement* pParent = a_pParent ? a_pParent : AGUI2::GetRootElement();

	TFLOAT fHalfHeight = pParent->GetHeight() * 0.5f;
	m_vPosition.x      = pParent->GetWidth() * 0.5f;
	m_vPosition.y      = fHalfHeight;

	for ( TINT i = NUM_RECTANGLES - 1; i >= 0; i-- )
	{
		m_aRectangles[ i ].SetDimensions( m_fWidth, m_fHeight );
		m_aRectangles[ i ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_MIDDLECENTER );
		m_aRectangles[ i ].Hide();

		( a_pParent ? a_pParent : AGUI2::GetRootElement() )->AddChildTail( m_aRectangles[ i ] );
	}

	UpdateSize( 0.0f );
	SetMode( MODE_IDLE );

	m_bCreated = TTRUE;
}

// $Barnyard: FUNCTION 006490d0
void ATargetReticle::SetActive( TBOOL a_bActive, TBOOL a_bSetLockTime, TFLOAT a_fLockTime )
{
	m_bActive = a_bActive;

	if ( !a_bActive )
		m_fRadius = 0.0f;

	if ( a_bSetLockTime )
		m_fRadius = a_fLockTime;

	for ( TINT i = NUM_RECTANGLES - 1; i >= 0; i-- )
	{
		if ( i < m_iNumVisible && m_bActive == TTRUE )
			m_aRectangles[ i ].Show();
		else
			m_aRectangles[ i ].Hide();
	}
}

// $Barnyard: FUNCTION 00649060
void ATargetReticle::UpdateSize( TFLOAT a_fLockTime )
{
	TFLOAT fSize = ( 0.03f - ( 0.03f / 36.0f ) * a_fLockTime ) * m_fScale * 640.0f;

	for ( TINT i = NUM_RECTANGLES - 1; i >= 0; i-- )
	{
		m_fHeight = fSize;
		m_fWidth  = fSize;
		m_aRectangles[ i ].SetDimensions( fSize, fSize );
	}
}

// $Barnyard: FUNCTION 006492a0
void ATargetReticle::SetMode( MODE a_iMode )
{
	if ( m_iMode == a_iMode )
		return;

	m_iMode           = a_iMode;
	m_fColourProgress = 0.0f;
	m_fBaseAngle      = 0.0f;
	TMath::Clip( m_iMode, 0, 9 );

	// Sets the target colour
	auto SetTargetColour = [ this ]( TFLOAT a_fR, TFLOAT a_fG, TFLOAT a_fB, TFLOAT a_fA ) {
		m_afTargetColour[ 0 ] = a_fR;
		m_afTargetColour[ 1 ] = a_fG;
		m_afTargetColour[ 2 ] = a_fB;
		m_afTargetColour[ 3 ] = a_fA;
	};

	switch ( m_iMode )
	{
		case MODE_LOCKED:
			m_iTexture      = 0;
			m_fTargetRadius = 23.0f;
			m_iNumVisible   = 4;
			m_fRadiusSpeed  = 100.0f;
			m_fSpinSpeed    = TMath::PI * -4.0f;
			SetTargetColour( 255.0f, 50.0f, 0.0f, 190.0f );
			m_fScale              = 1.4f;
			m_bStopSpinWhenLocked = TTRUE;
			break;
		case MODE_RELEASING:
			m_iTexture      = 3;
			m_fTargetRadius = 34.0f;
			m_iNumVisible   = 4;
			m_fSpinSpeed    = TMath::PI;
			m_fRadiusSpeed  = AGUI2::GetRootElement()->GetWidth() * 0.15f;
			SetTargetColour( 255.0f, 255.0f, 255.0f, 50.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_TARGET:
			m_iTexture      = 0;
			m_fTargetRadius = 26.0f;
			m_iNumVisible   = 4;
			m_fRadiusSpeed  = 70.0f;
			m_fSpinSpeed    = -TMath::HALF_PI;
			SetTargetColour( 255.0f, 50.0f, 0.0f, 190.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_BLOCKED:
			m_iTexture      = 0;
			m_fTargetRadius = 23.0f;
			m_iNumVisible   = 4;
			m_fSpinSpeed    = TMath::HALF_PI;
			m_fRadiusSpeed  = 70.0f;
			SetTargetColour( 255.0f, 255.0f, 0.0f, 200.0f );
			m_fScale              = 1.4f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_IDLE:
			m_iTexture      = 0;
			m_fTargetRadius = 30.0f;
			m_iNumVisible   = 4;
			m_fSpinSpeed    = TMath::HALF_PI;
			m_fRadiusSpeed  = 30.0f;
			SetTargetColour( 255.0f, 255.0f, 255.0f, 150.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_TARGETS:
			m_iTexture      = 3;
			m_fTargetRadius = 24.0f;
			m_iNumVisible   = 4;
			m_fSpinSpeed    = TMath::HALF_PI;
			m_fRadiusSpeed  = AGUI2::GetRootElement()->GetWidth() * 1.5f;
			SetTargetColour( 175.0f, 255.0f, 175.0f, 126.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_CURRENT_TARGET:
			m_iTexture      = 3;
			m_fTargetRadius = 28.0f;
			m_iNumVisible   = 4;
			m_fSpinSpeed    = TMath::HALF_PI;
			m_fRadiusSpeed  = AGUI2::GetRootElement()->GetWidth() * 1.5f;
			SetTargetColour( 255.0f, 75.0f, 0.0f, 190.0f );
			m_fScale              = 1.5f;
			m_bStopSpinWhenLocked = TTRUE;
			m_fBaseAngle          = TMath::HALF_PI * 0.5f;
			break;
		case MODE_POWERUP_TARGET:
			m_iTexture      = 0;
			m_fTargetRadius = 24.0f;
			m_iNumVisible   = 4;
			m_fRadiusSpeed  = 70.0f;
			m_fSpinSpeed    = TMath::HALF_PI * -3.0f;
			SetTargetColour( 255.0f, 0.0f, 0.0f, 200.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_POWERUP_IDLE:
			m_iTexture      = 0;
			m_fTargetRadius = 30.0f;
			m_iNumVisible   = 4;
			m_fRadiusSpeed  = 30.0f;
			m_fSpinSpeed    = TMath::HALF_PI * 3.0f;
			SetTargetColour( 255.0f, 255.0f, 255.0f, 150.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
		case MODE_GOLF:
			m_iTexture      = 0;
			m_fTargetRadius = 20.0f;
			m_iNumVisible   = 4;
			m_fRadiusSpeed  = 0.0f;
			m_fSpinSpeed    = TMath::HALF_PI + TMath::HALF_PI;
			SetTargetColour( 255.0f, 255.0f, 255.0f, 150.0f );
			m_fScale              = 1.0f;
			m_bStopSpinWhenLocked = TFALSE;
			break;
	}

	UpdateSize( 0.0f );

	for ( TINT i = m_iNumVisible - 1; i >= 0; i-- )
		m_aRectangles[ i ].SetTextureSection( ms_apTextures[ m_iTexture ] );

	SetActive( m_bActive, TFALSE, 0.0f );
	UpdateRectangles();
}

// $Barnyard: FUNCTION 00649720
void ATargetReticle::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bActive )
		return;

	MoveTowards( m_fRadius, m_fTargetRadius, a_fDeltaTime * m_fRadiusSpeed );
	MoveTowards( m_fColourProgress, 1.0f, a_fDeltaTime * m_fColourSpeed );

	// NOTE: TVector4::Lerp4 (006c7790) on the colours
	for ( TINT i = 0; i < 4; i++ )
		m_afColour[ i ] = ( m_afTargetColour[ i ] - m_afColour[ i ] ) * m_fColourProgress + m_afColour[ i ];

	m_uiColour = ( ( ( ( TUINT32( m_afColour[ 3 ] ) << 8 ) | TUINT32( m_afColour[ 0 ] ) ) << 8 | TUINT32( m_afColour[ 1 ] ) ) << 8 ) | TUINT32( m_afColour[ 2 ] );

	// The angle between two arrows
	TFLOAT fStep = TMath::TWO_PI / TFLOAT( m_iNumVisible );

	if ( m_bStopSpinWhenLocked && m_fRadius == m_fTargetRadius )
	{
		// Finish the turn and stop
		if ( m_fAngle != 0.0f )
		{
			m_fAngle += a_fDeltaTime * m_fSpinSpeed;

			if ( m_fAngle > fStep )
				m_fAngle = 0.0f;

			if ( -fStep > m_fAngle )
				m_fAngle = 0.0f;
		}
	}
	else
	{
		m_fAngle += a_fDeltaTime * m_fSpinSpeed;

		if ( m_fAngle > fStep )
			m_fAngle -= fStep;

		if ( -fStep > m_fAngle )
			m_fAngle += fStep;
	}

	UpdateRectangles();
}

// $Barnyard: FUNCTION 00648e80
void ATargetReticle::UpdateRectangles()
{
	for ( TINT i = m_iNumVisible - 1; i >= 0; i-- )
	{
		TFLOAT fAngle = TMath::TWO_PI / TFLOAT( m_iNumVisible ) * TFLOAT( i ) + m_fBaseAngle + m_fAngle;

		// NOTE: inlined TVector2::Rotate, rotates ( 0, -1 ) by the angle
		TFLOAT fSin, fCos;
		TMath::SinCos( fAngle, fSin, fCos );
		TVector2 vDirection( -fSin, -fCos );

		m_aPositions[ i ].x = vDirection.x * m_fRadius + m_vPosition.x;
		m_aPositions[ i ].y = vDirection.y * m_fRadius + m_vPosition.y;

		m_aRectangles[ i ].SetTransform( m_aPositions[ i ].x, m_aPositions[ i ].y, -fAngle );
		m_aRectangles[ i ].SetColour( m_uiColour );
	}
}

// $Barnyard: FUNCTION 00649180
void ATargetReticle::SetColour( TINT a_iA, TINT a_iR, TINT a_iG, TINT a_iB )
{
	m_afTargetColour[ 0 ] = TFLOAT( a_iR );
	m_afTargetColour[ 1 ] = TFLOAT( a_iG );
	m_afTargetColour[ 2 ] = TFLOAT( a_iB );
	m_afTargetColour[ 3 ] = TFLOAT( a_iA );

	for ( TINT i = 0; i < 4; i++ )
		m_afColour[ i ] = m_afTargetColour[ i ];
}

// $Barnyard: FUNCTION 00648fb0
void ATargetReticle::SetPosition( const Toshi::TVector2& a_rcPosition )
{
	m_vPosition = a_rcPosition;

	if ( m_vPosition.x < 0.0f )
		m_vPosition.x = 0.0f;

	if ( m_vPosition.y < 0.0f )
		m_vPosition.y = 0.0f;
}

// $Barnyard: FUNCTION 00649000
void ATargetReticle::SetPosition( TFLOAT a_fX, TFLOAT a_fY )
{
	m_vPosition.x = a_fX;
	m_vPosition.y = a_fY;

	if ( m_vPosition.x < 0.0f )
		m_vPosition.x = 0.0f;

	if ( m_vPosition.y < 0.0f )
		m_vPosition.y = 0.0f;
}

// $Barnyard: FUNCTION 00648f90
void ATargetReticle::GetPosition( Toshi::TVector2& a_rOutPosition ) const
{
	a_rOutPosition = m_vPosition;
}

// $Barnyard: FUNCTION 00649140
TBOOL ATargetReticle::IsLocked() const
{
	return m_fRadius == m_fTargetRadius;
}
