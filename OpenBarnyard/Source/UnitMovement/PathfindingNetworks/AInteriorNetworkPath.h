#pragma once

#include "GameInterface/AHackedQuickSave.h"

#include <Math/TVector4.h>

class AInteriorGoal;

class AInteriorNetworkPath
{
public:
	static constexpr TINT MAX_NUM_PATH_AREAS = 550;

	//-----------------------------------------------------------------------------
	// Purpose: The areas a path goes through (stored from the back of
	// m_aPathAreas, the first one is the area the path starts in) and the
	// point the unit walks to next.
	// NOTE: 0x4a0 bytes
	//-----------------------------------------------------------------------------
	class BuildPathState
	{
	public:
		using FLAGS = TUINT8;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE      = 0,
			FLAGS_NOT_FOUND = BITFLAG( 0 ), // the search failed (the path still counts as followed)
			FLAGS_SIDE_STEP = BITFLAG( 1 ), // walking to m_vSideStepTarget to get around an obstruction
		};

	public:
		BuildPathState();
		~BuildPathState();

		void  Reset();
		void  ClearPath();
		void  AdvancePath( TINT a_iNumAreas );
		TINT  GetPathAreaIndex( TINT a_iIndex ) const;
		void  PushPathArea( TINT16 a_iAreaIndex );
		TBOOL ReachedDestination() const;
		void  SetState( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcDestination, const Toshi::TVector4& a_rcForward );
		void  SetSideStepTarget( const Toshi::TVector4& a_rcTarget );

	public:
		TINT            m_iPathHead;                        // 0x000
		TINT            m_iNumPathAreas;                    // 0x004
		TINT            m_iGoalIndex;                       // 0x008, goal the path leads to (-1 if none)
		TINT16          m_aPathAreas[ MAX_NUM_PATH_AREAS ]; // 0x00c
		Toshi::TVector4 m_vPosition;                        // 0x458, the next point to walk to
		Toshi::TVector4 m_vDestination;                     // 0x468
		Toshi::TVector4 m_vForward;                         // 0x478, facing at the destination (zero if none)
		TINT            m_iSpecialAreaIndex;                // 0x488, index of the first special area of the path (see AInteriorNetwork::FindSpecialAreaInPath)
		Toshi::TVector4 m_vSideStepTarget;                  // 0x48c, see FLAGS_SIDE_STEP
		FLAGS           m_uiFlags;                          // 0x49c
	};

public:
	AInteriorNetworkPath();
	~AInteriorNetworkPath();

	void SetStart( const Toshi::TVector4& a_rcPosition, TINT a_iAreaIndex );

	void OnSerialize( AHackedQuickSave::Buffer* a_pBuffer );
	void OnDeserialize( AHackedQuickSave::Buffer* a_pBuffer );

public:
	TFLOAT          m_fRadius;        // 0x000, radius of the unit
	TINT            m_iStartArea;     // 0x004, area the unit is in
	Toshi::TVector4 m_vStartPosition; // 0x008, position of the unit
	BuildPathState  m_oPath;          // 0x018
	TUINT32         m_uiHeightFlags;  // 0x4b8, passed to AInteriorNetwork::ResolvePointHeight
};

TSTATICASSERT( sizeof( AInteriorNetworkPath::BuildPathState ) == 0x4a0 );
TSTATICASSERT( sizeof( AInteriorNetworkPath ) == 0x4bc );
