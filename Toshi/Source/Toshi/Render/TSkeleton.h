#pragma once
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Render/ResourceName.h"

namespace Toshi {

	class TSkeletonSequenceBone
	{
	public:
		/**
		 * Returns lerp progress coefficient that should be used for animating
		 * @param a_iCurrentAnimProgress The current progress of animation from 0 to 65535
		 * @param a_rCurrentKeyIndex The index of current frame
		 * @param a_rLerpFromIndex The index of key that is being interpolated from
		 * @param a_rLerpToIndex The index of key to which previous is being interpolated to
		 */
		float GetKeyPair(int a_iCurrentAnimProgress, unsigned short& a_rCurrentKeyIndex, unsigned short& a_rLerpFromIndex, unsigned short& a_rLerpToIndex);

		__forceinline uint16_t* GetKeyData(size_t a_iKeyIndex)
		{
			return TREINTERPRETCAST(uint16_t*, a_iKeyIndex * m_iKeySize + TREINTERPRETCAST(uintptr_t, m_pData));
		}

	private:
		uint8_t m_eChannelMode;
		uint8_t m_iKeySize;
		unsigned short m_iNumKeys;
		uint16_t* m_pData;
	};

	class TSkeletonSequence
	{
	public:
		enum Flag : unsigned short
		{
			FLAG_Overlay = BITFIELD(0),
		};

		enum LOOPMODE : int
		{
			LOOPMODE_LOOP
		};

	public:
		TBOOL IsLooping() const { return m_eLoopMode == LOOPMODE_LOOP; }
		TBOOL IsOverlay() const { return m_Flags & FLAG_Overlay; }

		float GetDuration() const { return m_fDuration; }
		const char* GetName() const { return m_szName; }
		short GetUnk2() const { return m_iUnk2; }

	private:
		const char* m_szName;
		Flag m_Flags;
		short m_iUnk2;
		LOOPMODE m_eLoopMode;
		float m_fDuration;
		TSkeletonSequenceBone* m_pSeqBones;
		int m_iUnk3;
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
		TSkeletonSequence* GetSequences() { return m_SkeletonSequences; }
		TSkeletonSequence* GetSequence(uint32_t a_iSequence) { return &m_SkeletonSequences[a_iSequence]; }
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
		TSkeletonSequence* m_SkeletonSequences;       // 0x38
		t_fnQuatLerp m_fnQuatLerp;                     // 0x3C
	};

	struct TSkeletonInstanceBone
	{
		TMatrix44 Matrix;
	};

	class TSkeletonInstance
	{
	private:
		friend TSkeleton;

	public:
		TSkeletonInstance() = default;

		void UpdateTime(float a_fDeltaTime);
		void UpdateState(TBOOL a_bForceUpdate);

		void RemoveAnimation(TAnimation* a_pAnimation, float a_fValue);

		void SetStateFromBasePose();

		TSkeleton* GetSkeleton() { return m_pSkeleton; }

	private:
		int m_iFlags;
		uint32_t m_iSize;
		TSkeleton* m_pSkeleton;
		short m_iBaseAnimationCount;
		short m_iOverlayAnimationCount;
		TDList<TAnimation> m_BaseAnimations;    // FIXME: Should be TQList
		TDList<TAnimation> m_OverlayAnimations; // FIXME: Should be TQList
		TDList<TAnimation> m_FreeAnimations;    // FIXME: Should be TQList
		TSkeletonInstanceBone* m_pBones;
		TAnimation* m_pAnimations;
		float m_fUnk3;
		int m_iUnk4;
		int m_iCurrentFrame;
		int m_iUnk5;
	};

}
