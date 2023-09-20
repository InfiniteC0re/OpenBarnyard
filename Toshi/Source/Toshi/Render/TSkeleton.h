#pragma once
#include "Toshi/Core/TQList.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Render/TAnimation.h"
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

		__forceinline uint16_t* GetKey(size_t a_iKeyIndex)
		{
			return TREINTERPRETCAST(uint16_t*, a_iKeyIndex * m_iKeySize + TREINTERPRETCAST(uintptr_t, m_pData));
		}

		__forceinline unsigned short GetKeyCount() const
		{
			return m_iNumKeys;
		}

		__forceinline TBOOL IsTranslateAnimated() const
		{
			return m_eFlags & 1;
		}

		__forceinline TBOOL Is2() const
		{
			return m_eFlags & 2;
		}

		__forceinline uint8_t GetFlags() const
		{
			return m_eFlags;
		}

	private:
		uint8_t m_eFlags;
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
		TBOOL IsOverlay() const { return m_eFlags & FLAG_Overlay; }

		TSkeletonSequenceBone* GetBones() { return m_pSeqBones; }
		TSkeletonSequenceBone* GetBone(int a_iIndex) { return &m_pSeqBones[a_iIndex]; }

		float GetDuration() const { return m_fDuration; }
		const char* GetName() const { return m_szName; }
		short GetUnk2() const { return m_iUnk2; }
		int GetUnk3() const { return m_iUnk3; }

	private:
		const char* m_szName;
		Flag m_eFlags;
		short m_iUnk2;
		LOOPMODE m_eLoopMode;
		float m_fDuration;
		TSkeletonSequenceBone* m_pSeqBones;
		int m_iUnk3;
	};

	class TSkeletonBone
	{
	public:
		const TQuaternion& GetRotation() const { return m_Rotation; }
		const TMatrix44& GetTransform() const { return m_Transform; }
		const TMatrix44& GetTransformInv() const { return m_TransformInv; }
		const TVector3& GetPosition() const { return m_Position; }
		const char* GetName() const { return m_szName; }
		int GetParentBone() const { return m_iParentBone; }

	private:
		TQuaternion m_Rotation;
		TMatrix44 m_Transform;
		TMatrix44 m_TransformInv;
		const char* m_szName;
		short m_iParentBone;
		TVector3 m_Position;
		char pad[12];
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

		t_fnQuatLerp GetQInterpFn() const { return m_fnQuatLerp; }
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
		TSkeletonSequence* GetSequence(short a_iSeqID) { TASSERT(a_iSeqID < m_iSequenceCount); return &m_SkeletonSequences[a_iSeqID]; }
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

	using TSkeletonInstanceBone = TMatrix44;

	class TSkeletonInstance
	{
	public:
		struct BoneCache
		{
			TQuaternion Rotation;
			TVector3 Position;
		};
		
		friend TSkeleton;

	public:
		TSkeletonInstance() = default;

		void UpdateTime(float a_fDeltaTime);
		void UpdateState(TBOOL a_bForceUpdate);
		TMatrix44* GetBoneTransformCurrent(int a_iBone, TMatrix44& a_rMatrix);

		TAnimation* AddAnimation(short a_iSequenceIndex, float a_fDestWeight, float a_fBlendInSpeed)
		{
			return AddAnimationFull(a_iSequenceIndex, a_fDestWeight, a_fBlendInSpeed, 0.0f, 0);
		}

		TAnimation* AddAnimationFull(short a_iSequenceIndex, float a_fDestWeight, float a_fBlendInSpeed, float a_fBlendOutSpeed, TAnimation::Flags a_eAnimFlags);
		void RemoveAnimation(TAnimation* a_pAnimation, float a_fBlendOutSpeed);

		void SetStateFromBasePose();
		TAnimation* IsAnimating(short a_iSequenceIndex);

		TSkeleton* GetSkeleton() { return m_pSkeleton; }
		int GetSequenceMaxUnk3() const { return m_iSequenceMaxUnk3; }
		void SetSequenceMaxUnk3(int a_iSequenceMaxUnk3) { m_iSequenceMaxUnk3 = a_iSequenceMaxUnk3; }

	public:
		inline static TMatrix44 g_aForwardMatrices[TANIMATION_MAXBONES];
		inline static BoneCache g_aBonesCaches[TANIMATION_MAXBONES];

	private:
		int m_iFlags;
		uint32_t m_iSize;
		TSkeleton* m_pSkeleton;
		short m_iBaseAnimationCount;
		short m_iOverlayAnimationCount;
		TQList<TAnimation> m_BaseAnimations;
		TQList<TAnimation> m_OverlayAnimations;
		TQList<TAnimation> m_FreeAnimations;
		TSkeletonInstanceBone* m_pBones;
		TAnimation* m_pAnimations;
		float m_fTotalWeight;
		int m_iLastUpdateTimeFrame;
		int m_iLastUpdateStateFrame;
		int m_iSequenceMaxUnk3;
	};

}
