#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TClass.h>

#include "AInteractionActivityMessage.h"

//-----------------------------------------------------------------------------
// Fixed pool of the messages of one AInteractionActivityMessage subclass, see
// AInteractionActivityMessageGroups.
//-----------------------------------------------------------------------------
class AInteractionActivityMessageGroupBase
    : public Toshi::TNodeList<AInteractionActivityMessageGroupBase>::TNode
{
public:
	AInteractionActivityMessageGroupBase( const Toshi::TClass* a_pClass )
	    : m_pClass( a_pClass )
	{}

	virtual ~AInteractionActivityMessageGroupBase();                                          // +0x00
	virtual AInteractionActivityMessage* Allocate()                                      = 0; // +0x04, TNULL if there are no free messages
	virtual void                         Free( AInteractionActivityMessage* a_pMessage ) = 0; // +0x08

	const Toshi::TClass* GetClass() const { return m_pClass; }

protected:
	const Toshi::TClass* m_pClass; // 0x10, class of the messages
};

TSTATICASSERT( sizeof( AInteractionActivityMessageGroupBase ) == 0x14 );
