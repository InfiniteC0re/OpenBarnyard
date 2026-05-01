#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorArcBounds.h"

#include <Math/TVector4.h>
#include <Toshi/TPString8.h>

class AInteriorNetwork;

struct AInteriorAreaLink
{
	TINT   m_iAreaIndex;
	TFLOAT m_fCost;
};

class AInteriorArea
{
public:
	AInteriorArea() = default;
	~AInteriorArea() = default;

	enum FLAGS : TUINT32
	{
		FLAGS_HEIGHT_RAYCAST_LONG  = BITFLAG( 2 ),
		FLAGS_HEIGHT_RAYCAST_SHORT = BITFLAG( 3 ),
		FLAGS_HEIGHT_FROM_TERRAIN  = BITFLAG( 4 ),
		FLAGS_ACCEPT_GOAL_HEIGHT   = BITFLAG( 5 ),
		FLAGS_NORMAL_NAV_AREA      = BITFLAG( 6 ),
		FLAGS_SPECIAL_GROUP_AREA   = BITFLAG( 7 ),
		FLAGS_NON_ROUTABLE         = BITFLAG( 9 ),
		FLAGS_RUNTIME_SPECIAL      = BITFLAG( 10 ),
	};

public:
	void  GetPosition( Toshi::TVector4& a_rOutPosition ) const;
	void  GetPositionXZ( Toshi::TVector4& a_rOutPosition ) const;
	TBOOL CheckPointInsideArea( const Toshi::TVector4& a_rcPoint ) const;

public:
	TFLOAT              m_fOriginX;
	TFLOAT              m_fOriginZ;
	TFLOAT              m_fMinX;
	TFLOAT              m_fMinZ;
	TFLOAT              m_fMaxX;
	TFLOAT              m_fMaxZ;
	AInteriorNetwork*   m_pNetwork;
	TINT                m_iIndex;
	TINT                m_iNumArcBounds;
	AInteriorArcBounds* m_pArcBounds;
	TINT                m_iNumLinks;
	AInteriorAreaLink*  m_pLinks;
	TFLOAT              m_fBaseHeight;
	TFLOAT              m_fPathHeuristicScale;

	// HACK: swap this with pooled name after reading the TRB data
	union
	{
		TCHAR*            m_szName_Internal;
		Toshi::TPString8* m_strName;
	};

	FLAGS m_eFlags;
};
