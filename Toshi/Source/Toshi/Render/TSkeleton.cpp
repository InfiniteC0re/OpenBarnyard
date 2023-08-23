#include "ToshiPCH.h"
#include "TSkeleton.h"

namespace Toshi {

	TSkeleton::TSkeleton()
	{
		m_iBoneCount = 0;
		m_iManualBoneCount = 0;
		m_iSequenceCount = 0;
		m_iAnimationMaxCount = 0;
		m_iInstanceCount = 0;
		m_pBones = TNULL;
		m_SkeletonSequences = TNULL;
	}

	void TSkeleton::Delete()
	{
		if (m_KeyLibraryInstance.GetLibrary() != TNULL)
		{
			auto& instance = GetKeyLibraryInstance();
			instance.SetSCount(instance.GetSCount() - 1);
			instance.SetLibrary(TNULL);
		}

		//tfree(this);
	}

	TBOOL TSkeleton::Create(uint32_t param_1)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	TSkeletonInstance* TSkeleton::CreateInstance(TBOOL a_bSetBasePose)
	{
		if (TREINTERPRETCAST(int, m_fnQuatLerp) < 3)
		{
			SetQInterpFn(TREINTERPRETCAST(QUATINTERP, m_fnQuatLerp));
		}

		auto iAutoBoneCount = GetAutoBoneCount();
		size_t iAnimationSize = iAutoBoneCount * 2 + TMath::AlignNum(sizeof(TSkeletonInstanceAnimation));
		size_t iInstanceSize = sizeof(TSkeletonInstance) + sizeof(TSkeletonInstanceBone) * iAutoBoneCount + iAnimationSize * GetAnimationMaxCount();
		TSkeletonInstance* pInstance;

		if (s_pAnimMemHeap)
		{
			pInstance = TSTATICCAST(TSkeletonInstance*, s_pAnimMemHeap->Malloc(iInstanceSize));
		}
		else
		{
			pInstance = TSTATICCAST(TSkeletonInstance*, TMalloc(iInstanceSize));
		}

		new (pInstance) TSkeletonInstance();
		pInstance->m_pSkeleton = this;
		pInstance->m_iSize = iInstanceSize;
		pInstance->m_iBaseAnimationCount = 0;
		pInstance->m_iOverlayAnimationCount = 0;
		pInstance->m_pBones = TREINTERPRETCAST(TSkeletonInstanceBone*, this + 1);
		pInstance->m_pAnimations = TREINTERPRETCAST(TSkeletonInstanceAnimation*, this + 1) + iAutoBoneCount;
		pInstance->m_fUnk3 = 3.0f;
		pInstance->m_iUnk4 = 0;
		pInstance->m_iCurrentFrame = 0;
		pInstance->m_iUnk5 = -1;

		for (int i = 0; i < GetAnimationMaxCount(); i++)
		{
			TSkeletonInstanceAnimation* pAnimation = TREINTERPRETCAST(
				TSkeletonInstanceAnimation*,
				TREINTERPRETCAST(uintptr_t, pInstance->m_pAnimations) + i * iAnimationSize
			);

			new (pAnimation) TSkeletonInstanceAnimation();
			pInstance->m_FreeAnimations.InsertHead(pAnimation);
		}

		if (a_bSetBasePose)
			pInstance->SetStateFromBasePose();

		m_iInstanceCount++;
		return pInstance;
	}

	void TSkeleton::SetQInterpFn(QUATINTERP a_eQuatInterp)
	{
		if (a_eQuatInterp == QUATINTERP_Default)
			m_fnQuatLerp = TQuaternion::Nlerp;
		else if (a_eQuatInterp == QUATINTERP_Nlerp)
			m_fnQuatLerp = TQuaternion::Nlerp;
		else if (a_eQuatInterp == QUATINTERP_Slerp)
			m_fnQuatLerp = TQuaternion::Slerp;
	}

	uint32_t TSkeleton::GetBoneID(const char* a_cBoneName, uint32_t length)
	{
		if (length == 0)
		{
			length = (uint32_t)TStringManager::String8Length(a_cBoneName);
		}

		for (short i = 0; i < m_iBoneCount; i++)
		{
			if (m_pBones[i].GetResourceName().GetNameLength() == length)
			{
				int comp = TStringManager::String8CompareNoCase(m_pBones[i].GetResourceName().GetName(), a_cBoneName, length);
				if (comp == 0) { return i; }
			}
		}

		return -1;
	}

	uint32_t TSkeleton::GetSequenceID(const char* a_sSequenceName, uint32_t length)
	{
		if (length == 0)
		{
			length = (uint32_t)TStringManager::String8Length(a_sSequenceName);
		}

		for (short i = 0; i < m_iSequenceCount; i++)
		{
			if (m_SkeletonSequences[i]->GetResourceName().GetNameLength() == length)
			{
				int comp = TStringManager::String8CompareNoCase(m_SkeletonSequences[i]->GetResourceName().GetName(), a_sSequenceName, length);
				if (comp == 0) { return i; }
			}
		}

		return -1;
	}


	void TSkeletonInstance::SetStateFromBasePose()
	{
		TIMPLEMENT();
	}

}
