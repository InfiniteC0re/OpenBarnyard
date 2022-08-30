#include "pch.h"
#include "TResource.h"

bool Toshi::TResource::Create()
{
	TASSERT(TFALSE == IsCreated(), "Trying to create TResource twice");

	m_State |= TResourceState_Created;
	return true;
}

bool Toshi::TResource::Validate()
{
	// Deblob does many TASSERTS beforehand (more validation than JPOG)
	// TASSERT(TFALSE == IsLinked(), "");
	TASSERT(m_State & TResourceState_Dying, "TResource is dying");
	// Thats what JPOG and Deblob does
	if (m_State & TResourceState_Dying) return false;
	m_State |= TResourceState_Valid;

	return true;
}

void Toshi::TResource::DestroyResource()
{
	//GetRenderer().DestroyResource(this);
}

Toshi::TResource* Toshi::TResource::Parent() const
{
	return m_pParent == TNULL ? TNULL : m_pParent->m_value;
}
