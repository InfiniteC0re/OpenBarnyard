#pragma once
#include "AUnitStaticData.h"

#include <Toshi/TPString8.h>

// Gives every unit of a type an AUnitBehaviour that starts with the "defaultfsm"
// ("AUnitBehaviourData" block).
//
// TClass 0x0078a3c8 (version 0.1, size 0x10), vtable 0x00728cd4.
class AUnitBehaviourData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AUnitBehaviourData, AUnitStaticData );

public:
	// NOTE: inlined into CreateObject 00593770 / CreateObjectInPlace 005932e0
	AUnitBehaviourData() {}

	~AUnitBehaviourData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

private:
	Toshi::TPString8 m_strDefaultFSM; // 0x0c, "defaultfsm"
};

TSTATICASSERT( sizeof( AUnitBehaviourData ) == 0x10 );
