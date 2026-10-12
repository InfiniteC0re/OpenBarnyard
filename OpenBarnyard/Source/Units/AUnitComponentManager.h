#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TDList.h>

class AUnitComponent;

// Owns and updates one kind of AUnitComponent. All managers live in the
// AUnitComponentManagerList owned by AUnitManager.
class AUnitComponentManager
    : public Toshi::TObject
    , public Toshi::TDList<AUnitComponentManager>::TNode
{
public:
	TDECLARE_CLASS( AUnitComponentManager, Toshi::TObject );

public:
	AUnitComponentManager();
	~AUnitComponentManager();

	virtual void Update( TFLOAT a_fDeltaTime )               = 0; // +0x0c
	virtual void ReleaseComponent( AUnitComponent* a_pComp ) = 0; // +0x10
};

// The set of component managers, updated by AUnitManager on its global cadence.
class AUnitComponentManagerList : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AUnitComponentManagerList, Toshi::TObject );

public:
	AUnitComponentManagerList();
	~AUnitComponentManagerList();

	void Update( TFLOAT a_fDeltaTime );

private:
	Toshi::TDList<AUnitComponentManager> m_llManagers; // 0x04
};
