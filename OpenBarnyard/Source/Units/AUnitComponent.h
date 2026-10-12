#pragma once
#include <Toshi/TObject.h>
#include <Toshi/T2SList.h>
#include <Toshi/TRefCounted.h>

#include "AUnitProxy.h"
#include "GameInterface/AHackedQuickSave.h"

class AFunctionTable;
class AFunctionCaller;
class AUnit;
class AUnitComponentManager;

// FSM fallback of the component tables (0041b1e0), shared by the subclasses' tables
Toshi::TObject* __stdcall AUnitComponentFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller );

// Base for every behaviour attached to an AUnit (kept in AUnit::m_pComponents).
// Concrete components are owned by an AUnitComponentManager, which takes them
// back when the unit is destroyed.
class AUnitComponent
    : public Toshi::TObject
    , public Toshi::T2SList<AUnitComponent>::Node
{
public:
	TDECLARE_CLASS( AUnitComponent, Toshi::TObject );

public:
	AUnitComponent();
	~AUnitComponent();

	virtual AFunctionTable*        GetFunctionTable();                            // +0x0c
	virtual AUnitComponentManager* GetManager() = 0;                              // +0x10, TNULL if the component owns itself
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) {} // +0x14, DOES_NOTHING
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) {} // +0x18, DOES_NOTHING
	virtual void                   Unk_0x1c() {}                                  // +0x1c, DOES_NOTHING
	virtual void                   OnSkeletonUpdateChanged( TINT a_iUnk ) {}      // +0x20, DOES_NOTHING
	virtual void                   OnVisibilityChanged( TINT a_iUnk ) {}          // +0x24, DOES_NOTHING
	virtual void                   OnHasInteractionChanged( TINT a_iUnk ) {}      // +0x28, DOES_NOTHING
	virtual void                   UpdateAI( TFLOAT a_fDeltaTime ) {}             // +0x2c, DOES_NOTHING, called by AUnit::UpdateAI
	virtual void                   Update( TFLOAT a_fDeltaTime ) {}               // +0x30, DOES_NOTHING, called by AUnit::UpdateComponents
	virtual void                   Unk_0x34( TINT a_iUnk ) {}                     // +0x34, DOES_NOTHING

	// Points m_oOwner at a_pUnit's proxy (or clears it for TNULL).
	void   SetOwner( AUnit* a_pUnit );
	AUnit* GetOwner();

	TBOOL IsAIEnabled() const { return m_bAIEnabled; }

	// Sets the flag without calling OnEnable/OnDisable (see AControlComponent::SetEnabled)
	void SetAIEnabledFlag( TBOOL a_bEnabled ) { m_bAIEnabled = a_bEnabled; }

public:
	static AFunctionTable ms_oFunctionTable;

protected:
	Toshi::TRef<AUnitProxy> m_oOwner;     // 0x08, the owning unit's proxy
	TBOOL                   m_bAIEnabled; // 0x0c
};
