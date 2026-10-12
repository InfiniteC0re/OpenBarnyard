#pragma once
#include "AUnit.h"
#include "Animation/AModel.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateGoalInterface.h"

#include <Toshi/TEvent.h>
#include <Toshi/TNodeList.h>
#include <Math/TSphere.h>

class ACollisionObject;

// A unit with a visual (model instance). Subclasses (AAnimatedUnit) own the
// model; AVisibleUnit listens to its change events, keeps the unit's steer on
// its interior goal and owns the unit's collision object and looping sounds.
//
// Hierarchy (TClass initialisers 0x00706a22..0x00706dc5):
//   AUnit <- AVisibleUnit <- AAnimatedUnit <- ACharacterUnit
//                                          <- AUnitPlayer
class AVisibleUnit
    : public AUnit
    , public AGoalCollisionUpdateGoalInterface
{
public:
	TDECLARE_CLASS( AVisibleUnit, AUnit );

	using ModelInstanceListener = Toshi::TListener<AModelInstance, AModelInstance::ChangeEvent, AVisibleUnit>;

	// Node of m_oLoopSounds (0x10 bytes)
	struct LoopSound : public Toshi::TNodeList<LoopSound>::TNode
	{
		TINT iCueIndex; // 0x0c, ASoundManager cue
	};

public:
	AVisibleUnit();
	~AVisibleUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() override;                              // +0x0c
	virtual void            SetSimLOD( TINT a_iLOD ) override;                        // +0x10, sets m_iLOD and tells the steer (SetLOD)
	virtual TINT            GetSimLOD() override;                                     // +0x14, returns m_iLOD
	virtual void            SetSize( TFLOAT a_fSize ) override = 0;                   // +0x28, AAnimatedUnit (00416900) scales the model
	virtual TBOOL           Create( AUnitTransform* a_pTransform ) override;          // +0x58
	virtual void            OnDestroy() override;                                     // +0x60
	virtual void            UpdateAI( TFLOAT a_fDeltaTime ) override;                 // +0x68
	virtual TBOOL           Think( TFLOAT a_fDeltaTime ) override;                    // +0x6c
	virtual void            UpdateComponents( TFLOAT a_fDeltaTime ) override;         // +0x74
	virtual void            OnComponentAdded( AUnitComponent* a_pComp ) override {}   // +0x88, DOES_NOTHING 006c6060
	virtual void            OnComponentRemoved( AUnitComponent* a_pComp ) override {} // +0x8c, DOES_NOTHING 006c6060

	//-----------------------------------------------------------------------------
	// AGoalCollisionUpdateInterface (secondary vtable 0x00717e4c, at +0x40)
	//-----------------------------------------------------------------------------
	// Moves the steer onto the goal of a_pInterface, raised by m_fGoalYOffset.
	virtual void OnGoalCollisionUpdate( AGoalCollisionUpdateInterface* a_pInterface ) override; // +0x04

	//-----------------------------------------------------------------------------
	// Own virtuals (vtable 0x00717e58)
	//-----------------------------------------------------------------------------
	virtual void            SetVisible( TBOOL a_bVisible ) = 0;                                                               // +0x90, FSM "SetVisible"
	virtual TBOOL           IsVisible()                    = 0;                                                               // +0x94, FSM "IsVisible"
	virtual TBOOL           IsInCameraFrustum();                                                                              // +0x98, FSM "IsInCameraFrustum"
	virtual TBOOL           IsInCameraFrustumDistance( TFLOAT a_fDistance );                                                  // +0x9c, FSM "IsInCameraFrustumDistance"
	virtual void            SetOpacity( TFLOAT a_fOpacity );                                                                  // +0xa0
	virtual void            UpdateBoundingSphere() = 0;                                                                       // +0xa4, AAnimatedUnit (00416850) fills m_oBoundingSphere
	virtual AModelInstance* GetModelInstance()     = 0;                                                                       // +0xa8
	virtual void            ConnectModelInstance( AModelInstance* a_pModelInstance );                                         // +0xac, (re)connects m_oModelInstanceListener
	virtual TBOOL           OnModelInstanceChange( AModelInstance* a_pModelInstance, AModelInstance::ChangeEvent* a_pEvent ); // +0xb0

	// Stops every looping sound cue of this unit.
	void StopAllLoopSounds();

	AGoalCollisionUpdateInterface* GetGoalInterface() { return this; }
	void                           SetGoalYOffset( TFLOAT a_fOffset ) { m_fGoalYOffset = a_fOffset; }
	const Toshi::TSphere&          GetBoundingSphere() const { return m_oBoundingSphere; }
	ACollisionObject*              GetCollisionObject() const { return m_pCollisionObject; }

public:
	static AFunctionTable ms_oFunctionTable;

private:
	static TBOOL __stdcall OnModelInstanceChangeCallback( AVisibleUnit* a_pCaller, AModelInstance* a_pModelInstance, AModelInstance::ChangeEvent* a_pEvent );

	// Broadcast model instance changes to the components (component vtable +0x24 / +0x20).
	void NotifyComponentsVisibility( TBOOL a_bVisible );
	void NotifyComponentsSkeletonUpdate( TBOOL a_bUpdating );

protected:
	// 0x40: AGoalCollisionUpdateGoalInterface
	ModelInstanceListener       m_oModelInstanceListener; // 0x4c
	Toshi::TSphere              m_oBoundingSphere;        // 0x60, not initialised by the ctor
	ACollisionObject*           m_pCollisionObject;       // 0x70
	TFLOAT                      m_fThinkTimer;            // 0x74, far-away interior steers think every 1.5s
	TFLOAT                      m_fGoalYOffset;           // 0x78, set by AUnitManager::SpawnUnitAtGoal
	Toshi::TNodeList<LoopSound> m_oLoopSounds;            // 0x7c
	TINT                        m_iLOD;                   // 0x8c, passed to the steer
	TINT                        m_iUnk90;                 // 0x90, not touched by AVisibleUnit itself
};

TSTATICASSERT( sizeof( AVisibleUnit ) == 0x94 );
