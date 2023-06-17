#include "ToshiPCH.h"
#include "TResource.h"
#include "TRender.h"

namespace Toshi
{
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
}