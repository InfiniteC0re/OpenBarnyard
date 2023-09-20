#include "ToshiPCH.h"
#include "TAnimation.h"
#include "TSkeleton.h"

namespace Toshi {

	TMemoryHeap* s_pAnimMemHeap = TNULL;

	TBOOL TAnimation::UpdateTime(float a_fDeltaTime)
	{
		auto pSkeleton = m_pSkeletonInstance->GetSkeleton();
		auto pSeq = pSkeleton->GetSequence(m_iSeqID);

		auto fSeqDuration = pSeq->GetDuration();

		m_fTotalTime += m_fSpeed * a_fDeltaTime;
		m_fSeqTime += m_fSpeed * a_fDeltaTime;

		if (pSeq->GetUnk2() == 0 && !HASFLAG(m_eFlags & Flags_Managed))
		{
			if (0.0f <= m_fSpeed || 0.0f <= m_fSeqTime)
			{
				if (fSeqDuration <= m_fSeqTime)
				{
					// Make m_fSeqTime less than animation duration
					m_fSeqTime -= TMath::FloorToInt(m_fSeqTime / fSeqDuration) * fSeqDuration;
					
					// Reset states of all bones
					for (int i = 0; i < pSkeleton->GetAutoBoneCount(); i++)
						*GetBone(i) = 0;
				}
			}
			else
			{
				m_fSeqTime = pSeq->GetDuration();
			}
		}
		else
		{
			if (m_fSeqTime < 0.0f && m_fSpeed < 0.0f)
				m_fSeqTime = fSeqDuration;
			
			if (0.0f <= m_fSeqTime)
			{
				if (fSeqDuration < m_fSeqTime)
				{
					m_fSeqTime = fSeqDuration;
				}
			}
			else
			{
				m_fSeqTime = 0.0f;
			}
		}

		if (m_eMode == MODE_UNK1)
		{
			m_fWeight = (m_fBlendInSpeed == 0.0f) ? m_fDestWeight : m_fWeight + (1.0f / m_fBlendInSpeed) * a_fDeltaTime;

			if (m_fDestWeight <= m_fWeight)
			{
				m_fWeight = m_fDestWeight;
				m_eMode = MODE_UNK2;
			}
		}
		else if (m_eMode == MODE_UNK2)
		{
			if (HASFLAG(m_eFlags & Flags_Managed) && fSeqDuration <= m_fTotalTime)
			{
				m_eMode = MODE_UNK3;
				return TTRUE;
			}
		}
		else if (m_eMode == MODE_UNK3)
		{
			m_fWeight = (m_fBlendOutSpeed == 0.0f) ? -1.0f : m_fWeight - (1.0f / m_fBlendOutSpeed) * a_fDeltaTime;

			if (m_fWeight <= 0.0f)
			{
				m_pSkeletonInstance->RemoveAnimation(this, 0.0f);
				return HASFLAG(m_eFlags & Flags_UpdateStateOnRemove);
			}
		}

		return TTRUE;
	}

	void TAnimation::RemoveAnimation(float a_fVal)
	{
		if (m_pSkeletonInstance)
			m_pSkeletonInstance->RemoveAnimation(this, a_fVal);
	}

	float TAnimation::SetDestWeight(float a_fDestWeight, float a_fBlendInSpeed)
	{
		float fOldValue = m_fDestWeight;

		m_fDestWeight = a_fDestWeight;
		m_fBlendInSpeed = a_fBlendInSpeed;

		if (0.0f < a_fBlendInSpeed)
		{
			m_eMode = MODE_UNK1;
		}
		else
		{
			m_fWeight = a_fDestWeight;
			m_eMode = MODE_UNK2;
		}

		return fOldValue;
	}

	TSkeletonSequence* TAnimation::GetSequencePtr() const
	{
		return m_pSkeletonInstance->GetSkeleton()->GetSequence(m_iSeqID);
	}

	int TAnimation::FindSequenceMaxUnk3(const TQList<TAnimation>& a_rList)
	{
		int iResult = -1;

		for (auto it = a_rList.Begin(); it != a_rList.End(); it++)
		{
			iResult = TMath::Max(iResult, it->GetSequencePtr()->GetUnk3());
		}

		return iResult;
	}

}
