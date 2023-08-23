#pragma once
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Render/ResourceName.h"

namespace Toshi {

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
	public:
		const char* GetName() const { return m_szName; }

	private:
		TQuaternion m_Rotation;
		TMatrix44 m_Transform;
		TMatrix44 m_TransformInv;
		const char* m_szName;
		char pad[28];
	};

	class TSkeleton
	{
	public:
		using t_fnQuatLerp = void(*)(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, float a_fProgress);

		using QUATINTERP = uint32_t;
		enum QUATINTERP_
		{
			QUATINTERP_Default,
			QUATINTERP_Slerp,
			QUATINTERP_Nlerp
		};

	public:
		TSkeleton();
		
		void Delete();
		TBOOL Create(uint32_t param_1);

		class TSkeletonInstance* CreateInstance(TBOOL a_bSetBasePose);

		void SetQInterpFn(QUATINTERP a_eQuatInterp);

		TSkeletonBone* GetBone(const char* a_cBoneName, uint32_t a_iLength) { return GetBone(GetBoneID(a_cBoneName, a_iLength)); }
		int GetBoneID(const char* a_cBoneName, uint32_t a_iLength);

		TKeyframeLibraryInstance& GetKeyLibraryInstance() { return m_KeyLibraryInstance; }
		int GetAnimationMaxCount() { return m_iAnimationMaxCount; }
		TSkeletonBone* GetBone(int a_iBone) { return &m_pBones[a_iBone]; }
		TSkeletonBone* GetBones() { return m_pBones; }
		int GetInstanceCount() const { return m_iInstanceCount; }
		int GetBoneCount() const { return m_iBoneCount; }
		int GetAutoBoneCount() const { return m_iBoneCount - m_iManualBoneCount; }

		int GetSequenceID(const char* a_sSequenceName, uint32_t a_iLength);
		TSkeletonSequence** GetSequences() { return m_SkeletonSequences; }
		TSkeletonSequence* GetSequence(uint32_t a_iSequence) { return m_SkeletonSequences[a_iSequence]; }
		TSkeletonSequence* GetSequence(const char* a_sSequenceName, uint32_t a_iLength) { return GetSequence(GetSequenceID(a_sSequenceName, a_iLength)); }

		short GetSequenceCount() { return m_iSequenceCount; }

		TSkeleton& operator=(const TSkeleton& a_rSkeleton) { TIMPLEMENT(); }

	private:
		short m_iBoneCount;                            // 0x00
		short m_iManualBoneCount;                      // 0x02
		short m_iSequenceCount;                        // 0x04
		short m_iAnimationMaxCount;                    // 0x06
		short m_iInstanceCount;                        // 0x08
		TKeyframeLibraryInstance m_KeyLibraryInstance; // 0x0C
		TSkeletonBone* m_pBones;                       // 0x34
		TSkeletonSequence** m_SkeletonSequences;       // 0x38
		t_fnQuatLerp m_fnQuatLerp;                     // 0x3C
	};

	struct TSkeletonInstanceBone
	{
		TMatrix44 Matrix;
	};

#pragma pack(push, 1)
	class TAnimation : public TDList<TAnimation>::TNode
	{
	public:
		TAnimation()
		{
			m_iUnk2 = 0;
		}

	private:
		int m_iUnk1;
		short m_iSeqID;
		short m_iUnk2;
	};
#pragma pack(pop)

	class TSkeletonInstance
	{
	private:
		friend TSkeleton;

	public:
		TSkeletonInstance() = default;

		void SetStateFromBasePose();

	private:
		int m_iFlags;
		uint32_t m_iSize;
		TSkeleton* m_pSkeleton;
		short m_iBaseAnimationCount;
		short m_iOverlayAnimationCount;
		TDList<TAnimation> m_BaseAnimations;
		TDList<TAnimation> m_OverlayAnimations;
		TDList<TAnimation> m_FreeAnimations;
		TSkeletonInstanceBone* m_pBones;
		TAnimation* m_pAnimations;
		float m_fUnk3;
		int m_iUnk4;
		int m_iCurrentFrame;
		int m_iUnk5;
	};

}
