#include "ToshiPCH.h"
#include "TAnimation.h"

namespace Toshi {

	TKeyframeLibrary::TKeyframeLibrary()
	{
		m_szName = TNULL;
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

	TKeyframeLibrary* TKeyframeLibrary::Create(TRBHeader* a_pTRBHeader)
	{
		TKeyframeLibrary* pLibrary = TMEMORY_NEW(TKeyframeLibrary, s_pAnimMemHeap);

		pLibrary->m_szName = a_pTRBHeader->m_szName;
		pLibrary->m_iReferenceCount = 0;
		pLibrary->m_iNumTranslations = a_pTRBHeader->m_iNumTranslations;
		pLibrary->m_iNumQuaternions = a_pTRBHeader->m_iNumQuaternions;
		pLibrary->m_iNumScales = a_pTRBHeader->m_iNumScales;
		pLibrary->m_iTranslationSize = a_pTRBHeader->m_iTranslationSize;
		pLibrary->m_iQuaternionSize = a_pTRBHeader->m_iQuaternionSize;
		pLibrary->m_iScaleSize = a_pTRBHeader->m_iScaleSize;
		pLibrary->m_pTranslations = a_pTRBHeader->m_pTranslations;
		pLibrary->m_pQuaternions = a_pTRBHeader->m_pQuaternions;
		pLibrary->m_pScales = a_pTRBHeader->m_pScales;

		return pLibrary;
	}

}
