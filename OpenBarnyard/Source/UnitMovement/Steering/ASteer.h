#pragma once
#include "GameInterface/AHackedQuickSave.h"
#include <Toshi/TObject.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>

#include "Units/AUnitProxy.h"

class AUnit;
class AFunctionTable;
class ASteerKinematics;
class ASteerTraits;

//-----------------------------------------------------------------------------
// Purpose: Base class of every unit steer (movement controller). Owned by an
// AUnit (AUnit::m_pSteer), allocated from ASteerPool. Most of the base class
// forwards to the steer's ASteerKinematics (GetKinematics, +0x114), which the
// base class doesn't have (RETURNS_ZERO), so the subclasses provide it.
// Names come from the FSM bindings (InitialiseClass) and JPOG's ASteer exports
// where the behaviour matches.
// NOTE: overloaded virtuals get distinct names (CreateFrom, SetPosition3,
// CopyPosition3, ...) since MSVC reorders overloads inside the vtable.
//-----------------------------------------------------------------------------
class ASteer : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASteer, Toshi::TObject );

	// NOTE: MOVEMENT_RUN/MOVEMENT_SPRINT are original names (APlayerSteer.cpp), the first two are guessed.
	enum Movement : TINT
	{
		MOVEMENT_STOPPED,
		MOVEMENT_WALK,
		MOVEMENT_RUN,   // above ASteerTraits::m_fRunSpeed
		MOVEMENT_SPRINT // above ASteerTraits::m_fSprintSpeed
	};

public:
	ASteer();
	~ASteer();

	virtual AFunctionTable*        GetFunctionTable();                                                                                                // +0x0c
	virtual Movement               GetMovementType();                                                                                                 // +0x10, updates the cached type from the speed (hysteresis)
	virtual TBOOL                  UpdateAnimation( void* a_pAnimInterface, void* a_pDeltaTime ) { return TFALSE; }                                   // +0x14, RETURNS_ZERO 00556a00
	virtual void                   Serialise( AHackedQuickSave::Buffer* a_pStream );                                                                  // +0x18
	virtual void                   Deserialise( AHackedQuickSave::Buffer* a_pStream );                                                                // +0x1c
	virtual TBOOL                  CreateFrom( ASteer* a_pOther, TINT a_iLOD );                                                                       // +0x20, takes over from the previous steer (AUnit::ChangeSteer)
	virtual TBOOL                  Create( AUnit* a_pOwner, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iLOD ); // +0x24, AUnit::Create
	virtual TINT                   Unk_0x28() { return 0; }                                                                                           // +0x28, RETURNS_ZERO 0054d8b0
	virtual void                   Unk_0x2c( TINT a_iUnk ) {}                                                                                         // +0x2c, DOES_NOTHING 00628f40
	virtual TBOOL                  Update( TFLOAT a_fDeltaTime ) { return TFALSE; }                                                                   // +0x30, RETURNS_FALSE 006ce000
	virtual TBOOL                  HasArrived() { return TFALSE; }                                                                                    // +0x34, RETURNS_FALSE 00660960
	virtual TBOOL                  IsFacingDesired( TFLOAT a_fTolerance );                                                                            // +0x38
	virtual TBOOL                  IsCommandSuccess() { return TFALSE; }                                                                              // +0x3c, RETURNS_FALSE 00660960
	virtual TBOOL                  IsCommandFailure() { return TFALSE; }                                                                              // +0x40, RETURNS_FALSE 00660960
	virtual TBOOL                  IsCommandFinished();                                                                                               // +0x44
	virtual TBOOL                  Unk_0x48( void* a_pUnk1, void* a_pUnk2 ) { return TFALSE; }                                                        // +0x48, RETURNS_ZERO 00556a00
	virtual TBOOL                  Command_WanderPathfindingSpecific() { return TFALSE; }                                                             // +0x4c, RETURNS_FALSE 00660960
	virtual TBOOL                  Unk_0x50() { return TFALSE; }                                                                                      // +0x50, RETURNS_FALSE 00660960
	virtual TBOOL                  Unk_0x54() { return TFALSE; }                                                                                      // +0x54, RETURNS_FALSE 00660960
	virtual TBOOL                  Command_StandStill() { return TFALSE; }                                                                            // +0x58, RETURNS_FALSE 00660960
	virtual TBOOL                  EndActionState() { return TFALSE; }                                                                                // +0x5c, RETURNS_FALSE 00660960
	virtual TBOOL                  Command_SeekUnit( AUnit* a_pUnit, TFLOAT a_fDistance ) { return TFALSE; }                                          // +0x60, RETURNS_ZERO 00556a00
	virtual TBOOL                  Command_FaceDirection( const Toshi::TVector4& a_rcDirection );                                                     // +0x64
	virtual TBOOL                  Command_FaceUnit( AUnit* a_pUnit );                                                                                // +0x68
	virtual TBOOL                  Unk_0x6c( void* a_pUnk1, void* a_pUnk2 ) { return TFALSE; }                                                        // +0x6c, RETURNS_ZERO 00556a00
	virtual void                   Command_CancelCurrentCommand() {}                                                                                  // +0x70, DOES_NOTHING 005acd80
	virtual void                   Unk_0x74() {}                                                                                                      // +0x74, DOES_NOTHING 005acd80
	virtual void                   Unk_0x78() {}                                                                                                      // +0x78, DOES_NOTHING 005acd80
	virtual void                   SetLOD( TINT a_iOldLOD, TINT a_iNewLOD );                                                                          // +0x7c
	virtual TFLOAT                 GetCollisionRadius();                                                                                              // +0x80
	virtual TBOOL                  SetPosition( const Toshi::TVector4& a_rcPosition );                                                                // +0x84
	virtual TBOOL                  SetPosition3( const Toshi::TVector3& a_rcPosition );                                                               // +0x88
	virtual void                   SetDirection( const Toshi::TVector4& a_rcDirection );                                                              // +0x8c
	virtual TBOOL                  SetRandomFacing();                                                                                                 // +0x90
	virtual void                   SetDesiredDirection( const Toshi::TVector4& a_rcDirection );                                                       // +0x94
	virtual void                   Unk_0x98( TINT a_iUnk ) {}                                                                                         // +0x98, DOES_NOTHING 00628f40
	virtual void                   SetSpeechDialogMode( TBOOL a_bEnabled ) {}                                                                         // +0x9c, DOES_NOTHING 00628f40
	virtual AUnit*                 GetTargetedUnit() { return TNULL; }                                                                                // +0xa0, RETURNS_ZERO 0054d8b0
	virtual void                   CopyPosition( Toshi::TVector4& a_rOutPosition );                                                                   // +0xa4
	virtual void                   CopyPosition3( Toshi::TVector3& a_rOutPosition );                                                                  // +0xa8
	virtual const Toshi::TVector4* GetPosition();                                                                                                     // +0xac
	virtual void                   CopyDirection( Toshi::TVector4& a_rOutDirection );                                                                 // +0xb0
	virtual void                   CopyDirection3( Toshi::TVector3& a_rOutDirection );                                                                // +0xb4
	virtual const Toshi::TVector4* GetDirection();                                                                                                    // +0xb8
	virtual void                   CopyVelocity( Toshi::TVector4& a_rOutVelocity );                                                                   // +0xbc
	virtual Toshi::TVector4        GetVelocity();                                                                                                     // +0xc0
	virtual TFLOAT                 GetSpeed();                                                                                                        // +0xc4
	virtual TFLOAT                 GetDesiredSpeed();                                                                                                 // +0xc8
	virtual TFLOAT                 GetSpeedRatio();                                                                                                   // +0xcc
	virtual TFLOAT                 GetMeasuredSpeedRatio();                                                                                           // +0xd0
	virtual TFLOAT                 GetMeasuredSpeed();                                                                                                // +0xd4, drives GetMovementType
	virtual TFLOAT                 GetTurnSpeed();                                                                                                    // +0xd8
	virtual TFLOAT                 GetSpeedFromModifier( TFLOAT a_fModifier );                                                                        // +0xdc
	virtual TBOOL                  Unk_0xe0() { return TTRUE; }                                                                                       // +0xe0, RETURNS_TRUE 0046cde0
	virtual TFLOAT                 GetGroundY();                                                                                                      // +0xe4
	virtual Toshi::TVector4        GetGroundPosition();                                                                                               // +0xe8, position with y = GetGroundY()
	virtual Toshi::TVector4&       PredictPosition( TINT a_iUnk, Toshi::TVector4& a_rOut );                                                           // +0xec
	virtual TBOOL                  FaceTowards( const Toshi::TVector4& a_rcTarget );                                                                  // +0xf0
	virtual TBOOL                  FaceCamera();                                                                                                      // +0xf4, returns FaceTowards
	virtual TBOOL                  Unk_0xf8() { return TFALSE; }                                                                                      // +0xf8, RETURNS_FALSE 00660960
	virtual void                   Unk_0xfc( TINT a_iUnk ) {}                                                                                         // +0xfc, DOES_NOTHING 00628f40
	virtual void                   SetSpeed( TFLOAT a_fSpeed );                                                                                       // +0x100
	virtual void                   SetDesiredSpeed( TFLOAT a_fSpeed );                                                                                // +0x104
	virtual void                   SetVelocity( const Toshi::TVector4& a_rcVelocity ) {}                                                              // +0x108, DOES_NOTHING 00628f40
	virtual void                   Unk_0x10c( TINT a_iUnk ) {}                                                                                        // +0x10c, DOES_NOTHING 00628f40
	virtual TBOOL                  Unk_0x110() { return TFALSE; }                                                                                     // +0x110, RETURNS_FALSE 00660960
	virtual ASteerKinematics*      GetKinematics() { return TNULL; }                                                                                  // +0x114, RETURNS_ZERO 0054d8b0, subclasses return their m_pKinematics
	virtual void                   SetHasInteraction( TINT a_iValue );                                                                                // +0x118, AUnit::SetHasInteraction
	virtual void                   SetInteracting( TINT a_iUnk ) {}                                                                                   // +0x11c, DOES_NOTHING 00628f40, called when the interaction ends
	virtual TBOOL                  IsInteracting() { return TFALSE; }                                                                                 // +0x120, RETURNS_FALSE 00660960
	virtual void                   SetIgnoreInvalidPosition( TINT a_iUnk ) {}                                                                         // +0x124, DOES_NOTHING 00628f40
	virtual TBOOL                  CanStartAction() { return TTRUE; }                                                                                 // +0x128, RETURNS_TRUE 0046cde0
	virtual TBOOL                  IsIgnoringInvalidPosition() { return TFALSE; }                                                                     // +0x12c, RETURNS_FALSE 00660960, lets Seek/SeekFacing run
	virtual void                   ResumeMovement() {}                                                                                                // +0x130, DOES_NOTHING 005acd80
	virtual TBOOL                  CanSeek( const Toshi::TVector4& a_rcTarget ) { return TTRUE; }                                                     // +0x134, RETURNS_TRUE 00413920
	virtual TBOOL                  Halt();                                                                                                            // +0x138
	virtual TBOOL                  HaltAtPosition( const Toshi::TVector3& a_rcPosition );                                                             // +0x13c
	virtual TBOOL                  HaltFacing( const Toshi::TVector3& a_rcFacing );                                                                   // +0x140
	virtual TBOOL                  Seek( const Toshi::TVector4& a_rcTarget, TFLOAT a_fSeekOffset );                                                   // +0x144
	virtual TBOOL                  SeekFacing( const Toshi::TVector4& a_rcTarget, const Toshi::TVector4& a_rcFacing );                                // +0x148
	virtual TBOOL                  HaltFacingDesired( const Toshi::TVector3& a_rcFacing );                                                            // +0x14c
	virtual TBOOL                  IsSeekingConstant();                                                                                               // +0x150
	virtual TBOOL                  IsFacingAtDesiredFacing();                                                                                         // +0x154
	virtual TBOOL                  GetSeekTarget( Toshi::TVector3& a_rOutTarget );                                                                    // +0x158
	virtual void                   Unk_0x15c( TINT a_iUnk ) {}                                                                                        // +0x15c, DOES_NOTHING 00628f40
	virtual TFLOAT                 GetRadius();                                                                                                       // +0x160
	virtual TBOOL                  GetTilt( void* a_pPitch, void* a_pRoll ) { return TFALSE; }                                                        // +0x164, RETURNS_ZERO 00556a00
	virtual void                   SetTilt( TFLOAT a_fPitch, TFLOAT a_fRoll, TBOOL a_bImmediate ) {}                                                  // +0x168, DOES_NOTHING 00413930, sets the tilt of the model
	virtual TINT                   GetMaxLOD() { return 0; }                                                                                          // +0x16c, RETURNS_ZERO 0054d8b0
	virtual void                   ChangedLOD( TINT a_iLOD ) {}                                                                                       // +0x170, DOES_NOTHING 00628f40

	// FSM natives
	void   SetToCurrentFullDesiredSpeed();
	TFLOAT GetPositionX();
	TFLOAT GetPositionY();
	TFLOAT GetPositionZ();
	void   SetPositionXYZ( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ );
	void   SetFacingFromPartsXYZ( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ );

	AUnit&              GetParentUnit() { return *m_oOwner->GetUnit(); }
	const ASteerTraits* GetTraits() const { return m_pTraits; }
	TFLOAT              GetSpeedModifier() const { return m_fSpeedModifier; }
	void                SetSpeedModifier( TFLOAT a_fModifier ) { m_fSpeedModifier = a_fModifier; }

protected:
	// Clamps a_riLOD to GetMaxLOD(). Inlined in retail.
	void ValidateLOD( TINT& a_riLOD );

public:
	static AFunctionTable ms_oFunctionTable;

protected:
	Toshi::TRef<AUnitProxy> m_oOwner;         // 0x04
	TFLOAT                  m_fSpeedModifier; // 0x08, = 1.0f, see SetToCurrentFullDesiredSpeed
	const ASteerTraits*     m_pTraits;        // 0x0c, owner's static data (AUnit::GetSteerTraits)
	TBOOL                   m_bUnk10;         // 0x10
	Movement                m_eMovementType;  // 0x14
};

TSTATICASSERT( sizeof( ASteer ) == 0x18 );
