#pragma once
#include "AUnitComponent.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>

class AAnimatableObject;
class AModelAttachComponentData;

// Owns the animatable objects listed in the unit type's AModelAttachComponentData and
// attaches them to (or detaches them from) the unit's model on request (FSM natives
// "AttachModel" / "DetachModel", AAttachActivity "attachmodel" / "detachmodel").
//
// TClass 0x0078a1b0 (version 1.0, size 0x20, runtime creatable), vtable 0x007288f0.
class AModelAttachComponent : public AUnitComponent
{
public:
	TDECLARE_CLASS( AModelAttachComponent, AUnitComponent );

public:
	AModelAttachComponent();
	~AModelAttachComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AFunctionTable*        GetFunctionTable() OVERRIDE; // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;       // +0x10

	// Creates an animatable object for every model of a_pData, owned by a_pUnit
	TBOOL CreateModels( AUnit* a_pUnit, AModelAttachComponentData* a_pData );

	// Attaches the model of the type a_strModel to the bone a_strParentBone of the unit's model (by its bone a_strChildBone)
	void AttachModel( Toshi::TPString8 a_strModel, Toshi::TPString8 a_strParentBone, Toshi::TPString8 a_strChildBone );

	// Detaches the model of the type a_strModel from the unit's model
	void DetachModel( Toshi::TPString8 a_strModel );

private:
	// Index of the model of the type a_rcModel, -1 if there's none
	// NOTE: inlined into AttachModel and DetachModel
	TINT FindModel( const Toshi::TPString8& a_rcModel ) const;

public:
	static AFunctionTable ms_oFunctionTable; // 0x0078a1e0

private:
	// NOTE: retail initialises the grow size to 0
	Toshi::TArray<AAnimatableObject*> m_aModels; // 0x10
};

TSTATICASSERT( sizeof( AModelAttachComponent ) == 0x20 );
