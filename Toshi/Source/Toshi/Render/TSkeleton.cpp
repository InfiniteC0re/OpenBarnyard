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
		size_t iAnimationSize = iAutoBoneCount * sizeof(short) + TMath::AlignNum(sizeof(TAnimation));
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
		pInstance->m_iFlags = 0;
		pInstance->m_pBones = TREINTERPRETCAST(TSkeletonInstanceBone*, this + 1);
		pInstance->m_pAnimations = TREINTERPRETCAST(TAnimation*, this + 1) + iAutoBoneCount;
		pInstance->m_fUnk3 = 3.0f;
		pInstance->m_iUnk4 = 0;
		pInstance->m_iCurrentFrame = 0;
		pInstance->m_iUnk5 = -1;

		for (int i = 0; i < GetAnimationMaxCount(); i++)
		{
			TAnimation* pAnimation = TREINTERPRETCAST(
				TAnimation*,
				TREINTERPRETCAST(uintptr_t, pInstance->m_pAnimations) + i * iAnimationSize
			);

			new (pAnimation) TAnimation();
			pInstance->m_FreeAnimations.InsertHead(pAnimation);
		}

		if (a_bSetBasePose)
			pInstance->SetStateFromBasePose();

		m_iInstanceCount++;
		return pInstance;
	}

	void TSkeleton::SetQInterpFn(QUATINTERP a_eQuatInterp)
	{
		if (a_eQuatInterp == QUATINTERP_Slerp)
			m_fnQuatLerp = TQuaternion::Slerp;
		else
			m_fnQuatLerp = TQuaternion::Nlerp;
	}

	int TSkeleton::GetBoneID(const char* a_cBoneName, uint32_t a_iLength)
	{
		for (short i = 0; i < m_iBoneCount; i++)
		{
			if (TStringManager::String8CompareNoCase(m_pBones[i].GetName(), a_cBoneName, a_iLength) == 0)
			{
				return i;
			}
		}

		return -1;
	}

	int TSkeleton::GetSequenceID(const char* a_sSequenceName, uint32_t a_iLength)
	{
		for (short i = 0; i < m_iSequenceCount; i++)
		{
			if (TStringManager::String8CompareNoCase(m_SkeletonSequences[i].GetName(), a_sSequenceName, a_iLength) == 0)
			{
				return i;
			}
		}

		return -1;
	}

	void TSkeletonInstance::UpdateTime(float a_fDeltaTime)
	{
		if (!m_BaseAnimations.IsEmpty() || (!m_OverlayAnimations.IsEmpty() &&
			m_iCurrentFrame != TSystemManager::GetSingleton()->GetFrameCount()))
		{
			m_iCurrentFrame = TSystemManager::GetSingleton()->GetFrameCount();
			m_fUnk3 = 0.0f;

			// Update base animations
			for (auto pAnim = m_BaseAnimations.Begin(); pAnim != m_BaseAnimations.End(); pAnim++)
			{
				if (pAnim->UpdateTime(a_fDeltaTime))
				{
					m_pSkeleton->GetSequence(pAnim->GetSequenceID()); // Throws assert if seqId is wrong
					m_fUnk3 += pAnim->m_fUnk4;
				}
			}

			// Update overlay animations
			for (auto pAnim = m_OverlayAnimations.Begin(); pAnim != m_OverlayAnimations.End(); pAnim++)
			{
				pAnim->UpdateTime(a_fDeltaTime);
			}

			if (HASFLAG(m_iFlags & 1))
			{
				m_iFlags &= ~1;
				UpdateState(TFALSE);
			}
		}
	}

	void TSkeletonInstance::UpdateState(TBOOL a_bForceUpdate)
	{
		TIMPLEMENT();
	}

	void TSkeletonInstance::SetStateFromBasePose()
	{
		for (int i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++)
		{
			m_pSkeleton->GetBone(i);
			m_pBones[i].Matrix.Identity();
		}
	}

	float TSkeletonSequenceBone::GetKeyPair(int a_iCurrentAnimTime, unsigned short& a_rCurrentKeyIndex, unsigned short& a_rLerpFromIndex, unsigned short& a_rLerpToIndex)
	{
		auto pFirstKeyTime = *GetKeyData(0);

		if (a_iCurrentAnimTime < pFirstKeyTime || a_iCurrentAnimTime == pFirstKeyTime)
		{
			// Animation haven't reached it's first frame
			a_rCurrentKeyIndex = 0;
			a_rLerpFromIndex = 0;
			a_rLerpToIndex = 0;
			return 0.0f;
		}

		auto iLastKeyIndex = m_iNumKeys - 1;
		auto pLastKeyTime = *GetKeyData(iLastKeyIndex);
		
		if (pLastKeyTime <= a_iCurrentAnimTime)
		{
			// Animation is over
			a_rCurrentKeyIndex = iLastKeyIndex;
			a_rLerpFromIndex = iLastKeyIndex;
			a_rLerpToIndex = iLastKeyIndex;
			return 0.0f;
		}

		auto pCurrentKeyTime = *GetKeyData(a_rCurrentKeyIndex);
		
		if (pCurrentKeyTime < a_iCurrentAnimTime)
		{
			// Current key is currently lerping
			auto iNextIndex = a_rCurrentKeyIndex + 1;
			auto iNextKeyTime = *GetKeyData(iNextIndex);

			while (iNextKeyTime <= a_iCurrentAnimTime)
			{
				// Skip keys that are already over
				a_rCurrentKeyIndex = iNextIndex++;
				iNextKeyTime = *GetKeyData(iNextIndex);
			}

			a_rLerpFromIndex = a_rCurrentKeyIndex;
			a_rLerpToIndex = iNextIndex;
		}
		else
		{
			if (pCurrentKeyTime == a_iCurrentAnimTime)
			{
				// Current time is right at the end of keys transition
				a_rLerpFromIndex = a_rCurrentKeyIndex;
				a_rLerpToIndex = a_rCurrentKeyIndex;
				return 0.0f;
			}

			// The animation is playing backwards?
			auto iPrevIndex = a_rCurrentKeyIndex - 1;
			auto iPrevKeyTime = *GetKeyData(iPrevIndex);

			while (a_iCurrentAnimTime < iPrevKeyTime || a_iCurrentAnimTime == iPrevKeyTime)
			{
				a_rCurrentKeyIndex = iPrevIndex--;
				iPrevKeyTime = *GetKeyData(iPrevIndex);
			}

			a_rLerpFromIndex = iPrevIndex;
			a_rLerpToIndex = a_rCurrentKeyIndex;
		}

		auto iLerpFromTime = *GetKeyData(a_rLerpFromIndex);
		auto iLerpToTime = *GetKeyData(a_rLerpToIndex);

		return ((a_iCurrentAnimTime - iLerpFromTime) * (1.0f / 65535)) / ((iLerpToTime - iLerpFromTime) * (1.0f / 65535));
	}

}
