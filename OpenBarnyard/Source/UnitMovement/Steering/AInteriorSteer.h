#pragma once
#include "ACommonSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"

#include <Toshi/TPString8.h>

class AInteriorNetwork;

//-----------------------------------------------------------------------------
// Purpose: Steer of units that walk an interior pathfinding network
// (AInteriorNetwork). Keeps an AInteriorNetworkPath through the network's
// areas and drives a 2D kinematics (ASteerKinematics2D) along it. Exposes the
// network commands (seek goal/area/point/unit, snap to goal/area, locators) to
// the FSM scripts.
// NOTE: the network the steer is connected to is ACommonSteer::m_pNetwork
// (returned by the ACommonSteer slot +0x18c).
// NOTE: vtable 0x00725d20, TClass 0x00789270, function table 0x00788da0.
//-----------------------------------------------------------------------------
class AInteriorSteer : public ACommonSteer
{
public:
	TDECLARE_CLASS( AInteriorSteer, ACommonSteer );

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_SPECIAL_AREA            = BITFLAG( 0 ),  // 0x001, the unit stands on a special area of the path (set by UpdatePosition)
		FLAGS_INTERACTION             = BITFLAG( 1 ),  // 0x002, SetInteracting, stops the path following
		FLAGS_IGNORE_INVALID_POSITION = BITFLAG( 2 ),  // 0x004, SetIgnoreInvalidPosition/IsIgnoringInvalidPosition
		FLAGS_OFF_NETWORK             = BITFLAG( 3 ),  // 0x008, ConnectToNetwork found the unit outside the network, walking back to it
		FLAGS_SEEKING_PATH_END        = BITFLAG( 4 ),  // 0x010
		FLAGS_SEEKING_UNIT            = BITFLAG( 5 ),  // 0x020, Command_SeekUnit
		FLAGS_AVOIDING_UNIT           = BITFLAG( 6 ),  // 0x040, Command_AvoidUnit
		FLAGS_KINEMATICS_NO_PUSH      = BITFLAG( 7 ),  // 0x080, ASteerKinematics::UPDATEFLAGS_NO_PUSH in UpdateKinematics
		FLAGS_KINEMATICS_NO_AVOID     = BITFLAG( 8 ),  // 0x100, ASteerKinematics::UPDATEFLAGS_NO_AVOID in UpdateKinematics
		FLAGS_NO_OBSTRUCTION_STOP     = BITFLAG( 9 ),  // 0x200, EnableKinematicsObstructionStop( TFALSE )
		FLAGS_SPEECH_DIALOG_MODE      = BITFLAG( 10 ), // 0x400, SetSpeechDialogMode
	};

	// m_eMode
	enum MODE : TINT
	{
		MODE_UNK0,
		MODE_STILL,  // Command_StateStill / attached to a locator, the kinematics isn't integrated
		MODE_NORMAL, // default
		MODE_FALLING // set by OnCreate when the unit doesn't stand on the ground
	};

public:
	AInteriorSteer();
	~AInteriorSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() OVERRIDE;                                                                                                // +0x0c
	virtual void            Serialise( AHackedQuickSave::Buffer* a_pStream ) OVERRIDE;                                                                  // +0x18
	virtual void            Deserialise( AHackedQuickSave::Buffer* a_pStream ) OVERRIDE;                                                                // +0x1c
	virtual TBOOL           CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL           Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24
	virtual TBOOL           Update( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                     // +0x30
	virtual TBOOL           HasArrived() OVERRIDE;                                                                                                      // +0x34
	virtual TBOOL           Command_WanderPathfindingSpecific() OVERRIDE;                                                                               // +0x4c
	virtual TBOOL           Command_SeekUnit( AUnit* a_pUnit, TFLOAT a_fDistance ) OVERRIDE;                                                            // +0x60
	virtual void            Command_CancelCurrentCommand() OVERRIDE;                                                                                    // +0x70
	virtual void            SetSpeechDialogMode( TBOOL a_bEnabled ) OVERRIDE;                                                                           // +0x9c

	// NOTE: folded with other getters of a pointer at +0x20 (00555e00)
	virtual ASteerKinematics* GetKinematics() OVERRIDE { return m_pKinematics; } // +0x114

	virtual void  SetInteracting( TINT a_iUnk ) OVERRIDE;                // +0x11c
	virtual TBOOL IsInteracting() OVERRIDE;                              // +0x120
	virtual void  SetIgnoreInvalidPosition( TINT a_iUnk ) OVERRIDE;      // +0x124
	virtual TBOOL IsIgnoringInvalidPosition() OVERRIDE;                  // +0x12c
	virtual void  ResumeMovement() OVERRIDE;                             // +0x130
	virtual TBOOL CanSeek( const Toshi::TVector4& a_rcTarget ) OVERRIDE; // +0x134

	//-----------------------------------------------------------------------------
	// ACommonSteer
	//-----------------------------------------------------------------------------
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x174
	virtual TBOOL Command_SnapToGoal( Toshi::TPString8 a_strGoal ) OVERRIDE;          // +0x17c, FSM Command_SnapToGoal
	virtual TBOOL SetGoal( AInteriorGoal* a_pGoal ) OVERRIDE;                         // +0x180
	virtual TINT  SetNetwork( AInteriorNetwork* a_pNetwork, TINT a_iUnk ) OVERRIDE;   // +0x184

	// NOTE: folded with other getters of a pointer at +0x18 (005f7a30)
	virtual void* GetCurrentNetwork() OVERRIDE { return m_pNetwork; } // +0x18c, the network

	virtual AInteriorArea* GetCurrentArea() OVERRIDE; // +0x190

	//-----------------------------------------------------------------------------
	// AInteriorSteer's own slots (+0x198..+0x19c)
	//-----------------------------------------------------------------------------
	virtual void  Command_CancelCurrentCommandNoStop(); // +0x198
	virtual TBOOL IsOnSpecialPathArea();                // +0x19c

	//-----------------------------------------------------------------------------
	// FSM natives
	//-----------------------------------------------------------------------------
	TBOOL Command_SeekGoal( Toshi::TPString8 a_strGoal );
	TBOOL Command_SeekGoalAndUseFacing( Toshi::TPString8 a_strGoal );
	TBOOL Command_SeekTypedArea( Toshi::TPString8 a_strType );
	TBOOL Command_SeekUnitPoint( AUnit* a_pUnit );
	TBOOL Command_SeekPointXYZ( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ );
	TBOOL Command_AvoidUnit( AUnit* a_pUnit, TFLOAT a_fDistance, TBOOL a_bUnk );
	TBOOL IsNetworkValid();
	TBOOL Command_FaceGoalFacing( Toshi::TPString8 a_strGoal );
	TBOOL Command_FaceGoal( Toshi::TPString8 a_strGoal );
	TBOOL Command_RotateFacing( TFLOAT a_fDegrees );
	TBOOL Command_StateStill();
	TBOOL Command_EndStateStill();
	TBOOL Command_SeekPlayer();
	TBOOL Command_SnapConnectNamedNetwork( Toshi::TPString8 a_strNetwork, TBOOL a_bUnk );
	TBOOL Command_DisconnectFromNetwork();
	TBOOL Command_IsConnectedToNetwork( Toshi::TPString8 a_strNetwork );
	TBOOL IsCloseToGoal( Toshi::TPString8 a_strGoal, TFLOAT a_fDistance );
	TBOOL Command_PatrolUnitPoint( AUnit* a_pUnit, TFLOAT a_fDistance, TFLOAT a_fAngle );
	TBOOL Command_PatrolUnitPointKeepDist( AUnit* a_pUnit, TFLOAT a_fAngle );
	TBOOL Command_SeekUnitPointOver( AUnit* a_pUnit );
	TBOOL Command_SeekFarGoalUnit( Toshi::TPString8 a_strGoal, AUnit* a_pUnit );
	TBOOL Command_SnapToNamedArea( Toshi::TPString8 a_strArea );
	void  EnableKinematicsObstructionStop( TBOOL a_bEnable );
	void  AttachToLocatorPS( const Toshi::TPString8& a_rcLocator );
	void  RemoveFromLocator();
	TBOOL WillGoalBeInFrustum( const Toshi::TPString8& a_rcGoal );
	TBOOL WillLocatorBeInFrustum( const Toshi::TPString8& a_rcLocator );

	// Places the unit at the position of area a_iAreaIndex of its network
	TBOOL SnapToArea( TINT a_iAreaIndex );

	// Restarts the path in area a_iAreaIndex at the current path start (the unit isn't moved)
	TBOOL SetStartArea( TINT a_iAreaIndex );

	// Whether the steer is following a path
	TBOOL IsFollowingPath();

	// Sets m_eMode (see MODE)
	TBOOL SetMode( MODE a_eMode );
	MODE  GetMode() const { return m_eMode; }

	// Whether the network of the steer has a goal called a_strGoal
	TBOOL HasGoal( Toshi::TPString8 a_strGoal );

	// Starts following a path to a_rcPoint (in area a_iArea) unless the steer is already following one
	TBOOL SeekPointInArea( const Toshi::TVector4& a_rcPoint, TINT a_iArea );

	// The path through the network the unit follows
	const AInteriorNetworkPath& GetPath() const { return m_oPath; }
	AInteriorNetworkPath&       GetPath() { return m_oPath; }

	// Sets AInteriorNetworkPath::m_uiHeightFlags of the path (inlined store, AVeggiePatchThief)
	void SetPathHeightFlags( TUINT32 a_uiFlags ) { m_oPath.m_uiHeightFlags = a_uiFlags; }

	// Starts following a path to the area containing a_rcTarget. Fails if the steer has no network,
	// is already following a path, or no path is found (unlike SeekPoint, the point isn't moved).
	TBOOL SeekPosition( const Toshi::TVector4& a_rcTarget );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devnpcwalk", see AInteriorSteer.cpp
	void Dev_UpdateNPCWalk();
#endif

	TBOOL ConnectToNetwork( AInteriorNetwork* a_pNetwork );

private:
	AInteriorNetwork* GetNetwork() { return TSTATICCAST( AInteriorNetwork, GetCurrentNetwork() ); }

	void  OnCreate();
	void  ResetCommand();
	TBOOL IsOnGround();
	void  StartFollowingPath();
	void  StopFollowingPath( AInteriorNetwork* a_pNetwork );
	void  UpdateModelTransform();
	TBOOL UpdateSeekUnit();
	TBOOL UpdateAvoidUnit();
	void  UpdateNoNetwork( TFLOAT a_fDeltaTime );
	void  UpdateSpecialArea( TFLOAT a_fDeltaTime, AInteriorNetwork* a_pNetwork );
	void  UpdatePathFollowing( TFLOAT a_fDeltaTime, AInteriorNetwork* a_pNetwork, TBOOL* a_pRepath, TBOOL* a_pUnk );
	void  OnReachedPathEnd( AInteriorNetwork* a_pNetwork, TBOOL* a_pRepath, TBOOL* a_pUnk );
	void  UpdateObstruction( TFLOAT a_fDeltaTime );
	void  UpdateMovement( TFLOAT a_fDeltaTime, AInteriorNetwork* a_pNetwork );
	void  UpdateKinematics( TFLOAT a_fDeltaTime, AInteriorNetwork* a_pNetwork );
	void  UpdatePosition( TFLOAT a_fDeltaTime, AInteriorNetwork* a_pNetwork, TBOOL a_bUnk, Toshi::TVector4* a_pPosition );
	TBOOL PatrolPoint( const Toshi::TVector4& a_rcPoint, TFLOAT a_fDistance, TFLOAT a_fAngle );
	TBOOL SeekFarGoal( Toshi::TPString8 a_strGoal, const Toshi::TVector4& a_rcPosition );
	void  AttachToLocator( const TCHAR* a_szLocator );

public:
	// Also used by the cow tipping minigame to make a unit walk to where a projectile lands
	TBOOL SeekPoint( const Toshi::TVector4& a_rcPoint );

	TBOOL IsSpeechDialogMode() const { return HASANYFLAG( m_eFlags, FLAGS_SPEECH_DIALOG_MODE ); }

	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

	static AFunctionTable ms_oFunctionTable;

private:
	inline static TFLOAT ms_fPathRadius      = 0.5f;  // DAT_007742c0, AInteriorNetworkPath::m_fRadius of every new steer
	inline static TFLOAT ms_fUpdateTimeShift = 0.01f; // DAT_007742c4, spreads the updates of the steers over frames
	inline static TFLOAT ms_fUpdateInterval  = 0.01f; // DAT_007742c8

private:
	ASteerKinematics*       m_pKinematics;       // 0x20, ASteerKinematics2D
	Toshi::TRef<AUnitProxy> m_oTargetUnit;       // 0x24, Command_SeekUnit/Command_AvoidUnit
	TFLOAT                  m_fTargetDistance;   // 0x28
	AInteriorNetworkPath    m_oPath;             // 0x2c
	FLAGS                   m_eFlags;            // 0x4e8
	TFLOAT                  m_fRepathTime;       // 0x4ec, the path is rebuilt when it runs out (reset to 20.0f)
	TFLOAT                  m_fTargetUpdateTime; // 0x4f0, the seek/avoid path is rebuilt every second
	TBOOL                   m_bAvoidUnk;         // 0x4f4, Command_AvoidUnit's third argument
	TFLOAT                  m_fUpdateTime;       // 0x4f8
	MODE                    m_eMode;             // 0x4fc
	TINT                    m_iUnk500;           // 0x500, reset whenever m_eMode changes
	TFLOAT                  m_fObstructionTime;  // 0x504
};

TSTATICASSERT( sizeof( AInteriorSteer ) == 0x508 );
