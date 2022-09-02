#include "pch.h"
#include "TClass.h"
#include "TSystem.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{
	TClass::TClass(
		const char* name, TClassProps* parent, uint32_t version, size_t size,
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
#ifdef TOSHI_DEBUG
			// since static initialization occurs TLog::Init, we need to initialize it earlier
			Toshi::TLog::Init();
			
			if (m_Parent->m_Name == nullptr)
			{
				TOSHI_CORE_CRITICAL("Unable to initialize {0} TClass. Say hi to the static initialization order fiasco", m_Name);
			}
			else
			{
				TOSHI_INFO("Attaching {0} to {1}", m_Name, m_Parent->m_Name);
			}
#endif

			// check if it's not attached yet
			TClassProps* tClass = m_Parent->m_LastAttached;
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
		TClassProps* tClassProps = m_LastAttached;
		
		while (tClassProps != nullptr)
		{
			TClass* tClass = TClassFromProps(*tClassProps);
			if (fCheck) fCheck(tClass, custom);

			if (tClass->m_LastAttached)
			{
				if (fBaseBegin) fBaseBegin(this, custom);
				tClass->RecurseTree2(fCheck, fBaseBegin, fBaseEnd, custom);
				if (fBaseEnd) fBaseEnd(this, custom);
			}

			tClassProps = tClassProps->m_Previous;
		}
	}

	TClass* TClass::FindRecurse(const char* const& name, TClass* parent, bool hasPrevious)
	{
		while (parent)
		{
			TClassProps* previous = hasPrevious ? parent->m_Previous : nullptr;
			int difference = TSystem::StringCompareNoCase(parent->m_Name, name, -1);

			if (difference == 0)
			{
				return parent;
			}

			if (parent->m_LastAttached)
			{
				TClass* result = FindRecurse(name, TClassFromProps(parent->m_LastAttached), true);

				if (result)
				{
					return result;
				}
			}

			parent = TClassFromProps(previous);
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