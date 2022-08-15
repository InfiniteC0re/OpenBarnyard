#include "TClass.h"
#include "Typedefs.h"

void __thiscall Toshi::TClass::InitialiseStatic()
{
	// this is not used after JPOG and can be done using static variable
	
	//if (m_pFunc != TNULL)
	//{
	//	TASSERT(TFALSE == m_bInitialised);
	//	//m_pFunc();
	//	m_bInitialised = true;
	//}
}

void __thiscall Toshi::TClass::DeinitialiseStatic()
{
	// this is not used after JPOG and can be done using static variable
	
	//if (m_pFunc2 != TNULL)
	//{
	//	TASSERT(TTRUE == m_bInitialised);
	//	m_bInitialised = false;
	//	//m_pFunc2();
	//}
}

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

Toshi::TClass::TClass(const char* name, TClass* parent, t_CreateTObject create, t_CreateTObjectAtPlace createAtPlace, t_RegisterScriptingAPI scripting, void* unk, size_t size)
{
	m_pcClassName = (char*)name;
	m_pTObjectCTOR = create;
	m_pTObjectCTORAtPlace = createAtPlace;
	m_pRegisterScriptingAPI = scripting;
	m_pFunc = unk;
	m_parent = parent;
	m_size = size;
	m_tclass = nullptr;
	AttachClassToParent();
}
