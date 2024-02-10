#include "ToshiPCH.h"
#include "TClass.h"
#include "TSystem.h"
#include "Toshi/Typedefs.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TClass::TClass(const TCHAR* a_szName, TClass* a_pParentClass, CreateObject_t a_fnCreate, CreateObjectInPlace_t a_fnCreateInPlace, Initialise_t a_fnInitialise, Deinitialise_t a_fnUnitialise, TUINT16 a_uiVersionMajor, TUINT16 a_uiVersionMinor, TUINT32 a_uiClassSize, TUINT32 a_uiClassAlignment) :
		m_szName(a_szName),
		m_pParent(a_pParentClass),
		m_fnCreate(a_fnCreate),
		m_fnCreateInPlace(a_fnCreateInPlace),
		m_fnInitialise(a_fnInitialise),
		m_fnDeinitialise(a_fnUnitialise),
		m_uiVersionMajor(a_uiVersionMajor),
		m_uiVersionMinor(a_uiVersionMinor),
		m_uiClassSize(a_uiClassSize),
		m_uiClassAlignment(a_uiClassAlignment),
		m_bInitialised(TFALSE)
	{
		if (m_pParent)
		{
			TClass* pClass = m_pParent->m_pLastChildren;

			while (pClass != TNULL)
			{
				if (pClass == this) return;
				pClass = pClass->m_pPrevious;
			}

			m_pPrevious = m_pParent->m_pLastChildren;
			m_pParent->m_pLastChildren = this;
		}
	}

	TClass::~TClass()
	{
		m_pLastChildren = TNULL;
		m_pPrevious = TNULL;
		m_pParent = TNULL;

		// Uninitialize static
		if (m_bInitialised && m_fnDeinitialise)
		{
			m_bInitialised = TFALSE;
			m_fnDeinitialise();
		}
	}

	void TClass::Initialize()
	{
		if (m_fnInitialise)
		{
			m_fnInitialise();
			m_bInitialised = TTRUE;
		}
	}

	void TClass::RecurseTree(RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom)
	{
		TBOOL valid = fCheck(this, custom);

		if (valid)
		{
			if (fBaseBegin) fBaseBegin(this, custom);
			RecurseTree2(fBaseBegin, fBaseEnd, fCheck, custom);
			if (fBaseEnd) fBaseEnd(this, custom);
		}
	}

	void TClass::RecurseTree2(RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom)
	{
		for (TClass* pClass = m_pLastChildren; pClass != TNULL; pClass = pClass->m_pPrevious)
		{
			if (fCheck) fCheck(pClass, custom);

			if (pClass->m_pLastChildren)
			{
				if (fBaseBegin) fBaseBegin(pClass, custom);
				pClass->RecurseTree2(fBaseBegin, fBaseEnd, fCheck, custom);
				if (fBaseEnd) fBaseEnd(pClass, custom);
			}
		}
	}

	TClass* TClass::FindRecurse(const TCHAR* const& name, TClass* parent, TBOOL hasPrevious)
	{
		while (parent)
		{
			TClass* previous = hasPrevious ? parent->m_pPrevious : TNULL;
			TINT difference = Toshi::TStringManager::String8CompareNoCase(parent->m_szName, name, -1);

			if (difference == 0)
			{
				return parent;
			}

			if (parent->m_pLastChildren)
			{
				TClass* result = FindRecurse(name, parent->m_pLastChildren, TTRUE);

				if (result)
				{
					return result;
				}
			}

			parent = previous;
		}

		return TNULL;
	}

	TClass* TClass::Find(const TCHAR* name, TClass* parent)
	{
		if (parent == TNULL) parent = &TObject::TClassObjectName;
		return FindRecurse(name, parent, TFALSE);
	}

	TObject* TClass::CreateObject() const
	{
		if (m_fnCreate != TNULL)
		{
			return m_fnCreate();
		}

		return TNULL;
	}

	TBOOL TClass::IsA(TClass* cmpClass)
	{
		if (cmpClass->m_pLastChildren == TNULL)
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

				pClass = pClass->m_pParent;
			}
		}

		return TFALSE;
	}
		
	TUINT32 TClass::GetMaxSizeOfDerivedClasses()
	{
		auto GetMaxSizeOfClass = [](TClass* a_pClass, void* a_pSize) -> TBOOL
		{
			if (*(TUINT32*)a_pSize < a_pClass->m_uiClassSize)
			{
				*(TUINT32*)a_pSize = a_pClass->m_uiClassSize;
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
			if (*(TUINT32*)a_pAlignment < a_pClass->m_uiClassAlignment)
			{
				*(TUINT32*)a_pAlignment = a_pClass->m_uiClassAlignment;
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