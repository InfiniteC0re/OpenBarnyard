#include "pch.h"
#include "AGolfReticle.h"
#include "GUI/AGUI2.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00470c40
AGolfReticle::AGolfReticle()
{
	m_bMarkerCreated = TFALSE;
}

// $Barnyard: FUNCTION 00470e40
AGolfReticle::~AGolfReticle()
{
	if ( m_bMarkerCreated )
		m_bMarkerCreated = TFALSE;
}

// $Barnyard: FUNCTION 00470ee0
void AGolfReticle::Create()
{
	m_bMarkerCreated = TTRUE;

	// The ring point, it points at the target when it's off the screen
	T2GUITextureSection* pRingPoint = T2GUITextureSectionManager::GetTextureSection( "Golf_Ring_Point" );
	m_oRingPoint.SetDimensions( 24.0f, 24.0f );
	m_oRingPoint.SetTextureSection( pRingPoint );
	m_oRingPoint.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oRingPoint );

	m_oRing.SetDimensions( 48.0f, 48.0f );
	m_oRing.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oRing.SetTransform( 0.0f, -6.0f, 0.0f );
	m_oRingPoint.AddChildTail( m_oRing );

	T2GUITextureSection* pRing = T2GUITextureSectionManager::GetTextureSection( "Golf_Ring" );

	for ( TINT i = 0; i < NUM_RING_PARTS; i++ )
	{
		m_aRingParts[ i ].SetDimensions( 24.0f, 24.0f );
		m_aRingParts[ i ].SetTextureSection( pRing );
		m_oRing.AddChildTail( m_aRingParts[ i ] );
	}

	m_aRingParts[ 0 ].SetTransform( -12.0f, -12.0f, 0.0f );
	m_aRingParts[ 1 ].SetTransform( 12.0f, -12.0f, TMath::HALF_PI );
	m_aRingParts[ 2 ].SetTransform( 12.0f, 12.0f, TMath::PI );
	m_aRingParts[ 3 ].SetTransform( -12.0f, 12.0f, TMath::HALF_PI * 3.0f );

	T2GUITextureSection* pBall = T2GUITextureSectionManager::GetTextureSection( "Putt_Ball" );
	m_oBall.SetDimensions( 24.0f, 24.0f );
	m_oBall.SetTextureSection( pBall );
	m_oBall.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oRing.AddChildTail( m_oBall );

	ATargetReticle::Create( TNULL );
	SetMode( MODE_GOLF );

	m_bVisible = TFALSE;
	m_bLocked  = TFALSE;
	m_oRingPoint.Hide();
	SetActive( TFALSE, TFALSE, 0.0f );
}

// $Barnyard: FUNCTION 00470cd0
void AGolfReticle::SetVisible( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;

	if ( !a_bVisible )
	{
		m_oRingPoint.Hide();
		SetActive( TFALSE, TFALSE, 0.0f );
	}
}

// $Barnyard: FUNCTION 00470d00
void AGolfReticle::SetLocked( TBOOL a_bLocked )
{
	m_bLocked = a_bLocked;

	if ( !a_bLocked )
		SetActive( TFALSE, TFALSE, 0.0f );
}

// $Barnyard: FUNCTION 00470d20
void AGolfReticle::SetMarkerColour( TUINT32 a_uiColour )
{
	m_oBall.SetColour( a_uiColour );
	m_oRingPoint.SetColour( a_uiColour );

	for ( TINT i = 0; i < NUM_RING_PARTS; i++ )
		m_aRingParts[ i ].SetColour( a_uiColour );

	SetColour( 150, ( a_uiColour >> 16 ) & 0xff, ( a_uiColour >> 8 ) & 0xff, a_uiColour & 0xff );
}

// $Barnyard: FUNCTION 00470d90
void AGolfReticle::SetTextures( const TCHAR* a_szBall, const TCHAR* a_szRingPoint )
{
	if ( a_szRingPoint )
	{
		T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szRingPoint );
		m_oRingPoint.SetDimensions( TMath::Abs( TFLOAT( pSection->GetPoint2X() - pSection->GetPoint1X() ) ), TMath::Abs( TFLOAT( pSection->GetPoint2Y() - pSection->GetPoint1Y() ) ) );
		m_oRingPoint.SetTextureSection( pSection );
	}

	T2GUITextureSection* pBall = T2GUITextureSectionManager::GetTextureSection( a_szBall );

	// The ball is as big as the ring point
	TFLOAT fHeight = m_oRingPoint.GetHeight();
	m_oBall.SetDimensions( m_oRingPoint.GetWidth(), fHeight );
	m_oBall.SetTextureSection( pBall );
}

// $Barnyard: FUNCTION 004710c0
TBOOL AGolfReticle::Update( const TVector4& a_rcWorldPos, TFLOAT a_fDeltaTime )
{
	if ( !m_bVisible )
		return TFALSE;

	TVector2 vScreenPos;

	if ( !ARenderer::GetSingleton()->ProjectToGUI( ACameraManager::GetSingleton()->GetCurrentCamera(), a_rcWorldPos, vScreenPos ) )
	{
		// Behind the camera, flip the direction
		TMatrix44 mInvCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		mInvCamera.InvertOrthogonal();

		TVector4 vCameraSpace;
		TMatrix44::TransformVector( vCameraSpace, mInvCamera, a_rcWorldPos );

		if ( vCameraSpace.z < 0.0f )
		{
			vScreenPos.x *= -1.0f;
			vScreenPos.y *= -1.0f;
		}
	}

	SetPosition( vScreenPos );

	// Position relative to the centre of the screen
	TFLOAT fWidth, fHeight;
	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );
	vScreenPos.x -= fWidth * 0.5f;
	vScreenPos.y -= fHeight * 0.5f;

	TFLOAT fAngle;

	if ( vScreenPos.y == 0.0f )
	{
		fAngle = TMath::Sign( vScreenPos.x ) * TMath::HALF_PI;
	}
	else
	{
		fAngle = -TMath::ATan( vScreenPos.x / vScreenPos.y );

		if ( vScreenPos.y > 0.0f )
			fAngle += TMath::PI;
	}

	AGUI2::GetRootElement()->GetDimensions( fWidth, fHeight );
	TFLOAT fHalfWidth  = fWidth * 0.5f;
	TFLOAT fHalfHeight = fHeight * 0.5f;

	if ( -fHalfWidth <= vScreenPos.x && vScreenPos.x <= fHalfWidth && -fHalfHeight <= vScreenPos.y && vScreenPos.y <= fHalfHeight )
	{
		// On the screen, show the reticle
		m_oRingPoint.Hide();

		if ( m_bLocked )
		{
			SetActive( TTRUE, TTRUE, 20.0f );
			ATargetReticle::Update( a_fDeltaTime );
		}
		else
		{
			SetActive( TFALSE, TFALSE, 0.0f );
		}

		return TFALSE;
	}

	// Off the screen, move the ring point to the edge of the screen and point it at the target
	TFLOAT fScaleY = TMath::Abs( vScreenPos.y ) / fHalfHeight;
	TFLOAT fScaleX = TMath::Abs( vScreenPos.x ) / fHalfWidth;
	TFLOAT fScale  = 1.0f / ( ( fScaleX < fScaleY ) ? fScaleY : fScaleX );

	vScreenPos.x *= fScale;
	vScreenPos.y *= fScale;

	fHalfWidth -= m_oRingPoint.GetWidth() * 0.5f;
	fHalfHeight -= m_oRingPoint.GetHeight() * 0.5f;
	TMath::Clip( vScreenPos.x, -fHalfWidth, fHalfWidth );
	TMath::Clip( vScreenPos.y, -fHalfHeight, fHalfHeight );

	m_oRingPoint.Show();
	m_oRingPoint.SetTransform( vScreenPos.x, vScreenPos.y, fAngle );
	m_oBall.SetTransform( 0.0f, 0.0f, -fAngle );
	SetActive( TFALSE, TFALSE, 0.0f );

	return TTRUE;
}
