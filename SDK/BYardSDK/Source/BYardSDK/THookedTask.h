#pragma once
#include "THookedObject.h"
#include "SDKHooks.h"

#include <Toshi/TClass.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TNodeTree.h>

class THookedTask : public Toshi::TObject
    , public Toshi::TRefCounted
    , public Toshi::TNodeTree<THookedTask>::TNode
{
public:
	TDECLARE_CLASS( THookedTask, Toshi::TObject );

public:
	THookedTask()
	{
		CALL_THIS( 0x006bc910, THookedTask*, THookedTask*, this );
	}

	virtual ~THookedTask() {}

	virtual TBOOL Create() { return TTRUE; }
	virtual TBOOL CreateFailed() { return TTRUE; }
	virtual TBOOL Reset() { return TTRUE; }
	virtual TBOOL OnCreate() { return TTRUE; }
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) { return TTRUE; }
	virtual void  OnPreDestroy() {}
	virtual void  OnDestroy() {}
	virtual TBOOL OnChildDying( THookedTask* child ) { return TTRUE; }
	virtual void  OnChildDied( Toshi::TClass* a_pClass, THookedTask* a_pDeletedTask ) {}
	virtual void  OnActivate() {}
	virtual void  OnDeactivate() {}

private:
	TUINT32 m_State;
};
