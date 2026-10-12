#include "pch.h"
#include "ASimMiniMap.h"
#include "AHudManager.h"
#include "GUI/AGUI2.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Helpers/AGameLoader.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ASimMiniMap );

// Textures of the icons
static const TCHAR* s_aszIconSections[ ASimMiniMap::ICON_NUMOF ] = { "Challenge", "Mission", "Item", "Enemy", "Player" }; // 0x007736b4

// Textures of the icons that are out of the map's range
static const TCHAR* s_aszEdgeIconSections[ ASimMiniMap::ICON_NUMOF ] = { "Arrow_Yel", "Arrow_Yel", "Arrow_Grn", "Arrow_Red", "Player" }; // 0x007736c8

// Converts (x, -z) world coordinates to UVs of Map_EnvMain
static constexpr TFLOAT WORLD_TO_UV_X = 0.0011972656f; // 0x007736dc
static constexpr TFLOAT WORLD_TO_UV_Y = 0.0023945312f; // 0x007736e0
static constexpr TFLOAT UV_OFFSET_X   = 0.4977539f;    // 0x007736e4
static constexpr TFLOAT UV_OFFSET_Y   = 0.5475781f;    // 0x007736e8

// Half of the part of the texture the map shows (in V, U is half of it)
static constexpr TFLOAT UV_RANGE = 0.15f; // 0x007736ec

// NOTE: inlined ACamera::GetDirection, rotates (0, 0, 1) by the rotation of the camera's matrix
static TVector4 GetCameraForward( ACamera* a_pCamera )
{
	const TMatrix44& rcMatrix = a_pCamera->GetMatrix();
	return TVector4( rcMatrix.m_f31, rcMatrix.m_f32, rcMatrix.m_f33, 0.0f );
}

// Returns the angle of a_rcDirection around the Y axis, [0; 2PI]
static TFLOAT GetDirectionAngle( TFLOAT a_fDirX, TFLOAT a_fDirZ )
{
	TFLOAT fDot = TVector2::VEC_POSY.y * a_fDirZ + TVector2::VEC_POSY.x * a_fDirX;
	TMath::Clip( fDot, -1.0f, 1.0f );

	TFLOAT fAngle = TMath::ACos( fDot );

	if ( TVector2::VEC_POSY.y * a_fDirX - TVector2::VEC_POSY.x * a_fDirZ < 0.0f )
		fAngle = TMath::TWO_PI - fAngle;

	return fAngle;
}

// $Barnyard: FUNCTION 004576b0
ASimMiniMap::Icon::Icon()
    : m_pUnit( TNULL )
    , m_vPosition( TVector4::VEC_ZERO )
    , m_eType( ICON_MISSION )
    , m_bRotate( TTRUE )
    , m_bOffEdge( TFALSE )
{
}

// $Barnyard: FUNCTION 00457710
ASimMiniMap::Icon::~Icon()
{
	m_oRect.RemoveSelf();
}

// $Barnyard: FUNCTION 00457a40
ASimMiniMap::ASimMiniMap()
    : AHudElement( MASK_UNK2 | MASK_MINIMAP )
    , m_iUnk2C0( 1 )
    , m_fUnk30C( 50.0f )
    , m_fUnk310( 10.0f )
    , m_vPlayerPos( TVector2::VEC_ZERO )
    , m_vCentreUV( TVector2::VEC_ZERO )
    , m_vUnk348( TVector2::VEC_ZERO )
{
	m_aMapUVs[ 0 ] = TVector2( 0.0f, 0.0f );
	m_aMapUVs[ 1 ] = TVector2( 1.0f, 0.0f );
	m_aMapUVs[ 2 ] = TVector2( 1.0f, 1.0f );
	m_aMapUVs[ 3 ] = TVector2( 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 00457bd0
ASimMiniMap::ASimMiniMap( Mask a_eMask )
    : AHudElement( a_eMask )
    , m_iUnk2C0( 1 )
    , m_fUnk30C( 50.0f )
    , m_fUnk310( 10.0f )
    , m_vPlayerPos( TVector2::VEC_ZERO )
    , m_vCentreUV( TVector2::VEC_ZERO )
    , m_vUnk348( TVector2::VEC_ZERO )
{
	m_aMapUVs[ 0 ] = TVector2( 0.0f, 0.0f );
	m_aMapUVs[ 1 ] = TVector2( 1.0f, 0.0f );
	m_aMapUVs[ 2 ] = TVector2( 1.0f, 1.0f );
	m_aMapUVs[ 3 ] = TVector2( 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 00457d60
// $Barnyard: FUNCTION 00458950
ASimMiniMap::~ASimMiniMap()
{
}

// $Barnyard: FUNCTION 00457650
void ASimMiniMap::GetDimensions( TINT& a_rWidth, TINT& a_rHeight )
{
	a_rWidth  = TINT( m_fWidth );
	a_rHeight = TINT( m_fHeight );
}

// $Barnyard: FUNCTION 00457770
TBOOL ASimMiniMap::OnAttach()
{
	for ( TINT i = 0; i < ICON_NUMOF; i++ )
		m_apIconSections[ i ] = T2GUITextureSectionManager::GetTextureSection( s_aszIconSections[ i ] );

	AGUI2::GetRootElement()->GetDimensions( m_fScreenWidth, m_fScreenHeight );

	// The bezel around the map
	T2GUITextureSection* pBezel = T2GUITextureSectionManager::GetTextureSection( "bezel" );
	m_oBezel.SetDimensions( pBezel->GetWidth(), pBezel->GetHeight() );
	m_oBezel.SetTextureSection( pBezel );
	m_oRoot.AddChildTail( m_oBezel );

	// The mask that makes the map round
	T2GUITextureSection* pMask = T2GUITextureSectionManager::GetTextureSection( "mapmask" );
	pMask->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASK );
	m_fWidth  = pMask->GetWidth();
	m_fHeight = pMask->GetHeight();
	m_oMask.SetDimensions( m_fWidth, m_fHeight );
	m_oMask.SetTextureSection( pMask );
	m_oRoot.AddChildTail( m_oMask );

	// Place the map in the bottom left corner
	m_oRoot.SetDimensions( m_fWidth, m_fHeight );
	m_oRoot.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	T2GUIElement* pGUIRoot = AGUI2::GetRootElement();
	TFLOAT        fY       = pGUIRoot->GetHeight() * 0.5f - m_oRoot.GetHeight() * 0.5f;
	TFLOAT        fX       = m_oRoot.GetWidth() * 0.5f - pGUIRoot->GetWidth() * 0.5f;
	m_oRoot.SetTranslation( fX, fY );
	pGUIRoot->AddChildTail( m_oRoot );

	// The map itself
	T2GUITextureSection* pMap = T2GUITextureSectionManager::GetTextureSection( "Map_EnvMain" );
	pMap->GetMaterial()->SetBlendState( T2GUIMaterial::BLENDSTATE_MASKED );
	pMap->GetMaterial()->SetMipMapLODBias( -10.0f );
	m_oMap.SetDimensions( m_fWidth, m_fHeight );
	m_oMap.SetTextureSection( pMap );
	m_oMap.SetUVs( m_aMapUVs[ 0 ], m_aMapUVs[ 1 ], m_aMapUVs[ 2 ], m_aMapUVs[ 3 ] );
	m_oMap.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oRoot.AddChildTail( m_oMap );

	m_vCentreUV.x = UV_OFFSET_X;
	m_vCentreUV.y = UV_OFFSET_Y;

	// The icons
	m_oIconContainer.SetDimensions( m_fWidth, m_fHeight );
	m_oRoot.AddChildTail( m_oIconContainer );

	// The arrow of the player in the middle
	T2GUITextureSection* pPlayer = m_apIconSections[ ICON_PLAYER ];
	m_oPlayerArrow.SetDimensions( pPlayer->GetWidth(), pPlayer->GetHeight() );
	m_oPlayerArrow.SetTextureSection( pPlayer );
	m_oIconContainer.AddChildTail( m_oPlayerArrow );

	Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 004575f0
void ASimMiniMap::Show()
{
	m_oBezel.T2GUIElement::Show();
	m_oMap.T2GUIElement::Show();
	m_oMask.T2GUIElement::Show();
	m_oIconContainer.Show();
	m_iUnk2C0           = 1;
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00457620
void ASimMiniMap::Hide()
{
	m_oBezel.T2GUIElement::Hide();
	m_oMap.T2GUIElement::Hide();
	m_oMask.T2GUIElement::Hide();
	m_oIconContainer.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00457e60
void ASimMiniMap::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bCurrentlyVisible || !m_pPlayer )
		return;

	AUnit*          pPlayer    = m_pPlayer->GetUnit();
	const TVector4* pPlayerPos = pPlayer->GetSteer()->GetPosition();
	TVector2        vecPlayerPos( pPlayerPos->x, -pPlayerPos->z );
	const TVector4* pPlayerDir = pPlayer->GetSteer()->GetDirection();

	// Rotate the arrow of the player to the direction the player is facing
	TFLOAT          fPlayerAngle    = GetDirectionAngle( pPlayerDir->x, pPlayerDir->z );
	T2GUITransform& rArrowTransform = m_oPlayerArrow.GetTransform();
	TVector2        vecArrowPos     = rArrowTransform.GetTranslation();
	rArrowTransform.SetIdentity();
	rArrowTransform.Rotate( fPlayerAngle );
	rArrowTransform.SetTranslation( vecArrowPos );

	// Rotate the map with the camera
	T2GUITransform& rRootTransform = m_oRoot.GetTransform();
	TVector2        vecRootPos     = rRootTransform.GetTranslation();
	rRootTransform.SetIdentity();

	TVector4 vecCameraDir = GetCameraForward( ACameraManager::GetSingleton()->GetCurrentCamera() );
	vecCameraDir.y        = 0.0f;
	vecCameraDir.Normalise();

	if ( vecCameraDir.IsEqual( TVector4::VEC_ZERO ) )
		vecCameraDir = TVector4::VEC_POSZ;

	TVector4 vecCross;
	vecCross.CrossProduct( vecCameraDir, TVector4::VEC_POSZ );
	vecCross.Normalise();

	TFLOAT fCameraAngle = ( vecCross.y <= 0.0f ) ? -vecCameraDir.GetAngleBetweenVector( TVector4::VEC_POSZ ) : vecCameraDir.GetAngleBetweenVector( TVector4::VEC_POSZ );
	rRootTransform.Rotate( fCameraAngle );
	rRootTransform.SetTranslation( vecRootPos );

	if ( vecPlayerPos.x != m_vPlayerPos.x || vecPlayerPos.y != m_vPlayerPos.y )
	{
		// Show the part of the map around the player
		m_vPlayerPos  = vecPlayerPos;
		m_vCentreUV.x = WORLD_TO_UV_X * m_vPlayerPos.x + UV_OFFSET_X;
		m_vCentreUV.y = WORLD_TO_UV_Y * m_vPlayerPos.y + UV_OFFSET_Y;

		TFLOAT   fHalfRangeU = UV_RANGE * 0.5f;
		TVector2 vecUV1( m_vCentreUV.x - fHalfRangeU, m_vCentreUV.y - UV_RANGE );
		TVector2 vecUV2( fHalfRangeU + m_vCentreUV.x, vecUV1.y );
		TVector2 vecUV3( vecUV2.x, UV_RANGE + m_vCentreUV.y );
		TVector2 vecUV4( vecUV1.x, vecUV3.y );

		TMath::Clip( vecUV1.x, 0.0f, 1.0f );
		TMath::Clip( vecUV4.x, 0.0f, 1.0f );
		TMath::Clip( vecUV2.x, 0.0f, 1.0f );
		TMath::Clip( vecUV3.x, 0.0f, 1.0f );
		m_oMap.SetUVs( vecUV1, vecUV2, vecUV3, vecUV4 );
	}

	for ( auto it = m_llUsedIcons.Begin(); it != m_llUsedIcons.End(); it++ )
	{
		Icon*  pIcon = it;
		AUnit* pUnit = pIcon->m_pUnit;

		TFLOAT fX, fY;
		TFLOAT fDirX, fDirZ;

		if ( !pUnit || pUnit->IsDying() )
		{
			fX    = pIcon->m_vPosition.x;
			fDirX = 0.0f;
			fDirZ = -1.0f;
			fY    = -pIcon->m_vPosition.z;
		}
		else
		{
			const TVector4* pPos = pUnit->GetSteer()->GetPosition();
			fY                   = -pPos->z;
			fX                   = pPos->x;

			const TVector4* pDir = pUnit->GetSteer()->GetDirection();
			fDirX                = pDir->x;
			fDirZ                = pDir->z;
		}

		// Position of the icon relative to the player
		TFLOAT fU = fX * WORLD_TO_UV_X + UV_OFFSET_X;
		TFLOAT fV = fY * WORLD_TO_UV_Y + UV_OFFSET_Y;

		TFLOAT   fOneOverRange = 1.0f / UV_RANGE;
		TVector2 vecOffset( fOneOverRange * m_fWidth * ( fU - m_vCentreUV.x ), fOneOverRange * m_fWidth * 0.5f * ( fV - m_vCentreUV.y ) );
		TBOOL    bOffEdge = vecOffset.ClampMagnitude( m_fWidth * 0.45f );

		T2GUITransform& rTransform = pIcon->m_oRect.GetTransform();
		rTransform.SetIdentity();

		TFLOAT fAngle;

		if ( !bOffEdge )
		{
			if ( pIcon->m_bOffEdge )
			{
				pIcon->m_oRect.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( s_aszIconSections[ pIcon->m_eType ] ) );
				pIcon->m_bOffEdge = TFALSE;
			}

			// Keep the icon upright
			fAngle = -fCameraAngle;
		}
		else
		{
			if ( !pIcon->m_bOffEdge )
			{
				pIcon->m_oRect.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( s_aszEdgeIconSections[ pIcon->m_eType ] ) );
				pIcon->m_bOffEdge = TTRUE;
			}

			// Point the arrow at the icon
			fAngle = atan2( vecOffset.x, -vecOffset.y );
		}

		rTransform.Rotate( fAngle );
		rTransform.SetTranslation( vecOffset );

		if ( pIcon->m_bRotate && !pIcon->m_bOffEdge )
			rTransform.Rotate( GetDirectionAngle( fDirX, fDirZ ) );
	}
}

// $Barnyard: FUNCTION 004587e0
TBOOL ASimMiniMap::Create()
{
	AHudManager::GetSingleton()->AddElement( this, TFALSE, TTRUE, TFALSE );

	for ( TINT i = 0; i < MAX_ICONS; i++ )
		m_llFreeIcons.InsertTail( &m_aIcons[ i ] );

	return TTRUE;
}

// $Barnyard: FUNCTION 00458820
void ASimMiniMap::Destroy()
{
	m_oBezel.RemoveSelf();
	m_oMap.RemoveSelf();
	m_oMask.RemoveSelf();
	m_oIconContainer.RemoveSelf();
	m_oPlayerArrow.RemoveSelf();

	while ( !m_llUsedIcons.IsEmpty() )
	{
		Icon* pIcon = m_llUsedIcons.Tail();
		pIcon->Remove();
		pIcon->m_oRect.RemoveSelf();
	}

	m_llUsedIcons.TGenericDList::RemoveAll();
	m_llFreeIcons.TGenericDList::RemoveAll();

	Detach();
}

// $Barnyard: FUNCTION 00458450
TINT ASimMiniMap::AddIcon( const TVector4& a_rcPosition, ICON a_eType, TFLOAT a_fUnused, TBOOL a_bRotate )
{
	Icon* pIcon = m_llFreeIcons.Tail();
	pIcon->Remove();

	pIcon->m_bOffEdge  = TFALSE;
	pIcon->m_vPosition = a_rcPosition;
	pIcon->m_pUnit     = TNULL;
	pIcon->m_eType     = a_eType;
	pIcon->m_bRotate   = a_bRotate;
	pIcon->m_oRect.SetTextureSection( *m_apIconSections[ a_eType ] );
	pIcon->m_oRect.GetTransform().SetIdentity();
	pIcon->m_oRect.SetInFront();
	m_oIconContainer.AddChildTail( pIcon->m_oRect );
	m_llUsedIcons.InsertTail( pIcon );

	for ( TINT i = 0; &m_aIcons[ i ] != pIcon; )
	{
		if ( ++i >= MAX_ICONS )
			return -1;
	}

	return TINT( pIcon - m_aIcons );
}

// $Barnyard: FUNCTION 00458510
TINT ASimMiniMap::AddUnitIcon( TRef<AUnitProxy> a_pUnitProxy, ICON a_eType, TFLOAT a_fUnused, TBOOL a_bRotate )
{
	// No minimap in the barn
	if ( AGameLoader::IsBarnExploreMiniGame() )
		return -1;

	Icon* pIcon = m_llFreeIcons.Tail();
	pIcon->Remove();

	pIcon->m_pUnit     = a_pUnitProxy ? a_pUnitProxy->GetUnit() : TNULL;
	pIcon->m_bOffEdge  = TFALSE;
	pIcon->m_vPosition = TVector4::VEC_ZERO;
	pIcon->m_bRotate   = a_bRotate;
	pIcon->m_eType     = a_eType;
	pIcon->m_oRect.SetTextureSection( *m_apIconSections[ a_eType ] );
	pIcon->m_oRect.GetTransform().SetIdentity();
	pIcon->m_oRect.SetInFront();
	m_oIconContainer.AddChildTail( pIcon->m_oRect );
	m_llUsedIcons.InsertTail( pIcon );

	TINT iIndex = 0;

	while ( &m_aIcons[ iIndex ] != pIcon )
	{
		if ( ++iIndex >= MAX_ICONS )
			return -1;
	}

	return iIndex;
}

// $Barnyard: FUNCTION 00458660
void ASimMiniMap::RemoveIcon( TINT a_iIndex )
{
	Icon* pIconToRemove = &m_aIcons[ a_iIndex ];

	for ( auto it = m_llUsedIcons.Begin(); it != m_llUsedIcons.End(); it++ )
	{
		if ( it == pIconToRemove )
		{
			it->m_oRect.RemoveSelf();
			it->Remove();
			m_llFreeIcons.InsertTail( pIconToRemove );
			return;
		}
	}
}

ASimMiniMap::Icon* ASimMiniMap::FindUnitIcon( AUnit* a_pUnit )
{
	for ( auto it = m_llUsedIcons.Begin(); it != m_llUsedIcons.End(); it++ )
	{
		if ( it->m_pUnit && it->m_pUnit == a_pUnit )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004586e0
void ASimMiniMap::RemoveUnitIcon( TRef<AUnitProxy> a_pUnitProxy )
{
	if ( Icon* pIcon = FindUnitIcon( a_pUnitProxy->GetUnit() ) )
	{
		pIcon->m_oRect.RemoveSelf();
		pIcon->Remove();
		m_llFreeIcons.InsertTail( pIcon );
	}
}

// $Barnyard: FUNCTION 00457680
void ASimMiniMap::SetIconVisible( TINT a_iIndex, TBOOL a_bVisible )
{
	m_aIcons[ a_iIndex ].m_oRect.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 004588e0
void ASimMiniMap::SetUnitIconVisible( TRef<AUnitProxy> a_pUnitProxy, TBOOL a_bVisible )
{
	if ( Icon* pIcon = FindUnitIcon( a_pUnitProxy->GetUnit() ) )
		pIcon->m_oRect.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00458970
void ASimMiniMap::SetPlayer( TRef<AUnitProxy> a_pPlayer )
{
	m_pPlayer = a_pPlayer;
}
