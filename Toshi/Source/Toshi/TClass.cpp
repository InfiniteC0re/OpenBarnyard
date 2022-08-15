#include "pch.h"
#include "TClass.h"
#include "Typedefs.h"

bool __thiscall Toshi::TClass::IsAttached() const
{
	if (m_parent != TNULL)
	{
		Toshi::TClass* tclass = (Toshi::TClass*)m_parent->m_tclass;
		bool bVar2 = tclass != TNULL;
		if (bVar2)
		{
			while (bVar2)
			{
				if (tclass == this) return true;
				tclass = tclass->m_parent;
				bVar2 = tclass != TNULL;
			}
		}
	}

	return false;
}

bool __thiscall Toshi::TClass::AttachClassToParent()
{
	if (m_parent != 0)
	{
		bool bVar1 = IsAttached();
		if (!bVar1)
		{
			m_attached = m_parent->m_tclass;
			m_parent->m_tclass = this;
			return true;
		}

	}
	return false;
}

Toshi::TClass::TClass(const char* name, TClass* parent, t_CreateTObject create, t_CreateTObjectAtPlace createAtPlace, t_RegisterScriptingAPI scripting, uint16_t verMajor, uint16_t verMinor, size_t size)
{
	m_pcClassName = name;
	m_pCreateTObject = create;
	m_pCreateTObjectAtPlace = createAtPlace;
	m_pRegisterScriptingAPI = scripting;
	m_parent = parent;
	m_version = (verMajor << 16) | verMinor;
	m_size = size;
	m_tclass = nullptr;

	AttachClassToParent();
}
