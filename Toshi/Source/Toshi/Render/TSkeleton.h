#pragma once
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Render/ResourceName.h"

namespace Toshi
{
	class TSkeletonSequence
	{
		enum Flag : unsigned short
		{
			FLAG_Overlay = BITFIELD(0),
		};

		enum LOOPMODE
		{
			LOOPMODE_LOOP
		};

		ResourceName m_ResourceName;
		Flag m_Flags;
		LOOPMODE m_eLoopMode;

	public:
		TBOOL IsLooping() { return m_eLoopMode == LOOPMODE_LOOP; }
		TBOOL IsOverlay() { return m_Flags & FLAG_Overlay; }
		ResourceName& GetResourceName() { return m_ResourceName; }
	};

	class TSkeletonBone
	{
		ResourceName m_ResourceName; // 0x90

	public:
		ResourceName& GetResourceName() { return m_ResourceName; }
	};

	class TSkeleton
	{
	public:
		TSkeleton();
		void Delete();
		TBOOL Create(uint32_t param_1);

		TSkeleton& operator=(const TSkeleton& a_rSkeleton) { TIMPLEMENT(); }

		uint32_t GetSize() { return m_iSize; }
		TSkeletonBone* GetBone(const char* a_cBoneName, uint32_t length) { return GetBone(GetBoneID(a_cBoneName, length)); }
		uint32_t GetBoneID(const char* a_cBoneName, uint32_t length);

		TKeyframeLibraryInstance& GetKeyLibraryInstance() { return m_KeyLibraryInstance; }
		short GetAnimationMaxCount() { return m_sAnimationMaxCount; }
		TSkeletonBone* GetBone(uint32_t index) { return &m_Bones[index]; }
		TSkeletonBone* GetBones() { return m_Bones; }
		short GetInstanceCount() { return m_sInstanceCount; }
		short GetAutoBoneCount() { return m_sBoneCount - m_sManualBoneCount; }

		uint32_t GetSequenceID(const char* a_sSequenceName, uint32_t length);
		TSkeletonSequence** GetSequences() { return m_SkeletonSequences; }
		TSkeletonSequence* GetSequence(uint32_t index) { return m_SkeletonSequences[index]; }
		TSkeletonSequence* GetSequence(const char* a_sSequenceName, uint32_t length) { return GetSequence(GetSequenceID(a_sSequenceName, length)); }

		short GetSequenceCount() { return m_sSequenceCount; }

	private:
		// Members complete
		uint32_t m_iSize;                              // 0x0
		short m_sBoneCount;                            // 0x4
		short m_sManualBoneCount;                      // 0x6
		short m_sSequenceCount;                        // 0x8
		short m_sAnimationMaxCount;                    // 0xA
		short m_sInstanceCount;                        // 0xC
		TKeyframeLibraryInstance m_KeyLibraryInstance; // 0x10
		TSkeletonBone* m_Bones;                        // 0x38
		TSkeletonSequence** m_SkeletonSequences;       // 0x3C
	};
}


