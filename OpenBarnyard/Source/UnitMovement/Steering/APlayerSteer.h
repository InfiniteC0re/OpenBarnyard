#pragma once
#include "ACommonSteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>
#include <Render/TCollisionCommon.h>

class ARenderer;
class AFade;
class AObjectTraits;
class AInteriorNetwork;
class AInteriorArea;
class AInteriorGoal;
class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: Steer of the player characters. Moves the player with a 3D kinematics
// (ASteerKinematics3D) and runs a small state machine on top of it: walking,
// standing still, jumping (JUMPSTATE sub states), falling, climbing ladders
// (CLIMBSTATE sub states) and throwing the carried object. Also handles falling
// into water (splash, fade to black), placing the player at interior goals and
// turning on the spot before moving (m_bTurning).
// The STATE_/JUMPSTATE_/CLIMBSTATE_ names and m_eActionState/m_eJumpState come
// from the original source (APlayerSteer.cpp) and the retail debug name tables.
// NOTE: vtable 0x00726520, TClass 0x007892f4 (parent ACommonSteer), size 0x5d8.
// NOTE: AAlienSteer (TClass 0x00788ac4, size 0x628) derives from this class.
//-----------------------------------------------------------------------------
class APlayerSteer : public ACommonSteer
{
public:
	TDECLARE_CLASS( APlayerSteer, ACommonSteer );

	// m_eActionState
	enum STATE : TINT
	{
		STATE_UNKNOWN,
		STATE_WALKING,
		STATE_STILL,
		STATE_JUMPING,
		STATE_FALLING,
		STATE_CLIMBING,
		STATE_THROW,
		STATE_NUMOF
	};

	// m_eJumpState
	enum JUMPSTATE : TINT
	{
		JUMPSTATE_NONE,
		JUMPSTATE_END,
		JUMPSTATE_FALL,
		JUMPSTATE_START,
		JUMPSTATE_UP,
		JUMPSTATE_APEX,
		JUMPSTATE_DOWN,
		JUMPSTATE_DOWN_END,
		JUMPSTATE_NUMOF
	};

	// m_eClimbState
	enum CLIMBSTATE : TINT
	{
		CLIMBSTATE_NONE,
		CLIMBSTATE_SEEK,
		CLIMBSTATE_ATTACH,
		CLIMBSTATE_MOVE,
		CLIMBSTATE_UP,
		CLIMBSTATE_DOWN,
		CLIMBSTATE_OFFLOAD_UP,
		CLIMBSTATE_OFFLOAD_DOWN,
		CLIMBSTATE_FALL,
		CLIMBSTATE_NUMOF
	};

	// Updates the current action state, returns TTRUE if the kinematics should be integrated
	using t_StateUpdate = TBOOL ( APlayerSteer::* )( TFLOAT a_fDeltaTime );

	// Animation event callback (see AnimEventCallback)
	using t_AnimEventCallback = void( __stdcall* )( APlayerSteer* a_pSteer, AAnimationBreakpointUnitAction::Event* a_pEvent );

	//-----------------------------------------------------------------------------
	// Listener of the global animation event list (AAnimationBreakpointUnitAction,
	// head at 0x0077e35c). Same layout as AAnimationBreakpointUnitAction::Listener.
	// NOTE: not a TGenericListener (no priority); many other classes have one too.
	// Disconnect is inlined in EndState and the destructor.
	//-----------------------------------------------------------------------------
	struct AnimEventListener
	{
		AnimEventListener()
		    : pNext( this ), pPrev( this ), pCaller( TNULL ), pCallback( TNULL )
		{}

		~AnimEventListener() { Disconnect(); }

		void Connect( APlayerSteer* a_pCaller, t_AnimEventCallback a_pCallback );
		void Disconnect();

		AnimEventListener*  pNext;     // 0x00
		AnimEventListener*  pPrev;     // 0x04
		APlayerSteer*       pCaller;   // 0x08
		t_AnimEventCallback pCallback; // 0x0c
	};

public:
	APlayerSteer();
	~APlayerSteer();

	//-----------------------------------------------------------------------------
	// ASteer
	//-----------------------------------------------------------------------------
	virtual Movement GetMovementType() OVERRIDE; // +0x10

	// NOTE: a_pAnimInterface is an AToshiAnimationInterface*, a_pDeltaTime a TFLOAT
	virtual TBOOL UpdateAnimation( void* a_pAnimInterface, void* a_pDeltaTime ) OVERRIDE; // +0x14

	virtual TBOOL CreateFrom( ASteer* a_pOther, TINT a_iLOD ) OVERRIDE;                                                                       // +0x20
	virtual TBOOL Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ) OVERRIDE; // +0x24

	// NOTE: update, a_pUnk is the delta time (TFLOAT)
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x30

	virtual TBOOL Command_StandStill() OVERRIDE;                                        // +0x58
	virtual TBOOL EndActionState() OVERRIDE;                                            // +0x5c, ends the current state
	virtual void  SetDesiredDirection( const Toshi::TVector4& a_rcDirection ) OVERRIDE; // +0x94
	virtual void  SetSpeechDialogMode( TBOOL a_bEnabled ) OVERRIDE;                     // +0x9c
	virtual void  SetDesiredSpeed( TFLOAT a_fSpeed ) OVERRIDE;                          // +0x104

	// NOTE: folded with other getters of a pointer at +0x20 (00555e00)
	virtual ASteerKinematics* GetKinematics() OVERRIDE { return m_pKinematics; } // +0x114

	virtual void  SetInteracting( TINT a_iUnk ) OVERRIDE;                                  // +0x11c
	virtual TBOOL CanStartAction() OVERRIDE;                                               // +0x128, TFALSE while an action animation (eat, wave, pickup...) plays
	virtual TBOOL GetTilt( void* a_pPitch, void* a_pRoll ) OVERRIDE;                       // +0x164
	virtual void  SetTilt( TFLOAT a_fPitch, TFLOAT a_fRoll, TBOOL a_bImmediate ) OVERRIDE; // +0x168

	//-----------------------------------------------------------------------------
	// ACommonSteer
	//-----------------------------------------------------------------------------
	virtual TBOOL SetDesiredVelocity( const Toshi::TVector4& a_rcVelocity ) OVERRIDE; // +0x174
	virtual TBOOL StopAndFace( const Toshi::TVector4& a_rcFacing ) OVERRIDE;          // +0x178
	virtual TBOOL SetGoal( AInteriorGoal* a_pGoal ) OVERRIDE;                         // +0x180

	virtual void*          GetCurrentNetwork() OVERRIDE;             // +0x18c, the network (AInteriorNetworkManager's default one if not connected)
	virtual AInteriorArea* GetCurrentArea() OVERRIDE;                // +0x190
	virtual TINT           GetAreaInNetwork( TINT a_iUnk ) OVERRIDE; // +0x194, a_iUnk is an AInteriorNetwork*, returns the AInteriorArea* the steer stands in

	//-----------------------------------------------------------------------------
	// APlayerSteer's own slots (+0x198..+0x1b4)
	//-----------------------------------------------------------------------------
	virtual TBOOL  SetState( STATE a_eState );                                                                     // +0x198
	virtual TBOOL  EndState( STATE a_eState );                                                                     // +0x19c
	virtual TBOOL  PlaceAtGoal( AInteriorGoal* a_pGoal, TBOOL a_bUseGoalFacing, TBOOL a_bSetGoal );                // +0x1a0
	virtual TBOOL  PlaceAtNamedGoal( const Toshi::TPString8& a_rcGoal, TBOOL a_bUseGoalFacing, TBOOL a_bSetGoal ); // +0x1a4
	virtual TFLOAT GetGroundHeight( const Toshi::TVector4& a_rcPosition );                                         // +0x1a8, highest hit of two rays cast down at the sides of a_rcPosition
	virtual void   Unk_0x1ac( TINT a_iUnk ) {}                                                                     // +0x1ac, DOES_NOTHING 00628f40

	// NOTE: folded with other float getters returning 0.0f (0056ce20)
	virtual TFLOAT Unk_0x1b0() { return 0.0f; } // +0x1b0

	virtual TBOOL SetDesiredFacing( const Toshi::TVector3& a_rcFacing ); // +0x1b4

	STATE      GetState() const { return m_eActionState; }
	void       SetRestoreAnimationSet( TBOOL a_bValue ) { m_bRestoreAnimationSet = a_bValue; }
	JUMPSTATE  GetJumpState() const { return m_eJumpState; }
	CLIMBSTATE GetClimbState() const { return m_eClimbState; }
	TBOOL      IsInWater() const { return m_bIsInWater; }
	void       SetLastPosition( const Toshi::TVector4& a_rcPosition ) { m_vLastPosition = a_rcPosition; }

	// Stops and turns towards a_rcFacing, moving waits until the turn is done
	TBOOL TurnTowards( const Toshi::TVector4& a_rcFacing );

	// Whether the steer was placed at the goal named a_rcGoal
	TBOOL IsAtGoal( const Toshi::TPString8& a_rcGoal );

	// Sets/clears the kinematics flag 0x200 (see AInteriorSteer::EnableKinematicsObstructionStop)
	void DisableKinematicsObstructionStop( TBOOL a_bDisable );

private:
	AInteriorNetwork* GetNetwork() { return TSTATICCAST( AInteriorNetwork, GetCurrentNetwork() ); }

	// ms_aStateUpdates
	TBOOL UpdateNone( TFLOAT a_fDeltaTime ) { return TFALSE; } // RETURNS_FALSE 006ce000, STATE_UNKNOWN and STATE_CLIMBING
	TBOOL UpdateWalking( TFLOAT a_fDeltaTime );
	TBOOL UpdateStill( TFLOAT a_fDeltaTime );
	TBOOL UpdateJumping( TFLOAT a_fDeltaTime );
	TBOOL UpdateFalling( TFLOAT a_fDeltaTime );
	TBOOL UpdateThrow( TFLOAT a_fDeltaTime );

	// Called by LadderAnimEndCallback after the animations are updated
	TBOOL UpdateClimbing( TFLOAT a_fDeltaTime );

	void             OnCreate();
	void             ApplyGroundGravity();
	void             DropToGround();
	void             PlaceAtCurrentGoal();
	TBOOL            UpdateGoal();
	AObjectTraits*   GetCarriedObjectTraits();
	TBOOL            StartFallAnimation();
	TBOOL            UpdateLanding( TFLOAT a_fDeltaTime );
	void             UpdateKinematics( TFLOAT a_fDeltaTime );
	void             UpdatePath( TFLOAT a_fDeltaTime );
	Toshi::TVector4& GetPointInFront( Toshi::TVector4& a_rOut );
	void             UpdateCarrying();
	void             UpdateWater( TFLOAT a_fDeltaTime );
	void             UpdateSinking();
	TBOOL            UpdateCollision( TFLOAT a_fDeltaTime, Toshi::TVector4* a_pPosition, TBOOL a_bUnk1, TBOOL a_bUnk2, TBOOL a_bUnk3 );
	void             PlayMilkDrip();
	void             OnAnimEvent( AAnimationBreakpointUnitAction::Event* a_pEvent );
	TBOOL            OnSinkFadeOver( AFade* a_pFade, TBOOL* a_pData );

	// Sweeps the collision sphere from a_rcCenter and slides it along the hit surfaces. Returns TTRUE if nothing was hit.
	TBOOL CollideAndSlide( const Toshi::TVector4& a_rcCenter, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rFacing, Toshi::TVector4& a_rNormal, TBOOL& a_rbUnk, TBOOL a_bSlopeSlowdown, ACollisionObject** a_ppHitObject, TFLOAT a_fDeltaTime );

	// Updates the falling/landing state after the collision, resets the player if it fell out of the world.
	void UpdateGroundState( const Toshi::TVector4& a_rcPrevPosition, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcNormal, TBOOL a_bCollided, TFLOAT a_fDeltaTime );

	// Lifts the player (negative gravity) if it walks into a step it can climb.
	void TryStepUp( TFLOAT a_fDeltaTime, TBOOL a_bSlopeSlowdown );

	// Moves a sphere with two collision spheres (the second one 1.8 radiuses higher), sliding along the hit surfaces.
	static TBOOL SweepAndSlide( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutNormal, ACollisionObject** a_ppHitObject, TUINT a_eFlags, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bSlopeSlowdown, TBOOL* a_pUnk, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope, TFLOAT a_fUnused );

	static TBOOL __stdcall LadderAnimEndCallback( APlayerSteer* a_pSteer, ARenderer* a_pRenderer, TBOOL** a_ppData );
	static TBOOL __stdcall SinkFadeOverCallback( APlayerSteer* a_pSteer, AFade* a_pFade, TBOOL* a_pData );
	static void __stdcall AnimEventCallback( APlayerSteer* a_pSteer, AAnimationBreakpointUnitAction::Event* a_pEvent );

public:
	static const TCHAR* ms_aStateNames[ STATE_NUMOF ];           // 0x007742cc, debug only
	static const TCHAR* ms_aJumpStateNames[ JUMPSTATE_NUMOF ];   // 0x007742e8, debug only
	static const TCHAR* ms_aClimbStateNames[ CLIMBSTATE_NUMOF ]; // 0x00774308, debug only

private:
	static t_StateUpdate ms_aStateUpdates[ STATE_NUMOF ]; // 0x00726380
	static TFLOAT        ms_fUpdateInterval;              // 0x007892f0, never written in retail

private:
	ASteerKinematics*                                 m_pKinematics;          // 0x20, ASteerKinematics3D
	AInteriorNetworkPath                              m_oPath;                // 0x24
	TFLOAT                                            m_fClimbSpeed;          // 0x4e0, last climbing speed (negative is down)
	TINT                                              m_iUnk4E4;              // 0x4e4
	TFLOAT                                            m_fGetOffStart;         // 0x4e8, climbed distance when ladder_getoff_start started
	TFLOAT                                            m_fUpdateTime;          // 0x4ec, accumulated until ms_fUpdateInterval
	STATE                                             m_eActionState;         // 0x4f0
	STATE                                             m_eNextState;           // 0x4f4, EndState( STATE_JUMPING ) sets it to STATE_FALLING
	JUMPSTATE                                         m_eJumpState;           // 0x4f8
	CLIMBSTATE                                        m_eClimbState;          // 0x4fc
	Toshi::TVector4                                   m_vLadderBase;          // 0x500, ladder area position, y snapped to the rungs
	Toshi::TVector4                                   m_vLadderStart;         // 0x510
	Toshi::TVector4                                   m_vLadderEnd;           // 0x520
	Toshi::TVector4                                   m_vDesiredFacing;       // 0x530
	AnimEventListener                                 m_oAnimEventListener;   // 0x540, connected while throwing
	TBOOL                                             m_bActionEnded;         // 0x550, set by the end_bp animation event (a ladder step ended)
	TBOOL                                             m_bGrounded;            // 0x551
	Toshi::TVector4                                   m_vThrowHandPosition;   // 0x554, cb_rlocator bone position
	TFLOAT                                            m_fThrowHandTime;       // 0x564, TScheduler total time when m_vThrowHandPosition was taken
	TBOOL                                             m_bSteppedUp;           // 0x568
	TBOOL                                             m_bUnk569;              // 0x569
	TBOOL                                             m_bSpeechDialogMode;    // 0x56a
	TBOOL                                             m_bTurning;             // 0x56b, moving waits until the steer faces m_vDesiredFacing
	TBOOL                                             m_bRestoreAnimationSet; // 0x56c
	Toshi::TListener<ARenderer, TBOOL*, APlayerSteer> m_oLadderAnimListener;  // 0x570, ARenderer::GetAnimationUpdateEndEmitter while climbing
	TBOOL                                             m_bUnk584;              // 0x584
	AInteriorGoal*                                    m_pGoal;                // 0x588
	Toshi::TVector4                                   m_vLastPosition;        // 0x58c, UpdateWater casts a ray from it to the current position
	TBOOL                                             m_bIsInWater;           // 0x59c
	TFLOAT                                            m_fWaterTime;           // 0x5a0
	TINT                                              m_iIgnoreGroups;        // 0x5a4, collision mask of UpdateCarrying's rays
	AFade*                                            m_pSinkFade;            // 0x5a8, fade to black after falling into water
	TINT                                              m_iWaterSplashAEffect;  // 0x5ac, "watersplash2a"
	TINT                                              m_iWaterSplashBEffect;  // 0x5b0, "watersplash2b"
	TINT                                              m_iWaterRippleEffect;   // 0x5b4, "waterripple"
	TINT                                              m_iMilkDripEffect;      // 0x5b8, "milkdrip"
	TINT                                              m_iBushHitEffect;       // 0x5bc, "bushhit"
	TINT                                              m_iBushesCue;           // 0x5c0
	Toshi::TListener<AFade, TBOOL, APlayerSteer>      m_oSinkFadeListener;    // 0x5c4, AFade::GetFadeOverEmitter of m_pSinkFade
};

TSTATICASSERT( sizeof( APlayerSteer ) == 0x5d8 );
