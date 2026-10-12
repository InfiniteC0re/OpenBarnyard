#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/T2SList.h>
#include <Math/TVector4.h>

#include "AUnitProxy.h"
#include "AUnitHash.h"

class AUnitType;
class AUnitComponent;
class AUnitTransform;
class AFunctionTable;
class ASteer;
class ASteerTraits;
class ACarryableUnitHelper;

// Base class for every spawnable unit. It lives in AUnitManager's unit list and
// publishes itself to the AUIDMap through a self-handle.
//
// NOTE: retail hands units around as TManagedPtr<AUnitProxy, AUnitProxyGetFunctor>
// (Wii/PS2 strings); Toshi::TRef<AUnitProxy> stands in for it until that exists.
class AUnit
    : public Toshi::TObject
    , public Toshi::TNodeList<AUnit>::TNode
{
public:
	TDECLARE_CLASS( AUnit, Toshi::TObject );

	friend class AUnitFactory;
	friend class AUnitManager;

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_DYING           = BITFLAG( 0 ), // tested across the manager's update passes
		FLAGS_HAS_INTERACTION = BITFLAG( 1 ), // 0x02, FSM "HasInteraction"
		FLAGS_AI_ENABLED      = BITFLAG( 2 ), // 0x04, included in the AI pass
		FLAGS_THINK_ENABLED   = BITFLAG( 3 ), // 0x08, included in the think pass (FSM "SetMovement")
		FLAGS_EXTERNAL_UPDATE = BITFLAG( 4 ), // 0x10, updated through AUnitManager::Update instead of the per-frame pass
		FLAGS_NO_QUERY        = BITFLAG( 5 ), // 0x20, skipped by AUnitHash queries
		FLAGS_HASHED          = BITFLAG( 6 ), // 0x40, set in ctor; the unit lives in the AUnitHash
		FLAGS_PENDING_REMOVAL = BITFLAG( 7 ), // 0x80, recycled/destroyed by AUnitManager::ProcessPendingUnits
		FLAGS_MILK_TARGET     = BITFLAG( 9 ), // 0x200, the unit can be aimed at with the milk reticle (AMilkReticleManager, AAimedSquirtProjectileLauncher)
	};

public:
	AUnit();
	virtual ~AUnit();

	//-----------------------------------------------------------------------------
	// Virtual table (slots 3..35). Named where understood (FSM bindings give some
	// of the names); the rest are kept as Unk_0xNN so every slot lands at its real
	// vtable offset.
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*     GetFunctionTable();                                              // +0x0c
	virtual void                SetSimLOD( TINT a_iUnk ) {}                                      // +0x10, DOES_NOTHING
	virtual TINT                GetSimLOD() { return 0; }                                        // +0x14, RETURNS_ZERO, passed to the steer's Create
	virtual TBOOL               SetMovement( TBOOL a_bEnabled );                                 // +0x18, toggles FLAGS_THINK_ENABLED, returns the old state
	virtual void                Update( TFLOAT a_fDeltaTime );                                   // +0x1c
	virtual const ASteerTraits* GetSteerTraits();                                                // +0x20, the type's ASteerTraits or a shared default
	virtual void                OnEnterSpecialPathArea();                                        // +0x24, steer related
	virtual void                SetSize( TFLOAT a_fSize ) {}                                     // +0x28, DOES_NOTHING
	virtual void                SetQueryStamp( TUINT a_uiStamp ) { m_uiQueryStamp = a_uiStamp; } // +0x2c, see AUnitHash
	virtual TUINT               GetQueryStamp() { return m_uiQueryStamp; }                       // +0x30
	virtual TINT                SetUnk34( TINT a_iValue );                                       // +0x34, returns the old value
	// $Barnyard: FUNCTION 00413910
	virtual TBOOL                 IsUnk34Set() { return m_iUnk34 != 0; }                  // +0x38
	virtual TBOOL                 DoesHaveUnitCountIdentifier() { return TFALSE; }        // +0x3c, RETURNS_FALSE
	virtual void                  SetUnitCountIdentifier( TUINT a_uiIndex ) {}            // +0x40, DOES_NOTHING
	virtual TUINT                 GetUnitCountIdentifier() { return 1; }                  // +0x44
	virtual TUINT                 GetUnitCountMaximum();                                  // +0x48, type's instance count
	virtual TBOOL                 Unk_0x4c() { return TFALSE; }                           // +0x4c, RETURNS_FALSE
	virtual void                  Unk_0x50( TINT a_iUnk ) {}                              // +0x50, DOES_NOTHING
	virtual ACarryableUnitHelper* GetCarryableHelper() { return TNULL; }                  // +0x54, RETURNS_ZERO, ACarryableUnit returns its helper
	virtual TBOOL                 Create( AUnitTransform* a_pTransform );                 // +0x58, builds steer + static data
	virtual void                  OnPostCreate() {}                                       // +0x5c, DOES_NOTHING
	virtual void                  OnDestroy();                                            // +0x60, tears down steer + static data
	virtual void                  OnLODChange() {}                                        // +0x64, DOES_NOTHING
	virtual void                  UpdateAI( TFLOAT a_fDeltaTime );                        // +0x68, walks m_pComponents
	virtual TBOOL                 Think( TFLOAT a_fDeltaTime ) { return TTRUE; }          // +0x6c, RETURNS_TRUE
	virtual void                  OnUpdate( TFLOAT a_fDeltaTime ) {}                      // +0x70, DOES_NOTHING
	virtual void                  UpdateComponents( TFLOAT a_fDeltaTime );                // +0x74, broadcasts to components
	virtual const TWCHAR*         GetScreenName();                                        // +0x78
	virtual void                  Unk_0x7c( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ) {} // +0x7c, DOES_NOTHING
	virtual Toshi::TClass*        GetSteerClass();                                        // +0x80, steer class from type name
	virtual void                  Unk_0x84() {}                                           // +0x84, DOES_NOTHING
	virtual void                  OnComponentAdded( AUnitComponent* a_pComp ) {}          // +0x88, DOES_NOTHING
	virtual void                  OnComponentRemoved( AUnitComponent* a_pComp ) {}        // +0x8c, DOES_NOTHING, called per component before teardown

	TBOOL ChangeSteer( Toshi::TClass* a_pSteerClass );
	void  SetSteerPosition( const Toshi::TVector4& a_rcPosition );
	void  SetSteerDirection( const Toshi::TVector4& a_rcDirection );

	// FSM natives
	TBOOL  CanPerceiveUnit( AUnit* a_pOther, TFLOAT a_fRange, TFLOAT a_fMinDot );
	TBOOL  CanPerceiveUnitMe( AUnit* a_pOther, TFLOAT a_fRange, TFLOAT a_fMinDot );
	TFLOAT GetDistanceSqToUnit( AUnit* a_pOther );
	TBOOL  HasInteraction() const;

	void  SetHasInteraction( TINT a_iValue );
	TBOOL CanInteract();
	TBOOL IsPlayer() const;

	void            AddComponent( AUnitComponent* a_pComponent );
	AUnitComponent* FindComponent( Toshi::TClass* a_pClass );
	AUnitComponent* FindComponentExact( Toshi::TClass* a_pClass );

	// Hands every component back to its manager (or deletes it).
	void DestroyComponents();

	// Kills particle effects attached to this unit.
	void KillAttachedEffects();

	void SetNoQuery( TBOOL a_bNoQuery );
	void SetHashed( TBOOL a_bHashed );
	void SetAIEnabled( TBOOL a_bEnabled );
	void SetExternalUpdate( TBOOL a_bExternal );
	void SetPendingRemoval( TBOOL a_bPending );

	Toshi::TRef<AUnitProxy> GetSelfProxy() const;

	TINT                   GetId() const { return m_iId; }
	AUnitHash::CellObject* GetHashNode() const { return m_pHashNode; }
	void                   SetHashNode( AUnitHash::CellObject* a_pNode ) { m_pHashNode = a_pNode; }
	ASteer*                GetSteer() const { return m_pSteer; }
	Toshi::TObject*        GetObstruction() const { return m_pObstruction; }

	// Sets a_uiFlags (AKinematicsObstructionUnit::FLAGS) on the obstruction m_pObstruction of the unit
	void       SetObstructionFlags( TUINT8 a_uiFlags );
	AUnitType* GetType() const { return m_pType; }
	FLAGS      GetFlags() const { return m_eFlags; }
	void       AddFlags( FLAGS a_eFlags ) { m_eFlags |= a_eFlags; }
	void       RemoveFlags( FLAGS a_eFlags ) { m_eFlags &= ~a_eFlags; }
	TBOOL      IsDying() const { return ( m_eFlags & FLAGS_DYING ) != 0; }
	void       SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

public:
	static AFunctionTable ms_oFunctionTable;

protected:
	Toshi::TObject*                 m_pObstruction; // 0x10, owned, deleted in OnDestroy
	FLAGS                           m_eFlags;       // 0x14
	Toshi::TRef<AUnitProxy>         m_oSelfProxy;   // 0x18, this unit's id handle
	ASteer*                         m_pSteer;       // 0x1c, created from GetSteerClass()
	TBOOL                           m_bUnk20;       // 0x20
	TINT                            m_iId;          // 0x24, assigned by AUnitManager
	AUnitType*                      m_pType;        // 0x28
	Toshi::T2SList<AUnitComponent>* m_pComponents;  // 0x2c
	AUnitHash::CellObject*          m_pHashNode;    // 0x30, cell in AUnitManager's AUnitHash
	TINT                            m_iUnk34;       // 0x34, =1
	TUINT                           m_uiQueryStamp; // 0x38, last AUnitHash query that visited this unit
	TUINT                           m_iUnk3c;       // 0x3c
};
