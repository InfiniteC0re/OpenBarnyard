#include "ToshiPCH.h"
#include "TAnimation.h"

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

	TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary(TKeyframeLibrary::TRBHeader* a_pTRBHeader)
	{
		TVALIDPTR(a_pTRBHeader);

		TKeyframeLibrary* pLibrary = TKeyframeLibrary::Create(a_pTRBHeader);

		m_List.InsertHead(pLibrary);
		m_iNumLibraries++;

		return pLibrary;
	}

	TKeyframeLibrary* TKeyframeLibraryManager::LoadLibraryFromTRB(TTRB* a_pTRB, const char* a_szSymbolName)
	{
		TVALIDPTR(a_pTRB);
		TVALIDPTR(a_szSymbolName);

		TKeyframeLibrary::TRBHeader* pTRBHeader = a_pTRB->CastSymbol<TKeyframeLibrary::TRBHeader>(a_szSymbolName);
		TKeyframeLibrary* pLibrary = TKeyframeLibrary::Create(pTRBHeader);

		m_List.InsertHead(pLibrary);
		m_iNumLibraries++;

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
