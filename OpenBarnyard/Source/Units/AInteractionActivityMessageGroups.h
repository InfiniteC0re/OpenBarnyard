#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TClass.h>

#include "AInteractionActivityMessageGroup.h"

//-----------------------------------------------------------------------------
// The groups of every AInteractionActivityMessage subclass, owned by
// AInteractionManager (+0xf1c).
//-----------------------------------------------------------------------------
class AInteractionActivityMessageGroups
    : public Toshi::TNodeList<AInteractionActivityMessageGroupBase>
{
public:
	// Lets every message class create its group.
	AInteractionActivityMessageGroups();

	AInteractionActivityMessageGroupBase* Find( const Toshi::TClass* a_pClass );

	// Takes a message of class a_pClass from its group.
	AInteractionActivityMessage* Allocate( const Toshi::TClass* a_pClass );

	// Resets the message and returns it to its group.
	void Free( AInteractionActivityMessage* a_pMessage );

	// Adds a group for the messages of class T if there is none, for T::CreateGroup.
	template <class T>
	void CreateGroup()
	{
		if ( !Find( &TGetClass( T ) ) )
			InsertTail( new AInteractionActivityMessageGroup<T>() );
	}

private:
	static TBOOL CreateGroupCallback( Toshi::TClass* a_pClass, void* a_pGroups );
};

TSTATICASSERT( sizeof( AInteractionActivityMessageGroups ) == 0x10 );
