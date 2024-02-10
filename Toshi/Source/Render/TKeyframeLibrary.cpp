#include "ToshiPCH.h"
#include "TAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TKeyframeLibrary::TKeyframeLibrary()
	{
		m_szName[0] = '\0';
		m_iReferenceCount = 0;
		m_iNumTranslations = 0;
		m_iNumQuaternions = 0;
		m_iNumScales = 0;
		m_iTranslationSize = 0;
		m_iQuaternionSize = 0;
		m_iScaleSize = 0;
		m_pTranslations = TNULL;
		m_pQuaternions = TNULL;
		m_pScales = TNULL;
		m_pTRB = TNULL;
	}

	void TKeyframeLibrary::Delete()
	{
		TASSERT(m_iReferenceCount == 0);

		if (m_pTRB)
		{
			m_pTRB->Close();
			delete m_pTRB;
			m_pTRB = TNULL;
		}

		delete this;
	}

	TKeyframeLibrary* TKeyframeLibrary::CreateFromTRB(TTRB* a_pTRB, const TCHAR* a_szSymbolName)
	{
		TVALIDPTR(a_pTRB);
		TVALIDPTR(a_szSymbolName);

		TKeyframeLibrary* pLibrary = new TKeyframeLibrary;
		TRBHeader* pTRBHeader = TSTATICCAST(TRBHeader*, a_pTRB->GetSymbolAddress(a_szSymbolName));

		auto iNameLen = TStringManager::String8Length(pTRBHeader->m_szName);
		TStringManager::String8Copy(pLibrary->m_szName, pTRBHeader->m_szName, TMath::Min(iNameLen, sizeof(pLibrary->m_szName)));
		
		pLibrary->m_iReferenceCount = 0;
		pLibrary->m_SomeVector = pTRBHeader->m_SomeVector;
		pLibrary->m_iNumTranslations = pTRBHeader->m_iNumTranslations;
		pLibrary->m_iNumQuaternions = pTRBHeader->m_iNumQuaternions;
		pLibrary->m_iNumScales = pTRBHeader->m_iNumScales;
		pLibrary->m_iTranslationSize = pTRBHeader->m_iTranslationSize;
		pLibrary->m_iQuaternionSize = pTRBHeader->m_iQuaternionSize;
		pLibrary->m_iScaleSize = pTRBHeader->m_iScaleSize;
		pLibrary->m_pTranslations = pTRBHeader->m_pTranslations;
		pLibrary->m_pQuaternions = pTRBHeader->m_pQuaternions;
		pLibrary->m_pScales = pTRBHeader->m_pScales;

		return pLibrary;
	}

}
