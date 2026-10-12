#pragma once
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Minigame map: a part of a map texture around a centre position (usually the
// player), optionally round (mapmask) and rotating with the camera or the
// player, with a player marker and icons for world positions. Icons out of
// the map's range are hidden or stick to the edge as arrows.
// Name from the Wii build (AGUI2MiniMap). vtable 0073a4e8, size 0x520.
// Used by AVeggiePatchDefenderGame, AHoneyCollectingGame, APrecisionSquirt,
// AChasingChicksMiniGame and AChickenCoopDefender.
//-----------------------------------------------------------------------------
class AGUI2MiniMap
    : public Toshi::T2GUIRectangle
{
public:
	enum ROTATION : TINT
	{
		ROTATION_NONE,
		ROTATION_CAMERA, // the map rotates with the current camera (Tick)
		ROTATION_PLAYER, // the map rotates with the direction passed to SetCentre
	};

	enum ICONTYPE : TINT
	{
		ICONTYPE_PLAYER, // the player marker, always in the centre (not added to the icon list)
		ICONTYPE_EDGE,   // stays on the edge of the map (with the edge texture) when out of range
		ICONTYPE_HIDE,   // hidden when out of range (any value other than 0 and 1)
	};

	// Indices to the icon texture tables
	enum ICON : TINT
	{
		ICON_APPLES,
		ICON_CHALLENGE,
		ICON_BIKE,
		ICON_GATE,
		ICON_PLAYER,
		ICON_MISSION,
		ICON_ITEM,
		ICON_ENEMY,
		ICON_DOT_WHITE,
		ICON_WHITE_STAR,
		ICON_HONEY,
		ICON_PING_RING,
		ICON_NUMOF
	};

	// Screen area the map size is relative to
	struct Viewport
	{
		TINT iX;
		TINT iY;
		TINT iWidth;
		TINT iHeight;
	};

	// NOTE: retail has no own vtable for this, it's a T2GUIRectangle with extra data
	struct Icon : public Toshi::T2GUIRectangle
	{
		Toshi::TVector2             vMapPos;        // 0x78, position on the map texture
		Toshi::TVector4             vDirection;     // 0x80, XZ direction
		ICONTYPE                    eType;          // 0x90
		TBOOL                       bEnabled;       // 0x94
		TFLOAT                      fScale;         // 0x98
		TFLOAT                      fSectionScale;  // 0x9c, scale of pSection to fit the map
		TBOOL                       bOnEdge;        // 0xa0, out of range (on the edge or hidden)
		Toshi::T2GUITextureSection* pSection;       // 0xa4
		Toshi::T2GUITextureSection* pEdgeSection;   // 0xa8, ICONTYPE_EDGE when on the edge
		TBOOL                       bFixedRotation; // 0xac, keeps facing up instead of following vDirection
	};

public:
	AGUI2MiniMap();
	~AGUI2MiniMap();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Resets the settings to the defaults (zoom 1, ROTATION_CAMERA, no icons)
	void Reset();

	// Creates the map:
	// a_szMapSection: texture section of the map
	// a_fScaleX/Y: size of the map relative to a_rcViewport
	// a_rcWorldPos1/a_rcMapPos1 and a_rcWorldPos2/a_rcMapPos2: two world positions and their pixel positions on the map texture
	// a_bRound: uses the round "mapmask" (and "bezel" frame), otherwise rectangular ("Chase_Radar" frame)
	// a_szCornerSection: if set, the frame is built from 4 copies of this section instead
	// a_bFrame: shows the bezel/radar frame (if a_szCornerSection is TNULL)
	// a_iMaxIcons: size of the icon list
	void Create( const TCHAR* a_szMapSection, TFLOAT a_fScaleX, TFLOAT a_fScaleY, const Viewport& a_rcViewport, const Toshi::TVector4& a_rcWorldPos1, const Toshi::TVector2& a_rcMapPos1, const Toshi::TVector4& a_rcWorldPos2, const Toshi::TVector2& a_rcMapPos2, TBOOL a_bRound, const TCHAR* a_szCornerSection, TBOOL a_bFrame, TINT a_iMaxIcons );

	// Sets the part of the map texture that is shown ([0.001; 1], 1 is the full texture)
	void SetZoom( TFLOAT a_fZoom );

	// Centres the map on a_rcPosition and rotates the player marker (and the map with ROTATION_PLAYER) to a_rcDirection. Call every frame.
	void SetCentre( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Adds an icon, returns its index or -1 if the list is full.
	// ICONTYPE_PLAYER sets the texture of the player marker and returns the number of icons instead.
	TINT AddIcon( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, ICONTYPE a_eType, const TCHAR* a_szSection, const TCHAR* a_szEdgeSection, TFLOAT a_fScale );

	// Same as above with the textures from the icon tables
	TINT AddIcon( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, ICONTYPE a_eType, ICON a_eIcon, TFLOAT a_fScale );

	void SetIconTransform( TINT a_iIcon, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );
	void SetIconVisible( TINT a_iIcon, TBOOL a_bVisible );
	// TFALSE keeps the icon upright (rotated against the map) instead of following its direction
	void SetIconRotates( TINT a_iIcon, TBOOL a_bRotates );
	void SetIconColour( TINT a_iIcon, TUINT32 a_uiColour );
	void SetIconScale( TINT a_iIcon, TFLOAT a_fScale );
	void SetIconAlpha( TINT a_iIcon, TFLOAT a_fAlpha );

	// Colours of the map texture, the frame, the player marker and all icons
	void SetColours( TUINT32 a_uiMapColour, TUINT32 a_uiFrameColour, TUINT32 a_uiPlayerColour, TUINT32 a_uiIconColour );

	void SetRotationMode( ROTATION a_eRotation ) { m_eRotation = a_eRotation; }
	TINT GetNumIcons() const { return m_iNumIcons; }

public:
	TFLOAT                m_fZoom;             // 0x78
	TFLOAT                m_fHalfZoom;         // 0x7c
	TFLOAT                m_fZoomDiagonal;     // 0x80
	TFLOAT                m_fHalfZoomDiagonal; // 0x84
	TFLOAT                m_fMapToScreenX;     // 0x88, map texture pixels to screen
	TFLOAT                m_fMapToScreenY;     // 0x8c
	TFLOAT                m_fWorldToMap;       // 0x90, world units to map texture pixels
	TFLOAT                m_fTextureWidth;     // 0x94, size of the map texture section
	TFLOAT                m_fTextureHeight;    // 0x98
	TFLOAT                m_fMapWidth;         // 0x9c, size on screen
	TFLOAT                m_fMapHeight;        // 0xa0
	TINT                  m_iNumIcons;         // 0xa4
	ROTATION              m_eRotation;         // 0xa8
	Toshi::TVector2       m_vCentreMapPos;     // 0xac
	Toshi::TVector2       m_vMapOrigin;        // 0xb4, map position of m_vWorldOrigin
	Toshi::TVector4       m_vWorldOrigin;      // 0xbc
	Toshi::TVector4       m_vCentreWorldPos;   // 0xcc
	Toshi::T2GUIRectangle m_oContainer;        // 0xdc, rotated by ROTATION_*
	Toshi::T2GUIRectangle m_oMap;              // 0x154, the map texture, parent of the icons
	Toshi::T2GUIRectangle m_oMask;             // 0x1cc, "mapmask" of round maps
	Toshi::T2GUIRectangle m_oFrame;            // 0x244, bezel/radar frame or parent of m_aCorners
	Toshi::T2GUIRectangle m_oPlayerIcon;       // 0x2bc
	Toshi::T2GUIRectangle m_aCorners[ 4 ];     // 0x334
	TBOOL                 m_bRound;            // 0x514
	TBOOL                 m_bHasCorners;       // 0x515
	Icon*                 m_pIcons;            // 0x518
	TINT                  m_iMaxIcons;         // 0x51c
};

TSTATICASSERT( sizeof( AGUI2MiniMap::Icon ) == 0xb0 );
TSTATICASSERT( sizeof( AGUI2MiniMap ) == 0x520 );
