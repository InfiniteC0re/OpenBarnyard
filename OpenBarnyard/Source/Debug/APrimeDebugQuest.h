#pragma once
#include <PrimeDebug.h>

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Purpose: the story commands of the PrimeDebug console (story.goto,
// story.advance, story.reset) and the Quests submenu of the debug menu.
// NOTE: not in retail.
//-----------------------------------------------------------------------------
class APrimeDebugQuest
{
public:
	static void Register();
	static void Unregister();

	// Updates the story.info variables
	static void Update( TFLOAT a_fDeltaTime );
};

#endif // PRIMEDEBUG_DEV
