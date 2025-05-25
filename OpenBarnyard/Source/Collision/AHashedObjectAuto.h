#pragma once
#include "AHashedObject.h"

//-----------------------------------------------------------------------------
// Purpose: A wrapper for AHashedObject that makes hash management automated.
//-----------------------------------------------------------------------------
class AHashedObjectAuto
    : public AHashedObject
{
public:
	AHashedObjectAuto();
	~AHashedObjectAuto();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Remove();

};
