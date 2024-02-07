#include "ToshiPCH.h"
#include "TResource.h"
#include "TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS(TResource);

	TResource::~TResource()
	{
		m_State |= TResourceState_Dead;
	}

	TBOOL TResource::Create()
	{
		TASSERT(TFALSE == IsCreated(), "This resource is already created");

		m_State |= TResourceState_Created;
		return TTRUE;
	}

	TBOOL TResource::Validate()
	{
		TASSERT(IsDying() == TFALSE, "Resource is dying and cannot be validated");
		TASSERT(Parent() == TNULL || Parent()->IsDying() == TFALSE, "Parent resource is dying");
		
		if (IsDying()) return TFALSE;
		
		m_State |= TResourceState_Valid;
		return TTRUE;
	}

	void TResource::Invalidate()
	{
		m_State &= ~TResourceState_Valid;
	}

	void TResource::DestroyResource()
	{
		GetRenderer()->DestroyResource(this);
	}

	void TResource::OnDestroy()
	{
		TASSERT(TTRUE == IsCreated(), "Tried to destroy resource that wasn't created");

		m_State &= ~TResourceState_Created;
		m_State |= TResourceState_Dying;
	}

	void TResource::SetParent(TResource* a_pParent)
	{
		auto pTree = TNode::Tree();

		pTree->Remove(this);

		if (!a_pParent)
		{
			if (pTree)
			{
				pTree->InsertAtRoot(this);
			}
		}
		else
		{
			pTree->Insert(a_pParent, this);
		}
	}

	void TResource::SetName(const char* name)
	{
		// 006b5350
		if (name == TNULL)
		{
			name = "res:";

			char UIdStr[12];
			T2String8::IntToString(m_UId, UIdStr, 0xE, 10);
		}

		TASSERT(TStringManager::String8Length(name) <= MAXNAMELEN, "Name is too long");
		TStringManager::String8Copy(m_Name, name, -1);
	}

	TBOOL TResource::RecurseSimple(t_RecurseCb a_pCallback, TResource* a_pResource, void* a_pUserData)
	{
		if (a_pResource)
		{
			return Recurse(a_pCallback, a_pResource, TFALSE, a_pUserData);
		}
		else
		{
			return Recurse(a_pCallback, GetChild(), TTRUE, a_pUserData);
		}
	}

	TBOOL TResource::Recurse(t_RecurseCb a_pCallback, TResource* a_pResource, TBOOL a_bFlag, void* a_pUserData)
	{
		TResource* pResource = a_pResource;

		while (pResource)
		{
			TResource* pNext = pResource->GetNextResource();

			if (pNext == a_pResource || pNext == pResource || !a_bFlag)
			{
				pNext = TNULL;
			}

			if (!a_pCallback(pResource, a_pUserData)) return TFALSE;

			auto pAttached = pResource->GetChild();
			if (pAttached && !Recurse(a_pCallback, pResource, TTRUE, a_pUserData))
			{
				return TFALSE;
			}

			pResource = pNext;
		}

		return TTRUE;
	}

}