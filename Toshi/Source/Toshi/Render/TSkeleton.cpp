#include "ToshiPCH.h"
#include "TSkeleton.h"

namespace Toshi {
	
	TSkeleton::TSkeleton()
	{
		m_iSize = 0;
		m_sBoneCount = 0;
		m_sManualBoneCount = 0;
		m_sSequenceCount = 0;
		m_sAnimationMaxCount = 0;
		m_sInstanceCount = 0;

		m_KeyLibraryInstance = TKeyframeLibraryInstance();
		m_Bones = TNULL;
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

	uint32_t TSkeleton::GetBoneID(const char* a_cBoneName, uint32_t length)
	{
		if (length == 0)
		{
			length = (uint32_t)TStringManager::String8Length(a_cBoneName);
		}

		for (short i = 0; i < m_sBoneCount; i++)
		{
			if (m_Bones[i].GetResourceName().GetNameLength() == length)
			{
				int comp = TStringManager::String8CompareNoCase(m_Bones[i].GetResourceName().GetName(), a_cBoneName, length);
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

		for (short i = 0; i < m_sSequenceCount; i++)
		{
			if (m_SkeletonSequences[i]->GetResourceName().GetNameLength() == length)
			{
				int comp = TStringManager::String8CompareNoCase(m_SkeletonSequences[i]->GetResourceName().GetName(), a_sSequenceName, length);
				if (comp == 0) { return i; }
			}
		}

		return -1;
	}


}
