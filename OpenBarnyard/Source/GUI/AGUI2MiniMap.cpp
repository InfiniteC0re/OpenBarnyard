#include "pch.h"
#include "AGUI2MiniMap.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"

#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Icon textures (in range), indexed by AGUI2MiniMap::ICON
static const TCHAR* s_aszIconSections[ AGUI2MiniMap::ICON_NUMOF ] = {
	"Apples", "Challenge", "Bike", "Gate", "Player", "Mission", "Item", "Enemy", "Dot_White", "White_Star", "Map_Honey", "Ping_Ring"
}; // 0x0077986c

// Icon textures (on the edge of the map), indexed by AGUI2MiniMap::ICON
static const TCHAR* s_aszEdgeIconSections[ AGUI2MiniMap::ICON_NUMOF ] = {
	"Arrow_Red", "Arrow_Yel", "Arrow_Yel", "Arrow_Grn", "Player", "Arrow_Yel", "Arrow_Grn", "Arrow_Red", "Arrow_White", "Arrow_White", "Arrow_Yel", "Arrow_White"
}; // 0x0077989c

// NOTE: retail calls ACamera::GetDirection here (inlined in ASimMiniMap), rotates (0, 0, 1) by the rotation of the camera's matrix
static TVector4 GetCameraForward( ACamera* a_pCamera )
{
	const TMatrix44& rcMatrix = a_pCamera->GetMatrix();
	return TVector4( rcMatrix.m_f31, rcMatrix.m_f32, rcMatrix.m_f33, 0.0f );
}

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// Returns the signed angle between a_rcDirection and +Z, positive when the cross product points down
static TFLOAT GetAngleToPosZ( const TVector4& a_rcDirection, TFLOAT& a_rCrossY )
{
	TVector4 vecCross;
	vecCross.CrossProduct( a_rcDirection, TVector4::VEC_POSZ );
	vecCross.Normalise();
	a_rCrossY = vecCross.y;

	return a_rcDirection.GetAngleBetweenVector( TVector4::VEC_POSZ );
}

// $Barnyard: FUNCTION 00643050
AGUI2MiniMap::AGUI2MiniMap()
    : m_pIcons( TNULL )
    , m_iMaxIcons( 0 )
{
	Reset();
}

// $Barnyard: FUNCTION 00644050
// $Barnyard: FUNCTION 00643ed0
AGUI2MiniMap::~AGUI2MiniMap()
{
	for ( TINT i = m_iNumIcons - 1; i >= 0; i-- )
		m_pIcons[ i ].RemoveSelf();

	delete[] m_pIcons;

	m_oPlayerIcon.RemoveSelf();
	m_oFrame.RemoveSelf();
	m_oMask.RemoveSelf();
	m_oMap.RemoveSelf();
	m_oContainer.RemoveSelf();
	RemoveSelf();

	m_iNumIcons = 0;
}

// $Barnyard: FUNCTION 006428d0
void AGUI2MiniMap::Reset()
{
	m_fZoom           = 1.0f;
	m_fWorldToMap     = 1.0f;
	m_fTextureWidth   = 10.0f;
	m_fTextureHeight  = 10.0f;
	m_fMapWidth       = 100.0f;
	m_fMapHeight      = 100.0f;
	m_iNumIcons       = 0;
	m_vCentreWorldPos = TVector4::VEC_ZERO;
	m_eRotation       = ROTATION_CAMERA;
	m_bRound          = TFALSE;
}

// $Barnyard: FUNCTION 00643120
void AGUI2MiniMap::Create( const TCHAR* a_szMapSection, TFLOAT a_fScaleX, TFLOAT a_fScaleY, const Viewport& a_rcViewport, const TVector4& a_rcWorldPos1, const TVector2& a_rcMapPos1, const TVector4& a_rcWorldPos2, const TVector2& a_rcMapPos2, TBOOL a_bRound, const TCHAR* a_szCornerSection, TBOOL a_bFrame, TINT a_iMaxIcons )
{
	Reset();

	m_bHasCorners = TFALSE;
	m_bRound      = a_bRound;
	m_fMapWidth   = TFLOAT( a_rcViewport.iWidth ) * a_fScaleX;
	m_fMapHeight  = TFLOAT( a_rcViewport.iHeight ) * a_fScaleY;

	// Round maps are square
	if ( a_bRound )
		m_fMapWidth = m_fMapHeight;

	T2GUIElement::SetDimensions( m_fMapWidth, m_fMapHeight );
	m_oContainer.T2GUIElement::SetDimensions( m_fMapWidth, m_fMapHeight );
	m_oMap.T2GUIElement::SetDimensions( m_fMapWidth, m_fMapHeight );
	m_oMask.T2GUIElement::SetDimensions( m_fMapWidth, m_fMapHeight );
	m_oFrame.T2GUIElement::SetDimensions( m_fMapWidth, m_fMapHeight );

	if ( m_bRound )
	{
		T2GUITextureSection* pMask = T2GUITextureSectionManager::GetTextureSection( "mapmask" );
		pMask->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );
		m_oMask.SetTextureSection( pMask );
	}

	T2GUITextureSection* pMap = T2GUITextureSectionManager::GetTextureSection( a_szMapSection );
	m_fTextureWidth           = TMath::Abs( TFLOAT( pMap->GetPoint2X() - pMap->GetPoint1X() ) );
	m_fTextureHeight          = TMath::Abs( TFLOAT( pMap->GetPoint2Y() - pMap->GetPoint1Y() ) );

	if ( m_bRound )
		pMap->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );

	m_oMap.SetTextureSection( pMap );

	TBOOL bHideFrame = TFALSE;

	if ( a_szCornerSection == TNULL )
	{
		T2GUITextureSection* pFrame = TNULL;

		if ( a_bFrame == TTRUE )
			pFrame = T2GUITextureSectionManager::GetTextureSection( m_bRound ? "bezel" : "Chase_Radar" );

		if ( pFrame )
		{
			m_oFrame.SetTextureSection( pFrame );
			m_oFrame.SetAlpha( 1.0f );
		}
		else
		{
			bHideFrame = TTRUE;
		}

		for ( TINT i = TARRAYSIZE( m_aCorners ) - 1; i >= 0; i-- )
			m_aCorners[ i ].SetAlpha( 0.0f );
	}
	else
	{
		// The frame is made of 4 quarters of the map
		m_bHasCorners = TTRUE;

		T2GUITextureSection* pCorner = T2GUITextureSectionManager::GetTextureSection( a_szCornerSection );
		TFLOAT               fSize   = m_fMapWidth * 0.5f;

		for ( TINT i = TARRAYSIZE( m_aCorners ) - 1; i >= 0; i-- )
		{
			m_aCorners[ i ].T2GUIElement::SetDimensions( fSize, fSize );
			m_aCorners[ i ].SetTextureSection( pCorner );
			m_aCorners[ i ].SetAlpha( 1.0f );
			m_oFrame.AddChildTail( m_aCorners[ i ] );
		}

		m_aCorners[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_BOTTOMRIGHT );
		m_aCorners[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_BOTTOMLEFT );
		m_aCorners[ 2 ].SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_TOPLEFT );
		m_aCorners[ 3 ].SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_TOPRIGHT );

		m_aCorners[ 1 ].FlipU();
		m_aCorners[ 2 ].FlipV();
		m_aCorners[ 2 ].FlipU();
		m_aCorners[ 3 ].FlipV();

		m_oFrame.SetAlpha( 0.0f );
	}

	m_oContainer.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oMap.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oMask.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oFrame.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	AddChildTail( m_oContainer );

	if ( !m_bRound )
	{
		m_oContainer.AddChildTail( m_oMask );
		m_oContainer.AddChildTail( m_oMap );
		m_oContainer.AddChildTail( m_oFrame );
	}
	else
	{
		if ( a_szCornerSection == TNULL )
			m_oContainer.AddChildTail( m_oFrame );

		m_oContainer.AddChildTail( m_oMask );
		m_oContainer.AddChildTail( m_oMap );

		if ( a_szCornerSection != TNULL )
			m_oContainer.AddChildTail( m_oFrame );
	}

	SetColour( 0x00FFFFFF );
	m_oContainer.SetColour( 0x00FFFFFF );
	m_oMap.SetColour( 0xC9FFFFFF );

	if ( !m_bHasCorners )
		m_oFrame.SetColour( 0xC9FFFFFF );

	if ( bHideFrame == TTRUE )
		m_oFrame.SetAlpha( 0.0f );

	if ( m_bRound )
		m_oMask.SetColour( 0xFFFFFFFF );
	else
		m_oMask.SetColour( 0x00FFFFFF );

	m_fWorldToMap  = TVector2::Distance( a_rcMapPos1, a_rcMapPos2 ) / TVector4::Distance( a_rcWorldPos1, a_rcWorldPos2 );
	m_vWorldOrigin = a_rcWorldPos1;
	m_vMapOrigin   = a_rcMapPos1;
	SetZoom( m_fZoom );

	// Create the icon list
	if ( m_pIcons )
	{
		if ( a_iMaxIcons == m_iMaxIcons )
			return;

		m_iMaxIcons = a_iMaxIcons;
		delete[] m_pIcons;
	}
	else
	{
		m_iMaxIcons = a_iMaxIcons;
	}

	m_pIcons = new Icon[ m_iMaxIcons ];
}

// $Barnyard: FUNCTION 00642ec0
void AGUI2MiniMap::SetZoom( TFLOAT a_fZoom )
{
	TMath::Clip( a_fZoom, 0.001f, 1.0f );

	TFLOAT fHalfZoom    = a_fZoom * 0.5f;
	m_fZoom             = a_fZoom;
	m_fHalfZoom         = fHalfZoom;
	m_fZoomDiagonal     = TMath::Sqrt( a_fZoom * a_fZoom + a_fZoom * a_fZoom );
	m_fHalfZoomDiagonal = TMath::Sqrt( fHalfZoom * fHalfZoom + fHalfZoom * fHalfZoom );

	TFLOAT fOneOverZoom = 1.0f / a_fZoom;
	m_fMapToScreenX     = fOneOverZoom * m_fMapWidth / m_fTextureWidth;
	m_fMapToScreenY     = fOneOverZoom * m_fMapHeight / m_fTextureHeight;
}

// $Barnyard: FUNCTION 00643600
void AGUI2MiniMap::SetCentre( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	m_vCentreWorldPos = a_rcPosition;

	TVector4 vOffset  = a_rcPosition - m_vWorldOrigin;
	m_vCentreMapPos.x = vOffset.x * m_fWorldToMap + m_vMapOrigin.x;
	m_vCentreMapPos.y = -vOffset.z * m_fWorldToMap + m_vMapOrigin.y;

	TVector2 vCentreUV( m_vCentreMapPos.x / m_fTextureWidth, m_vCentreMapPos.y / m_fTextureHeight );

	// Show the part of the texture around the centre
	TVector2 vCorner( -1.0f, -1.0f );
	vCorner.Normalize();
	TVector2 vUV1( vCorner.x * m_fHalfZoomDiagonal + vCentreUV.x, vCorner.y * m_fHalfZoomDiagonal + vCentreUV.y );

	vCorner = TVector2( 1.0f, -1.0f );
	vCorner.Normalize();
	TVector2 vUV2( vCorner.x * m_fHalfZoomDiagonal + vCentreUV.x, vCorner.y * m_fHalfZoomDiagonal + vCentreUV.y );

	vCorner = TVector2( 1.0f, 1.0f );
	vCorner.Normalize();
	TVector2 vUV3( vCorner.x * m_fHalfZoomDiagonal + vCentreUV.x, vCorner.y * m_fHalfZoomDiagonal + vCentreUV.y );

	vCorner = TVector2( -1.0f, 1.0f );
	vCorner.Normalize();
	TVector2 vUV4( vCorner.x * m_fHalfZoomDiagonal + vCentreUV.x, vCorner.y * m_fHalfZoomDiagonal + vCentreUV.y );

	m_oMap.SetUVs( vUV1, vUV2, vUV3, vUV4 );

	// Rotate the player marker
	m_oPlayerIcon.GetTransform().SetIdentity();

	TVector4 vDirection = a_rcDirection;
	NormaliseXZ( vDirection );

	TFLOAT fCrossY;
	TFLOAT fAngle = GetAngleToPosZ( vDirection, fCrossY );

	if ( fCrossY <= 0.0f )
		m_oPlayerIcon.GetTransform().Rotate( fAngle );
	else
		m_oPlayerIcon.GetTransform().Rotate( -fAngle );

	if ( m_eRotation == ROTATION_PLAYER )
	{
		m_oContainer.GetTransform().SetIdentity();

		if ( fCrossY <= 0.0f )
			m_oContainer.GetTransform().Rotate( -fAngle );
		else
			m_oContainer.GetTransform().Rotate( fAngle );
	}
}

// $Barnyard: FUNCTION 00642960
TINT AGUI2MiniMap::AddIcon( const TVector4& a_rcPosition, const TVector4& a_rcDirection, ICONTYPE a_eType, const TCHAR* a_szSection, const TCHAR* a_szEdgeSection, TFLOAT a_fScale )
{
	TINT iIndex = m_iNumIcons;

	if ( m_iNumIcons >= m_iMaxIcons )
		return -1;

	T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szSection );

	TFLOAT fSectionWidth  = TFLOAT( pSection->GetPoint2X() - pSection->GetPoint1X() );
	TFLOAT fSectionHeight = TFLOAT( pSection->GetPoint2Y() - pSection->GetPoint1Y() );
	TFLOAT fMapSize       = ( m_fMapHeight >= m_fMapWidth ) ? m_fMapWidth : m_fMapHeight;
	TFLOAT fSectionSize   = ( fSectionHeight <= fSectionWidth ) ? fSectionWidth : fSectionHeight;

	// Scale the section to fit the map
	TFLOAT fFitScale = fMapSize / fSectionSize;
	TFLOAT fHeight   = fFitScale * fSectionHeight * a_fScale;
	TFLOAT fWidth    = fSectionWidth * fFitScale * a_fScale;

	if ( a_eType != ICONTYPE_PLAYER )
	{
		Icon& rIcon = m_pIcons[ m_iNumIcons ];

		rIcon.eType    = a_eType;
		rIcon.bEnabled = TTRUE;
		rIcon.T2GUIElement::SetDimensions( fWidth, fHeight );
		rIcon.SetTextureSection( pSection );
		rIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		rIcon.SetColour( 0xBBFFFFFF );

		rIcon.vDirection   = a_rcDirection;
		rIcon.vDirection.y = 0.0f;
		rIcon.vDirection.Normalise();

		rIcon.fScale         = 1.0f;
		rIcon.fSectionScale  = fFitScale * a_fScale;
		rIcon.bOnEdge        = TFALSE;
		rIcon.pSection       = pSection;
		rIcon.pEdgeSection   = T2GUITextureSectionManager::GetTextureSection( a_szEdgeSection );
		rIcon.bFixedRotation = TFALSE;
		rIcon.SetInFront();

		T2GUIRectangle* pParent = m_bHasCorners ? &m_oFrame : &m_oMap;
		pParent->AddChildTail( rIcon );

		// Keep the player marker on top of the icons
		if ( 0.0f != m_oPlayerIcon.GetWidth() )
			pParent->AddChildTail( m_oPlayerIcon );

		TVector4 vOffset = a_rcPosition - m_vWorldOrigin;
		rIcon.vMapPos.x  = vOffset.x * m_fWorldToMap + m_vMapOrigin.x;
		rIcon.vMapPos.y  = -vOffset.z * m_fWorldToMap + m_vMapOrigin.y;

		rIcon.Show();
		m_iNumIcons++;
		return iIndex;
	}
	else
	{
		m_oPlayerIcon.T2GUIElement::SetDimensions( fWidth, fHeight );
		m_oPlayerIcon.SetTextureSection( pSection );
		m_oPlayerIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		m_oPlayerIcon.SetColour( 0xFFFFFFFF );
		m_oPlayerIcon.SetInFront();

		if ( m_bHasCorners )
			m_oFrame.AddChildTail( m_oPlayerIcon );
		else
			m_oMap.AddChildTail( m_oPlayerIcon );

		return iIndex;
	}
}

// $Barnyard: FUNCTION 00643950
TINT AGUI2MiniMap::AddIcon( const TVector4& a_rcPosition, const TVector4& a_rcDirection, ICONTYPE a_eType, ICON a_eIcon, TFLOAT a_fScale )
{
	return AddIcon( a_rcPosition, a_rcDirection, a_eType, s_aszIconSections[ a_eIcon ], s_aszEdgeIconSections[ a_eIcon ], a_fScale );
}

// $Barnyard: FUNCTION 00642d60
void AGUI2MiniMap::SetIconTransform( TINT a_iIcon, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
	{
		TVector4 vOffset = a_rcPosition - m_vWorldOrigin;

		Icon& rIcon        = m_pIcons[ a_iIcon ];
		rIcon.vDirection   = a_rcDirection;
		rIcon.vDirection.y = 0.0f;
		rIcon.vDirection.Normalise();

		rIcon.vMapPos.x = vOffset.x * m_fWorldToMap + m_vMapOrigin.x;
		rIcon.vMapPos.y = -vOffset.z * m_fWorldToMap + m_vMapOrigin.y;
	}
}

// $Barnyard: FUNCTION 00642e40
void AGUI2MiniMap::SetColours( TUINT32 a_uiMapColour, TUINT32 a_uiFrameColour, TUINT32 a_uiPlayerColour, TUINT32 a_uiIconColour )
{
	m_oMap.SetColour( a_uiMapColour );
	m_oFrame.SetColour( a_uiFrameColour );
	m_oPlayerIcon.SetColour( a_uiPlayerColour );

	for ( TINT i = m_iNumIcons - 1; i >= 0; i-- )
		m_pIcons[ i ].SetColour( a_uiIconColour );
}

// $Barnyard: FUNCTION 00642f40
void AGUI2MiniMap::SetIconVisible( TINT a_iIcon, TBOOL a_bVisible )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
	{
		m_pIcons[ a_iIcon ].bEnabled = a_bVisible;
		m_pIcons[ a_iIcon ].SetVisible( a_bVisible == TTRUE );
	}
}

// $Barnyard: FUNCTION 00642f90
void AGUI2MiniMap::SetIconRotates( TINT a_iIcon, TBOOL a_bRotates )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
		m_pIcons[ a_iIcon ].bFixedRotation = !a_bRotates;
}

// $Barnyard: FUNCTION 00642fc0
void AGUI2MiniMap::SetIconColour( TINT a_iIcon, TUINT32 a_uiColour )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
		m_pIcons[ a_iIcon ].SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 00642ff0
void AGUI2MiniMap::SetIconScale( TINT a_iIcon, TFLOAT a_fScale )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
		m_pIcons[ a_iIcon ].fScale = a_fScale;
}

// $Barnyard: FUNCTION 00643020
void AGUI2MiniMap::SetIconAlpha( TINT a_iIcon, TFLOAT a_fAlpha )
{
	if ( a_iIcon > -1 && a_iIcon < m_iNumIcons )
		m_pIcons[ a_iIcon ].SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 00643980
void AGUI2MiniMap::Tick( TFLOAT a_fDeltaTime )
{
	T2GUIElement::Tick( a_fDeltaTime );

	TFLOAT fMapAngle = 0.0f;

	if ( m_eRotation == ROTATION_CAMERA )
	{
		// Rotate the map with the camera
		TVector4 vecCameraDir = GetCameraForward( ACameraManager::GetSingleton()->GetCurrentCamera() );
		vecCameraDir.y        = 0.0f;
		vecCameraDir.Normalise();

		if ( vecCameraDir.IsEqual( TVector4::VEC_ZERO ) )
			vecCameraDir = TVector4::VEC_POSZ;

		TFLOAT fCrossY;
		TFLOAT fAngle = GetAngleToPosZ( vecCameraDir, fCrossY );

		m_oContainer.GetTransform().SetIdentity();
		fMapAngle = ( fCrossY <= 0.0f ) ? -fAngle : fAngle;
		m_oContainer.GetTransform().Rotate( fMapAngle );
	}

	for ( TINT i = m_iNumIcons - 1; i >= 0; i-- )
	{
		Icon& rIcon = m_pIcons[ i ];

		if ( !rIcon.bEnabled )
			continue;

		TVector2 vPos = rIcon.vMapPos - m_vCentreMapPos;
		vPos.x        = vPos.x * m_fMapToScreenX;
		vPos.y        = vPos.y * m_fMapToScreenY;

		if ( rIcon.eType == ICONTYPE_EDGE )
		{
			TFLOAT fHalfWidth = m_fMapWidth * 0.5f;

			if ( m_bRound )
			{
				if ( !vPos.ClampMagnitude( fHalfWidth ) )
				{
					if ( rIcon.bOnEdge )
					{
						// Back in range
						rIcon.SetDimensions(
						    TMath::Abs( TFLOAT( rIcon.pSection->GetPoint2X() - rIcon.pSection->GetPoint1X() ) ) * rIcon.fSectionScale,
						    TMath::Abs( TFLOAT( rIcon.pSection->GetPoint2Y() - rIcon.pSection->GetPoint1Y() ) ) * rIcon.fSectionScale
						);
						rIcon.SetTextureSection( rIcon.pSection );
						rIcon.bOnEdge = TFALSE;
					}
				}
				else if ( !rIcon.bOnEdge )
				{
					// Out of range, show the edge texture
					rIcon.SetDimensions(
					    TMath::Abs( TFLOAT( rIcon.pEdgeSection->GetPoint2X() - rIcon.pEdgeSection->GetPoint1X() ) ),
					    TMath::Abs( TFLOAT( rIcon.pEdgeSection->GetPoint2Y() - rIcon.pEdgeSection->GetPoint1Y() ) )
					);
					rIcon.SetTextureSection( rIcon.pEdgeSection );
					rIcon.bOnEdge = TTRUE;
				}
			}
			else
			{
				TMath::Clip( vPos.x, -fHalfWidth, fHalfWidth );

				TFLOAT fHalfHeight = m_fMapHeight * 0.5f;
				TMath::Clip( vPos.y, -fHalfHeight, fHalfHeight );
			}
		}
		else
		{
			rIcon.Show();
			rIcon.bOnEdge = TFALSE;

			TFLOAT fHalfWidth = m_fMapWidth * 0.5f;

			if ( m_bRound )
			{
				if ( fHalfWidth * fHalfWidth < vPos.y * vPos.y + vPos.x * vPos.x )
				{
					rIcon.Hide();
					rIcon.bOnEdge = TTRUE;
				}
			}
			else
			{
				TFLOAT fHalfHeight = m_fMapHeight * 0.5f;

				if ( -fHalfWidth > vPos.x || vPos.x > fHalfWidth || -fHalfHeight > vPos.y || vPos.y > fHalfHeight )
				{
					rIcon.Hide();
					rIcon.bOnEdge = TTRUE;
				}
			}
		}

		T2GUITransform& rTransform = rIcon.GetTransform();
		rTransform.SetIdentity();
		rTransform.Scale( rIcon.fScale, rIcon.fScale );
		rTransform.SetTranslation( vPos );

		TFLOAT fAngle;

		if ( rIcon.bOnEdge )
		{
			// Arrow pointing out of the map
			fAngle = TMath::ATan2( vPos.x, -vPos.y );
		}
		else if ( rIcon.bFixedRotation )
		{
			fAngle = -fMapAngle;
		}
		else
		{
			TFLOAT fCrossY;
			TFLOAT fDirAngle = GetAngleToPosZ( rIcon.vDirection, fCrossY );
			fAngle           = ( fCrossY <= 0.0f ) ? fDirAngle : -fDirAngle;
		}

		rTransform.Rotate( fAngle );
	}
}
