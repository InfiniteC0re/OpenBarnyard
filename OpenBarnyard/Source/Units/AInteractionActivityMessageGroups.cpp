#include "pch.h"
#include "AInteractionActivityMessageGroups.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005a0230
AInteractionActivityMessageGroups::AInteractionActivityMessageGroups()
{
	TGetClass( AInteractionActivityMessage ).RecurseTree( TNULL, TNULL, CreateGroupCallback, this );
}

// $Barnyard: FUNCTION 005a0160
TBOOL AInteractionActivityMessageGroups::CreateGroupCallback( TClass* a_pClass, void* a_pGroups )
{
	if ( a_pClass != &TGetClass( AInteractionActivityMessage ) )
	{
		// Create a temporary message to let it add the group of its class
		AInteractionActivityMessage* pMessage = TSTATICCAST( AInteractionActivityMessage, a_pClass->CreateObject() );
		pMessage->CreateGroup( TSTATICCAST( AInteractionActivityMessageGroups, a_pGroups ) );
		delete pMessage;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005a01e0
AInteractionActivityMessageGroupBase* AInteractionActivityMessageGroups::Find( const TClass* a_pClass )
{
	for ( auto it = Begin(); it != End(); it++ )
	{
		if ( it->GetClass() == a_pClass )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005a02a0
AInteractionActivityMessage* AInteractionActivityMessageGroups::Allocate( const TClass* a_pClass )
{
	return Find( a_pClass )->Allocate();
}

// $Barnyard: FUNCTION 005a02c0
void AInteractionActivityMessageGroups::Free( AInteractionActivityMessage* a_pMessage )
{
	AInteractionActivityMessageGroupBase* pGroup = Find( a_pMessage->GetClass() );

	a_pMessage->Reset();
	pGroup->Free( a_pMessage );
}
