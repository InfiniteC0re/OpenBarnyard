#pragma once
#include "AUnitStaticData.h"

// Static data of a unit type's interaction component ("AInteractionComponentData" block).
// Creates an AInteractionComponent for every unit of the type.
class AInteractionComponentData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( AInteractionComponentData, AUnitStaticData );

public:
	AInteractionComponentData();
	~AInteractionComponentData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual void  OnUnitCreate( AUnit* a_pUnit ) OVERRIDE;              // +0x10

private:
	TBOOL m_bSpontaneousInteractions; // 0x0c, "spontaneousinteractions"
	TBOOL m_bAllowConversation;       // 0x0d, "allowconversation"
};

TSTATICASSERT( sizeof( AInteractionComponentData ) == 0x10 );
