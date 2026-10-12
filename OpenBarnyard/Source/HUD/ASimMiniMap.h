#pragma once
#include "AHudElement.h"
#include "Units/AUnitProxy.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Toshi/TDList.h>
#include <Toshi/TRefCounted.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: the round map in the bottom left corner of the sim (EnvMain). It
// shows the area around the player and icons of units and places (quest givers,
// items...). Icons out of its range are drawn as arrows on its edge.
// Owned by ASimState (+0xbc).
// TClass 00782170, vtable 0071c6a4.
//-----------------------------------------------------------------------------
class ASimMiniMap
    : public AHudElement
{
public:
	TDECLARE_CLASS( ASimMiniMap, AHudElement );

	static constexpr TINT MAX_ICONS = 250;

	using ICON = TINT;
	enum ICON_ : ICON
	{
		ICON_CHALLENGE,
		ICON_MISSION,
		ICON_ITEM,
		ICON_ENEMY,
		ICON_PLAYER,
		ICON_NUMOF
	};

	struct Icon : public Toshi::TDList<Icon>::TNode
	{
		Icon();
		~Icon();

		AUnit*                m_pUnit;     // 0x08, the icon follows the unit if set
		Toshi::TVector4       m_vPosition; // 0x0c, used without a unit
		ICON                  m_eType;     // 0x1c
		TBOOL                 m_bRotate;   // 0x20, rotates with the direction of the unit
		Toshi::T2GUIRectangle m_oRect;     // 0x24
		TBOOL                 m_bOffEdge;  // 0x9c, out of range, shown as an arrow on the edge
	};

public:
	ASimMiniMap();
	ASimMiniMap( Mask a_eMask );
	~ASimMiniMap();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual void  GetDimensions( TINT& a_rWidth, TINT& a_rHeight ) override;
	virtual TBOOL OnAttach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Adds the map to AHudManager
	virtual TBOOL Create(); // +0x2c

	// Removes the map and its icons
	virtual void Destroy(); // +0x30

	// Adds an icon at a_rcPosition and returns its index (-1 if there's no free icon)
	TINT AddIcon( const Toshi::TVector4& a_rcPosition, ICON a_eType, TFLOAT a_fUnused, TBOOL a_bRotate );

	// Adds an icon following a_pUnitProxy and returns its index (-1 if there's no free icon or in the barn)
	TINT AddUnitIcon( Toshi::TRef<AUnitProxy> a_pUnitProxy, ICON a_eType, TFLOAT a_fUnused, TBOOL a_bRotate );

	void RemoveIcon( TINT a_iIndex );
	void RemoveUnitIcon( Toshi::TRef<AUnitProxy> a_pUnitProxy );

	void SetIconVisible( TINT a_iIndex, TBOOL a_bVisible );
	void SetUnitIconVisible( Toshi::TRef<AUnitProxy> a_pUnitProxy, TBOOL a_bVisible );

	// Sets the unit the map is centred on
	void SetPlayer( Toshi::TRef<AUnitProxy> a_pPlayer );

private:
	Icon* FindUnitIcon( AUnit* a_pUnit );

public:
	Toshi::TDList<Icon>         m_llUnk14;                      // 0x14, unused
	Toshi::T2GUITextureSection* m_apIconSections[ ICON_NUMOF ]; // 0x1c
	TUINT8                      PADDING1[ 4 ];                  // 0x30
	Toshi::T2GUIElement         m_oRoot;                        // 0x34, rotates with the camera
	Toshi::T2GUIRectangle       m_oBezel;                       // 0x88, "bezel"
	Toshi::T2GUIRectangle       m_oMap;                         // 0x100, "Map_EnvMain"
	Toshi::T2GUIRectangle       m_oMask;                        // 0x178, "mapmask"
	Toshi::T2GUIElement         m_oIconContainer;               // 0x1f0
	Toshi::T2GUIRectangle       m_oPlayerArrow;                 // 0x244
	TUINT8                      PADDING2[ 4 ];                  // 0x2bc
	TINT                        m_iUnk2C0;                      // 0x2c0, set to 1 by Show
	TUINT8                      PADDING3[ 0x2d4 - 0x2c4 ];      // 0x2c4
	TFLOAT                      m_fScreenWidth;                 // 0x2d4
	TFLOAT                      m_fScreenHeight;                // 0x2d8
	TUINT8                      PADDING4[ 0x30c - 0x2dc ];      // 0x2dc
	TFLOAT                      m_fUnk30C;                      // 0x30c
	TFLOAT                      m_fUnk310;                      // 0x310
	Toshi::TRef<AUnitProxy>     m_pPlayer;                      // 0x314
	Toshi::TVector2             m_vPlayerPos;                   // 0x318, (x, -z) of the player
	Toshi::TVector2             m_vCentreUV;                    // 0x320, UV of the player on the map texture
	Toshi::TVector2             m_aMapUVs[ 4 ];                 // 0x328
	Toshi::TVector2             m_vUnk348;                      // 0x348
	TFLOAT                      m_fWidth;                       // 0x350
	TFLOAT                      m_fHeight;                      // 0x354
	Icon                        m_aIcons[ MAX_ICONS ];          // 0x358
	Toshi::TDList<Icon>         m_llFreeIcons;                  // 0x9f98
	Toshi::TDList<Icon>         m_llUsedIcons;                  // 0x9fa0
};

TSTATICASSERT( sizeof( ASimMiniMap::Icon ) == 0xa0 );
TSTATICASSERT( sizeof( ASimMiniMap ) == 0x9fa8 );
