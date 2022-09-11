#include "ToshiPCH.h"
#include "TSkeleton.h"

Toshi::TSkeleton::TSkeleton()
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

void Toshi::TSkeleton::Delete()
{
	if (m_KeyLibraryInstance.GetLibrary() != TNULL)
	{
		m_KeyLibraryInstance.m_pKeyLibrary->m_iReferenceCount--;
		m_KeyLibraryInstance.m_pKeyLibrary = TNULL;
	}
	tfree(this);
}

bool Toshi::TSkeleton::Create(uint32_t param_1)
{
	TIMPLEMENT();
	return false;
}

int Toshi::TSkeleton::GetBoneID(const char* a_cBoneName, int length)
{
	if (length == 0)
	{
		length = Toshi::TSystem::StringLength(a_cBoneName);
	}

	for (size_t i = 0; i < m_sBoneCount; i++)
	{
		if (m_Bones[i].GetResourceName().GetNameLength() == length)
		{
			int comp = TSystem::StringCompareNoCase(m_Bones[i].GetResourceName().GetName(), a_cBoneName, length);
			return comp == 0;
		}
	}

	return -1;
}

int Toshi::TSkeleton::GetSequenceID(const char* a_sSequenceName, int length)
{
	if (length == 0)
	{
		length = Toshi::TSystem::StringLength(a_sSequenceName);
	}

	for (size_t i = 0; i < m_sBoneCount; i++)
	{
		if (m_SkeletonSequences[i]->GetResourceName().GetNameLength() == length)
		{
			int comp = TSystem::StringCompareNoCase(m_SkeletonSequences[i]->GetResourceName().GetName(), a_sSequenceName, length);
			return comp == 0;
		}
	}

	return -1;
}
