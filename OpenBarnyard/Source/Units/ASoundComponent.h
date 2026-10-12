#pragma once
#include "AUnitComponent.h"
#include "AUnitStaticData.h"
#include "Sound/ASoundLegacy.h"

//-----------------------------------------------------------------------------
// Lets the FSM scripts of a unit play random sounds at its position
// ("PlaySound" native), without overlapping and not too often.
//-----------------------------------------------------------------------------
class ASoundComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( ASoundComponent, AUnitComponent );

	friend class ASoundComponentData;

public:
	ASoundComponent();
	~ASoundComponent();

	virtual AFunctionTable*        GetFunctionTable() OVERRIDE; // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;       // +0x10

	// Plays a_iSound at the position of the owner if the previous one has finished, at least
	// a_fMinInterval seconds have passed and a random roll (more likely the longer it's been) is below a_fChance
	void PlaySound( ASoundId a_iSound, TFLOAT a_fChance, TFLOAT a_fMinInterval );

public:
	static AFunctionTable ms_oFunctionTable; // DAT_0078c3bc

private:
	ASoundComponentData* m_pStaticData;   // 0x10
	TINT                 m_iCueIndex;     // 0x14
	TFLOAT               m_fLastPlayTime; // 0x18
	TFLOAT               m_fTimeBudget;   // 0x1c, every call adds the time since the last sound, every sound takes 0.25s
};

TSTATICASSERT( sizeof( ASoundComponent ) == 0x20 );

//-----------------------------------------------------------------------------
// Static data of a unit type ("ASoundComponentData" block, no properties).
// Creates an ASoundComponent for every unit of the type.
//-----------------------------------------------------------------------------
class ASoundComponentData
    : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ASoundComponentData, AUnitStaticData );

public:
	ASoundComponentData();
	~ASoundComponentData();

	virtual void OnUnitCreate( AUnit* a_pUnit ) OVERRIDE; // +0x10
};

TSTATICASSERT( sizeof( ASoundComponentData ) == 0x0C );
