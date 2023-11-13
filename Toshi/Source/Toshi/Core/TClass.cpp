#include "ToshiPCH.h"
#include "TClass.h"
#include "TSystem.h"
#include "Toshi/Typedefs.h"

namespace Toshi
{
	TClass::TClass(
		const char* name, TClass* parent, TUINT32 version, TUINT32 size, TUINT32 alignment,
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
		m_Alignment = alignment;

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

	void TClass::RecurseTree(t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, t_RecurceTreeCheck fCheck, void* custom)
	{
		TBOOL valid = fCheck(this, custom);

		if (valid)
		{
			if (fBaseBegin) fBaseBegin(this, custom);
			RecurseTree2(fBaseBegin, fBaseEnd, fCheck, custom);
			if (fBaseEnd) fBaseEnd(this, custom);
		}
	}

	void TClass::RecurseTree2(t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, t_RecurceTreeCheck fCheck, void* custom)
	{
		for (TClass* pClass = m_LastAttached; pClass != TNULL; pClass = pClass->m_Previous)
		{
			if (fCheck) fCheck(pClass, custom);

			if (pClass->m_LastAttached)
			{
				if (fBaseBegin) fBaseBegin(pClass, custom);
				pClass->RecurseTree2(fBaseBegin, fBaseEnd, fCheck, custom);
				if (fBaseEnd) fBaseEnd(pClass, custom);
			}
		}
	}

	TClass* TClass::FindRecurse(const char* const& name, TClass* parent, TBOOL hasPrevious)
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
				TClass* result = FindRecurse(name, parent->m_LastAttached, TTRUE);

				if (result)
				{
					return result;
				}
			}

			parent = previous;
		}

		return TNULL;
	}

	TClass* TClass::Find(const char* name, TClass* parent)
	{
		if (parent == TNULL) parent = &TObject::s_Class;
		return FindRecurse(name, parent, TFALSE);
	}

	TObject* TClass::CreateObject() const
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
		
	TUINT32 TClass::GetMaxSizeOfDerivedClasses()
	{
		auto GetMaxSizeOfClass = [](TClass* a_pClass, void* a_pSize) -> TBOOL
		{
			if (*(TUINT32*)a_pSize < a_pClass->m_Size)
			{
				*(TUINT32*)a_pSize = a_pClass->m_Size;
			}

			return TTRUE;
		};

		TUINT32 uiClassSize = 0;
		if (GetMaxSizeOfClass(this, &uiClassSize))
		{
			RecurseTree2(TNULL, TNULL, GetMaxSizeOfClass, &uiClassSize);
		}

		return uiClassSize;
	}

	TUINT32 TClass::GetMaxAlignmentOfDerivedClasses()
	{
		auto GetMaxAlignmentOfClass = [](TClass* a_pClass, void* a_pAlignment) -> TBOOL
		{
			if (*(TUINT32*)a_pAlignment < a_pClass->m_Alignment)
			{
				*(TUINT32*)a_pAlignment = a_pClass->m_Alignment;
			}

			return TTRUE;
		};

		TUINT32 uiAlignment = 0;
		if (GetMaxAlignmentOfClass(this, &uiAlignment))
		{
			RecurseTree2(TNULL, TNULL, GetMaxAlignmentOfClass, &uiAlignment);
		}

		return uiAlignment;
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