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
		m_Initialized = false;
		m_Unk = 0;

		if (m_Parent)
		{
			// check if it's not attached yet
			TClass* tClass = m_Parent->m_LastAttached;
			while (tClass != nullptr)
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
		m_LastAttached = nullptr;
		m_Previous = nullptr;
		m_Parent = nullptr;

		// Uninitialize static
		if (m_Initialized && m_Uninitialize)
		{
			m_Initialized = false;
			m_Uninitialize();
		}
	}

	void TClass::Initialize()
	{
		if (m_Initialize)
		{
			m_Initialize();
			m_Initialized = true;
		}
	}

	void TClass::RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom)
	{
		bool valid = fCheck(this, custom);

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

		while (tClass != nullptr)
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

	TClass* TClass::FindRecurse(const char* const& name, TClass* parent, bool hasPrevious)
	{
		while (parent)
		{
			TClass* previous = hasPrevious ? parent->m_Previous : nullptr;
			int difference = TSystem::StringCompareNoCase(parent->m_Name, name, -1);

			if (difference == 0)
			{
				return parent;
			}

			if (parent->m_LastAttached)
			{
				TClass* result = FindRecurse(name, parent->m_LastAttached, true);

				if (result)
				{
					return result;
				}
			}

			parent = previous;
		}

		return nullptr;
	}

	TObject* TClass::CreateObject()
	{
		if (m_Create != nullptr)
		{
			return m_Create();
		}

		return nullptr;
	}
		
	bool TClass::TryInitialize(TClass* tClass)
	{
		if (!tClass->IsInitialized())
		{
			tClass->Initialize();
		}
		
		return true;
	}
}