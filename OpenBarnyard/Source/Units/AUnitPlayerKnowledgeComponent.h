#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"

class AUnitPlayerKnowledgeComponentData;

//-----------------------------------------------------------------------------
// What a unit knows about the player: one bit for every knowledge registered in
// APlayerKnowledgeManager (set by APropagateKnowledgeActivity, checked by the
// knowledge requirement of the interactions).
// TClass 0x0078c32c (version 1.0, size 0x18, runtime creatable), vtable 0x0072d378.
//-----------------------------------------------------------------------------
class AUnitPlayerKnowledgeComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AUnitPlayerKnowledgeComponent, AUnitComponent );

	friend class AUnitPlayerKnowledgeComponentData;

public:
	AUnitPlayerKnowledgeComponent();
	~AUnitPlayerKnowledgeComponent();

	virtual AUnitComponentManager* GetManager() OVERRIDE; // +0x10

	// Sets the bit a_iKnowledge (index in APlayerKnowledgeManager)
	void SetKnowledge( TINT a_iKnowledge );

	// TRUE if the bit a_iKnowledge is set
	TBOOL HasKnowledge( TINT a_iKnowledge );

private:
	AUnitPlayerKnowledgeComponentData* m_pData;       // 0x10, not initialised by the constructor
	TUINT32                            m_uiKnowledge; // 0x14
};

TSTATICASSERT( sizeof( AUnitPlayerKnowledgeComponent ) == 0x18 );

//-----------------------------------------------------------------------------
// Static data that gives every unit of the type an AUnitPlayerKnowledgeComponent.
// TClass version 1.0, size 0xc, vtable 0x0072d360.
//-----------------------------------------------------------------------------
class AUnitPlayerKnowledgeComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitPlayerKnowledgeComponentData, AUnitStaticData );

public:
	AUnitPlayerKnowledgeComponentData();
	~AUnitPlayerKnowledgeComponentData();

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( AUnitPlayerKnowledgeComponentData ) == 0xc );
