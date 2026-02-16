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

protected:
	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void RemoveHash();

};
