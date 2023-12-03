#pragma once
#include "THookedObject.h"
#include "SDKHooks.h"

#include <Toshi/Core/TClass.h>
#include <Toshi/Core/TRefCounted.h>
#include <Toshi/Core/TNodeTree.h>

TOBJECT(THookedTask, Toshi::TObject, TTRUE),
	public Toshi::TRefCounted,
	public Toshi::TNodeTree<THookedTask>::TNode
{
public:
	THookedTask()
	{
		CALL_THIS(0x006bc910, THookedTask*, THookedTask*, this);
	}

	virtual ~THookedTask() {}

	virtual TBOOL Create() { return TTRUE; }
	virtual TBOOL CreateFailed() { return TTRUE; }
	virtual TBOOL Reset() { return TTRUE; }
	virtual TBOOL OnCreate() { return TTRUE; }
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) { return TTRUE; }
	virtual void OnPreDestroy() {}
	virtual void OnDestroy() {}
	virtual TBOOL OnChildDying(THookedTask* child) { return TTRUE; }
	virtual void OnChildDied(Toshi::TClass* a_pClass, THookedTask* a_pDeletedTask) {}
	virtual void OnActivate() {}
	virtual void OnDeactivate() {}

private:
	TUINT32 m_State;
};
