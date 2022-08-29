#include "pch.h"
#include "TResource.h"

bool Toshi::TResource::Create()
{
	TASSERT(TFALSE == IsCreated(), "");
	m_Flags |= 2;
	return true;
}

bool Toshi::TResource::Validate()
{
	// Deblob does many TASSERTS beforehand (more validation than JPOG)
	// TASSERT(TFALSE == IsLinked(), "");
	TASSERT(m_Flags & TResource::FLAGS_DYING, "");
	// Thats what JPOG and Deblob does
	if (m_Flags & TResource::FLAGS_DYING) return false;
	m_Flags |= 1;
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
