#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateGoalInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"

#include <File/TTRB.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2SList.h>
#include <Toshi/TDList.h>

class AKinematicsObstruction;
class AUnit;

class AInteriorNetwork
    : public Toshi::TObject
    , public Toshi::TDList<AInteriorNetwork>::TNode
{
public:
	TDECLARE_CLASS( AInteriorNetwork, Toshi::TObject );

	// Result of ConnectUnit
	using CONNECT = TINT;
	enum CONNECT_ : CONNECT
	{
		CONNECT_FAILED  = 0, // never returned by ConnectUnit, the callers check it
		CONNECT_INSIDE  = 1, // the unit is inside of an area
		CONNECT_OUTSIDE = 2, // the unit walks to the closest area first
	};

	// Unknown virtual of the class, no override known
	virtual TBOOL Unknown1( void* ) { return TFALSE; } // +0x0c, RETURNS_FALSE

	// Node of the area search of FindPointAwayFrom (lives in a static pool, ms_aAreaSearchNodes)
	struct AreaSearchNode
	    : public Toshi::TDList<AreaSearchNode>::TNode
	{
		AInteriorArea* m_pArea; // 0x8
	};

	//-----------------------------------------------------------------------------
	// Purpose: Table of the search through the linked areas of BuildPathToArea
	// and BuildPathToAnyArea, one node per area. The open nodes are kept sorted
	// by their total cost.
	// NOTE: a node is only updated if its new heuristic is lower than the old
	// one, and the heuristic of an area never changes, so the first path found
	// to an area is kept (retail behaviour).
	// NOTE: static, 0x007845b0
	//-----------------------------------------------------------------------------
	class PathSearch
	{
	public:
		enum STATE : TINT
		{
			STATE_NEW,
			STATE_OPEN,
			STATE_CLOSED,
		};

		struct Node
		    : public Toshi::TDList<Node>::TNode
		{
			Node();

			TINT   m_iArea;      // 0x08
			TINT   m_iParent;    // 0x0c, area the node was reached from
			STATE  m_eState;     // 0x10
			TFLOAT m_fCost;      // 0x14, from the start
			TFLOAT m_fHeuristic; // 0x18, estimate to the target
			TFLOAT m_fTotal;     // 0x1c
		};

	public:
		PathSearch();
		~PathSearch();

		void  Reset();
		void  OpenStart( TINT a_iArea, TFLOAT a_fHeuristic );
		Node* PopBest();
		void  Visit( TINT a_iArea, TINT a_iParent, TFLOAT a_fCost, TFLOAT a_fHeuristic );
		TINT  GetParent( TINT a_iArea ) const;

	public:
		Node                m_aNodes[ AInteriorNetworkPath::MAX_NUM_PATH_AREAS ]; // 0x0000
		Toshi::TDList<Node> m_llOpenNodes;                                        // 0x44c0
	};

	struct INetwork
	{
		TINT           m_iNumAreas;
		TINT           m_iNumGoals;
		TINT           m_iNumSpecialAreas;
		TINT           m_iNumNormalAreas;
		AInteriorArea* m_pAreas;
		AInteriorGoal* m_pGoals;
		TINT*          m_piSpecialAreas;
		void**         m_ppSpecialAreaData;
		TFLOAT*        m_pfSpecialAreaHeights; // one per special area
		TINT           m_iUnk2;
		TINT           m_iUnk3;
		TINT           m_iUnk4;
	};

	struct LookupList
	{
		TINT  iCount;
		TINT* pIndices;

		operator TINT*() { return pIndices; }
	};

	static constexpr TUINT MAX_NUM_GROUPS            = 70;
	static constexpr TINT  MAX_NUM_AREA_SEARCH_NODES = 100;

public:
	AInteriorNetwork();
	~AInteriorNetwork();

	TBOOL LoadFromFile( const Toshi::TPString8& a_rcNetworkName, const TCHAR* a_szFilePath );
	void  ResolvePointHeight( TINT a_iAreaIndex, Toshi::TVector4& a_rPoint, TUINT a_uiFlags );

	TINT FindAreaIndex( const Toshi::TPString8& a_rcName ) const;
	TINT FindContainingArea( const Toshi::TVector4& a_rcPoint ) const;

	// Returns index of the first area containing a_rcPosition (XZ only), a_iHintArea is checked first, -1 if not found
	TINT           FindAreaAtPosition( const Toshi::TVector4& a_rcPosition, TINT a_iHintArea = -1 ) const;
	AInteriorGoal* FindGoal( const Toshi::TPString8& a_rcName ) const;

	// Returns the number of goal names that contain a_szSubString
	TINT           CountGoalsContaining( const TCHAR* a_szSubString ) const;
	AInteriorGoal* FindGoalInArea( const Toshi::TPString8& a_rcName, TINT a_iAreaIndex = -1 ) const;
	TBOOL          HasGoal( const Toshi::TPString8& a_rcName ) const;

	AInteriorArea* GetArea( TINT a_iIndex ) const { return &m_pINetwork->m_pAreas[ a_iIndex ]; }

	// Number of goals called a_rcName
	TINT GetNumGoals( const Toshi::TPString8& a_rcName ) const;

	// Same as FindGoal without the assert: returns the first goal of the network if there's none called a_szName
	AInteriorGoal* FindGoalOrFirst( const TCHAR* a_szName ) const;
	AInteriorGoal* FindRandomGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindRandomVisibleGoal( const Toshi::TPString8& a_rcName ) const;
	AInteriorGoal* FindFarthestGoal( const Toshi::TPString8& a_rcName, const Toshi::TVector4& a_rcPosition ) const;

	// Returns the goal called a_szName (case insensitive) closest to a_rcPosition, TNULL if there's none
	AInteriorGoal* FindClosestGoal( const TCHAR* a_szName, const Toshi::TVector4& a_rcPosition ) const;

	// Adds the unit's obstruction to the network and starts a_pPath in the area at a_rcPosition (or the closest one).
	// Returns CONNECT_INSIDE if a_rcPosition is inside of an area, CONNECT_OUTSIDE otherwise.
	CONNECT ConnectUnit( AInteriorNetworkPath* a_pPath, AUnit* a_pUnit, const Toshi::TVector4& a_rcPosition );

	// Resets a_pPath and removes the unit's obstruction from the network
	TBOOL DisconnectUnit( AInteriorNetworkPath* a_pPath, AUnit* a_pUnit );

	// Index of the area whose position is the closest (on the XZ plane) to a_rcPosition, -1 if there are no areas
	TINT FindClosestArea( const Toshi::TVector4& a_rcPosition ) const;

	// Restarts a_pPath in area a_iAreaIndex at the path's start position (copied to a_rOutPosition), a_rcUnused isn't used
	TBOOL SetPathStartArea( AInteriorNetworkPath* a_pPath, TINT a_iAreaIndex, Toshi::TVector4& a_rOutPosition, const Toshi::TVector4& a_rcUnused );

	// Height of the special area a_iAreaIndex (0.0f if it isn't one)
	TFLOAT GetSpecialAreaHeight( TINT a_iAreaIndex ) const;

	// Point a_iPoint of the special area a_iAreaIndex (of the first special area if it isn't one), e.g. the ends of a ladder
	const Toshi::TVector4& GetSpecialAreaPoint( TINT a_iAreaIndex, TINT a_iPoint ) const;

	// Same as FindClosestGoal, but returns the first goal of the network if there's none
	AInteriorGoal* FindClosestGoalOrFirst( const TCHAR* a_szName, const Toshi::TVector4& a_rcPosition ) const;

	// Returns the goal closest to a_rcPosition whose name contains a_szSubString (the first goal if there's none)
	AInteriorGoal* FindClosestGoalContaining( const TCHAR* a_szSubString, const Toshi::TVector4& a_rcPosition ) const;

	// Notifies (and unregisters) every goal interface of the collision group a_iGroupId, a_pUnused is not used
	void UpdateGoalCollisionGroup( void* a_pUnused, TINT a_iGroupId );

	// Unlinks all nodes from the free list of area search nodes
	static void ClearAreaSearchNodes();

	//-----------------------------------------------------------------------------
	// Paths
	//-----------------------------------------------------------------------------

	// Resets the areas and the next point of a_pPath (this isn't used)
	void ResetPath( AInteriorNetworkPath* a_pPath );

	// Searches the linked areas for a path from a_iStartArea to a_iTargetArea, the areas are pushed to a_pState
	TBOOL BuildPathToArea( TINT a_iStartArea, TINT a_iTargetArea, AInteriorNetworkPath::BuildPathState* a_pState );

	// Same as BuildPathToArea, but to any of the target areas (a_pOutArea gets the one that was reached)
	TBOOL BuildPathToAnyArea( TINT a_iStartArea, const TINT* a_pTargetAreas, TINT a_iNumTargetAreas, AInteriorNetworkPath::BuildPathState* a_pState, TINT* a_pOutArea );

	// Builds a path to the point (XZ packed into x and y) in area a_iTargetArea, the unit will face a_pForward (optional) once there
	TBOOL BuildPathToPoint( AInteriorNetworkPath* a_pPath, const Toshi::TVector2& a_rcTarget, TINT a_iTargetArea, const Toshi::TVector4* a_pForward );

	// Builds a path to the position in area a_iTargetArea, the unit will face a_pForward (optional) once there
	TBOOL BuildPathToPosition( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget, TINT a_iTargetArea, const Toshi::TVector4* a_pForward );

	// Builds a path to the position, in the area that contains it
	TBOOL BuildPathToContainingArea( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget );

	// Builds a path to a random point of the closest area called a_strType
	TBOOL BuildPathToTypedArea( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strType );

	// Builds a path to a random point of area a_iArea
	TBOOL BuildPathToRandomPointInArea( AInteriorNetworkPath* a_pPath, TINT a_iArea );

	// Builds a path to a random point of a random normal area (tries 50 areas, then the start area)
	TBOOL BuildPathToRandomArea( AInteriorNetworkPath* a_pPath );

	// Builds a path to the goal, a_bUseFacing makes the unit face the forward of the goal once there
	TBOOL BuildPathToGoal( AInteriorNetworkPath* a_pPath, AInteriorGoal* a_pGoal, TBOOL a_bUseFacing );

	// Builds a path to a random goal called a_strGoal that has no obstruction on it
	TBOOL BuildPathToRandomVisibleGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, TBOOL a_bUseFacing );

	// Builds a path to the goal called a_strGoal farthest from a_rcPosition
	TBOOL BuildPathToFarthestGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, const Toshi::TVector4& a_rcPosition );

	// Builds a path to the exit point of goal a_iGoal (see AInteriorGoal::FLAGS_HAS_EXIT)
	TBOOL BuildPathToGoalExit( AInteriorNetworkPath* a_pPath, TINT a_iGoal );

	// Builds the path to its destination (or goal) again. Returns TTRUE if the next point changed.
	TBOOL RebuildPath( AInteriorNetworkPath* a_pPath );

	// Updates the next point of the path from the area the unit is in, the path is rebuilt if the unit left it. Returns TTRUE if the next point changed.
	TBOOL UpdatePath( AInteriorNetworkPath* a_pPath );

	// Finds the farthest point (a_rOutPoint) of the path that can be walked to in a straight line from a_rcStart, and its area
	TBOOL FindNextPathPoint( AInteriorNetworkPath::BuildPathState* a_pState, TFLOAT a_fRadius, const Toshi::TVector2& a_rcStart, const Toshi::TVector2& a_rcTarget, Toshi::TVector2& a_rOutPoint, TINT* a_pOutArea );

	// Sets the next point of the path towards a_rcTarget, a_pForward (optional) is the facing at the destination
	TBOOL FinishPath( AInteriorNetworkPath* a_pPath, const Toshi::TVector2& a_rcTarget, const Toshi::TVector4* a_pForward );
	TBOOL FinishPath( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcTarget, const Toshi::TVector4* a_pForward );

	// Whether a special area is among the first a_iMaxAreas areas of the path (its index is stored in m_iSpecialAreaIndex)
	TBOOL FindSpecialAreaInPath( AInteriorNetworkPath::BuildPathState* a_pState, TINT a_iMaxAreas );

	// Moves the start of the path to a_rPosition, keeping the circle of the unit inside of the network (it slides along the outer edges).
	// a_rPosition gets the new position, a_pOutPush (optional) the average of the normals of the hit edges.
	void UpdateHardAvoidance( AInteriorNetworkPath* a_pPath, Toshi::TVector4& a_rPosition, Toshi::TVector2* a_pOutPush, TBOOL a_bResolveHeight );

	// Looks for the area of the path that contains the start position of the path. If there's one,
	// it becomes the start area and the path is advanced to it.
	// NOTE: the name is invented, only used by the Cow Tipping projectile aiming helper 0066ab90 (ACowTippingMiniGameState_Projectiles.cpp)
	void UpdateStartAreaFromPath( AInteriorNetworkPath* a_pPath );

	// Packs the XZ position of a_rcPosition into a_rOutPoint
	// NOTE: the name is invented, only used by the Cow Tipping projectile aiming helper 0066ab90 (ACowTippingMiniGameState_Projectiles.cpp)
	Toshi::TVector2& GetPointXZ( const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutPoint ) const;

	// Whether a_rcPosition is in the start area of the path with a_fRadiusScale times the radius of the path around it
	TBOOL IsInsideStartArea( AInteriorNetworkPath* a_pPath, const Toshi::TVector4& a_rcPosition, TBOOL a_bOnlyOuterEdges, TFLOAT a_fRadiusScale );

	// Places a_pPath at the goal (a_rOutPosition and a_rOutFacing get the position and the facing)
	TBOOL SnapPathToGoal( AInteriorNetworkPath* a_pPath, AInteriorGoal* a_pGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing );

	// Places a_pPath at a random goal called a_strGoal that has no obstruction on it
	TBOOL SnapPathToRandomVisibleGoal( AInteriorNetworkPath* a_pPath, Toshi::TPString8 a_strGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing );

	// Places a_pPath at the position of area a_iArea
	TBOOL SnapPathToArea( AInteriorNetworkPath* a_pPath, TINT a_iArea, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutFacing );

	//-----------------------------------------------------------------------------
	// Areas
	//-----------------------------------------------------------------------------

	// Index of the first area containing the point (XZ packed into x and y), -1 if there's none
	TINT FindAreaContainingPointXZ( const Toshi::TVector2& a_rcPoint ) const;

	// Index of the first area the circle fits in (a_iHintArea is checked first), -1 if there's none
	TINT FindAreaFittingCircle( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TINT a_iHintArea ) const;

	// Index of a random area. a_bNormalOnly picks a routable normal area (or any routable area after 20 tries).
	TINT GetRandomArea( TBOOL a_bNormalOnly );

	// Random position in a random area (see GetRandomArea)
	Toshi::TVector4 GetRandomAreaPosition( TBOOL a_bNormalOnly );

	// Every area index in a random order. a_piOutIndices[ 0 ] receives the number of indices, they follow it (up to 64).
	TBOOL GetShuffledAreas( TINT* a_piOutIndices ) const;

	// Searches the areas around a_rcPosition (up to a_iMaxDepth links away) for an area at least a_fMinDistance away in the direction of
	// a_rcDirection (the dot product with it has to be greater than a_fMinDot). a_bAllowCloser takes the farthest area if there's none.
	TBOOL FindPointAwayFrom( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMinDistance, const Toshi::TVector4& a_rcDirection, TFLOAT a_fMinDot, Toshi::TVector4& a_rOutPosition, TBOOL a_bAllowCloser, TINT a_iMaxDepth );

	// Whether a_rcPosition is close to the camera and in front of it (the height of the units is only resolved there)
	TBOOL IsVisibleFromCamera( const Toshi::TVector4& a_rcPosition );

	//-----------------------------------------------------------------------------
	// Goals and units
	//-----------------------------------------------------------------------------

	// The a_iIndex-th goal called a_strName, TNULL if there are fewer
	AInteriorGoal* FindGoalByIndex( Toshi::TPString8 a_strName, TINT a_iIndex ) const;

	// See AInteriorGoal::CanUnitUse
	TBOOL CanUnitUseGoal( TINT a_iGoal, AUnit* a_pUnit );

	// Whether the obstruction of the unit is in the network
	TBOOL IsUnitConnected( AUnit* a_pUnit );

	// Adds the kinematics obstruction of the unit (if it has one) to the network
	void AddUnitObstruction( AUnit* a_pUnit );

	// Removes the kinematics obstruction of the unit (if it has one) from the network
	void RemoveUnitObstruction( AUnit* a_pUnit );

	// Whether the kinematics obstruction of the unit is in the network (doesn't check if the unit has one)
	TBOOL HasUnitObstruction( AUnit* a_pUnit );


	void PushGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface, AInteriorGoal* a_pGoal );

	// Like PushGoalInterface, but leaves the interface's goal untouched
	void InsertGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface, AInteriorGoal* a_pGoal );

	const Toshi::TPString8& GetName() const { return m_strName; }

	TBOOL IsPersistent() const { return m_bIsPersistent; }
	void  SetPersistent( TBOOL a_bPersistent ) { m_bIsPersistent = a_bPersistent; }

private:
	friend class AInteriorNetworkManager;

	void BuildAreaNameLookup();
	void BuildGoalNameLookup();

	void ClearAreaNameLookup();
	void ClearGoalNameLookup();

	TBOOL EraseGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface );

	// Distance from a_iArea to the closest of the target areas, the heuristic of the path search (TMath::MAXFLOAT if a_iArea is a special area)
	TFLOAT GetHeuristicDistance( TINT a_iArea, const TINT* a_pTargetAreas, TINT a_iNumTargetAreas ) const;
	TBOOL  EraseGoalInterfaceFromGroup( AGoalCollisionUpdateInterface* a_pGoalInterface, TINT a_iGroupId );

public:
	// Free list of area search nodes (0x00788a78)
	static Toshi::TDList<AreaSearchNode> ms_oFreeAreaSearchNodes;

	// 0x007840d0
	static AreaSearchNode ms_aAreaSearchNodes[ MAX_NUM_AREA_SEARCH_NODES ];

	// 0x007845b0
	static PathSearch ms_oPathSearch;

public:
	Toshi::TTRB*     m_pTRB;
	INetwork*        m_pINetwork;
	Toshi::TPString8 m_strName;
	TBOOL            m_bSetHeightFromEnv;

	TINT                                          m_iMaxGroup;
	Toshi::T2SList<AGoalCollisionUpdateInterface> m_vecGroups[ MAX_NUM_GROUPS ];

	AGoalCollisionUpdateGoalInterface*           m_pUpdateGoalInterfaces;
	Toshi::T2Vector<AKinematicsObstruction*, 82> m_vecObstructions; // 0x13c, of the units connected to the network

	// ...

	// Persistent networks (the ones loaded on startup) are kept by AInteriorNetworkManager::Reset
	TBOOL m_bIsPersistent; // 0x288

	Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>* m_pAreaNameLookup;
	Toshi::T2Map<Toshi::TPString8, LookupList, Toshi::TPString8::Comparator>* m_pGoalNameLookup;
};
