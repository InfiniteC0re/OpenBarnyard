#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TKeyframeLibrary* TKeyframeLibraryManager::GetLibrary(const char* a_szName)
	{
		for (auto it = m_List.Begin(); it != m_List.End(); it++)
		{
			if (TStringManager::String8CompareNoCase(it->GetName(), a_szName) == 0)
			{
				return it;
			}
		}

		return TNULL;
	}

	TKeyframeLibrary* TKeyframeLibraryManager::LoadLibraryFromTRB(const char* a_szFilePath)
	{
		TVALIDPTR(a_szFilePath);

		auto pTRB = new TTRB();

		if (pTRB->Load(a_szFilePath, 0) != TTRB::ERROR_OK)
		{
			delete pTRB;
			return TNULL;
		}

		TKeyframeLibrary* pLibrary = TKeyframeLibrary::CreateFromTRB(pTRB, "keylib");

		m_List.InsertHead(pLibrary);
		m_iNumLibraries++;

		pLibrary->SetTRB(pTRB);
		return pLibrary;
	}

	void TKeyframeLibraryManager::UnloadLibrary(TKeyframeLibrary* a_pLibrary)
	{
		TVALIDPTR(a_pLibrary);

		a_pLibrary->Remove();
		a_pLibrary->Delete();
		m_iNumLibraries--;
	}

}
