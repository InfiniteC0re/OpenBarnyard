#pragma once
#include "TMemory.h"
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

class TMemoryInitialiser
{
public:
	// $Barnyard: FUNCTION 006bfca0
	TMemoryInitialiser()
	{
		Initialise();
	}

	// $Barnyard: FUNCTION 006bfcb0
	~TMemoryInitialiser()
	{
		Deinitialise();
	}

	TINT Initialise()
	{
		TMemory::Initialise( 0, 0 );
		return 0;
	}

	void Deinitialise()
	{
		TMemory::Deinitialise();
	}
};

TOSHI_NAMESPACE_END
