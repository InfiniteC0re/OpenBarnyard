#include "ToshiPCH.h"
#include "TClass.h"
#include "TSystem.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{
	TClass::TClass(
		const char* name, TClass* parent, uint32_t version, size_t size,
		t_CreateTObject fCreate, t_CreateTObjectInPlace fCreateInPlace,
		t_InitializeStatic fInit, t_UninitializeStatic fUninit
	)
	{
		m_Name = name;
		m_Create = fCreate;
		m_CreateInPlace = fCreateInPlace;
		m_Initialize = fInit;
		m_Uninitialize = fUninit;
		m_Parent = parent;
		m_Version = version;
		m_Size = size;
		m_Initialized = TFALSE;
		m_Unk = 0;

		if (m_Parent)
		{
			// check if it's not attached yet
			TClass* tClass = m_Parent->m_LastAttached;
			while (tClass != TNULL)
			{
				if (tClass == this) return;
				tClass = tClass->m_Previous;
			}

			m_Previous = m_Parent->m_LastAttached;
			m_Parent->m_LastAttached = this;
		}
	}

	TClass::~TClass()
	{
		m_LastAttached = TNULL;
		m_Previous = TNULL;
		m_Parent = TNULL;

		// Uninitialize static
		if (m_Initialized && m_Uninitialize)
		{
			m_Initialized = TFALSE;
			m_Uninitialize();
		}
	}

	void TClass::Initialize()
	{
		if (m_Initialize)
		{
			m_Initialize();
			m_Initialized = TTRUE;
		}
	}

	void TClass::RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom)
	{
		TBOOL valid = fCheck(this, custom);

		if (valid)
		{
			if (fBaseBegin) fBaseBegin(this, custom);
			RecurseTree2(fCheck, fBaseBegin, fBaseEnd, custom);
			if (fBaseEnd) fBaseEnd(this, custom);
		}
	}

	void TClass::RecurseTree2(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom)
	{
		TClass* tClass = m_LastAttached;

		while (tClass != TNULL)
		{
			if (fCheck) fCheck(tClass, custom);

			if (tClass->m_LastAttached)
			{
				if (fBaseBegin) fBaseBegin(tClass, custom);
				tClass->RecurseTree2(fCheck, fBaseBegin, fBaseEnd, custom);
				if (fBaseEnd) fBaseEnd(tClass, custom);
			}

			tClass = tClass->m_Previous;
		}
	}

	const TClass* TClass::FindRecurse(const char* const& name, const TClass* parent, TBOOL hasPrevious)
	{
		while (parent)
		{
			TClass* previous = hasPrevious ? parent->m_Previous : TNULL;
			int difference = Toshi::TStringManager::String8CompareNoCase(parent->m_Name, name, -1);

			if (difference == 0)
			{
				return parent;
			}

			if (parent->m_LastAttached)
			{
				const TClass* result = FindRecurse(name, parent->m_LastAttached, TTRUE);

				if (result)
				{
					return result;
				}
			}

			parent = previous;
		}

		return TNULL;
	}

	const TClass* TClass::Find(const char* name, const TClass* parent)
	{
		if (parent == TNULL) parent = &TObject::s_Class;
		return FindRecurse(name, parent, TFALSE);
	}

	TObject* TClass::CreateObject()
	{
		if (m_Create != TNULL)
		{
			return m_Create();
		}

		return TNULL;
	}

	TBOOL TClass::IsA(TClass* cmpClass)
	{
		if (cmpClass->m_LastAttached == TNULL)
		{
			return (this == cmpClass);
		}
		else
		{
			TClass* pClass = this;

			while (pClass != TNULL)
			{
				if (pClass == cmpClass)
				{
					return TTRUE;
				}

				pClass = pClass->m_Parent;
			}
		}

		return TFALSE;
	}
		
	TBOOL TClass::TryInitialize(TClass* tClass)
	{
		if (!tClass->IsInitialized())
		{
			tClass->Initialize();
		}
		
		return TTRUE;
	}
}