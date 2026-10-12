#pragma once
#include "AInteractionActivityMessage.h"

class AKinematicsObstruction;

//-----------------------------------------------------------------------------
// Activity message that keeps the obstruction an interaction puts around its
// units (an AKinematicsObstructionInteraction from AKinematicsObstructionManager).
//-----------------------------------------------------------------------------
class AKinematicsObstructionActivityMessage
    : public AInteractionActivityMessage
{
public:
	TDECLARE_CLASS( AKinematicsObstructionActivityMessage, AInteractionActivityMessage );

public:
	AKinematicsObstructionActivityMessage();
	~AKinematicsObstructionActivityMessage();

	virtual void Reset() OVERRIDE;                                                     // +0x0c
	virtual void CreateGroup( AInteractionActivityMessageGroups* a_pGroups ) OVERRIDE; // +0x20

	// Creates the obstruction (an AKinematicsObstructionInteraction) around the units of the sequence
	void CreateObstruction();

	// Removes and destroys the obstruction if there is one.
	void ReleaseObstruction();

private:
	AKinematicsObstruction* m_pObstruction; // 0x18
};

TSTATICASSERT( sizeof( AKinematicsObstructionActivityMessage ) == 0x1c );
