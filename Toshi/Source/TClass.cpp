#include "TClass.h"

char* __thiscall Toshi::TClass::GetName()
{
	return m_pcClassName;
}

void __thiscall Toshi::TClass::InitialsieStatic()
{
	if (m_pFunc != TNULL)
	{
		TASSERT(TFALSE == m_bInitialised);
		//m_pFunc();
		m_bInitialised = true;
	}
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

void __thiscall Toshi::TClass::DeinitialsieStatic()
{
	if (m_pFunc2 != TNULL)
	{
		TASSERT(TTRUE == m_bInitialised);
		m_bInitialised = false;
		//m_pFunc2();
	}

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

__thiscall Toshi::TClass::TClass(char const* a_pClassName, class Toshi::TClass* param_2, void * param_3, void * param_4, void* param_5, void* param_6, unsigned int param_7)
{
	m_pcClassName = (char*)a_pClassName;
	m_pTObjectCTOR = param_3;
	m_pTObjectDTOR = param_4;
	m_pFunc = param_5;
	m_pFunc2 = param_6;
	m_parent = param_2;
	m_unk = param_7;
	m_tclass = nullptr;
	AttachClassToParent();
}
