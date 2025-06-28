#pragma once
#include "Toshi/TQList.h"
#include "Toshi/TSystem.h"

#include "Math/TQuaternion.h"
#include "Math/TMatrix44.h"
#include "Render/TAnimation.h"

#define TBONE_INVALID -1
#define TSEQ_INVALID  -1

TOSHI_NAMESPACE_START

class TSkeletonInstance;

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
	TFLOAT GetKeyPair( TINT a_iCurrentAnimProgress, TUINT16& a_rCurrentKeyIndex, TUINT16& a_rLerpFromIndex, TUINT16& a_rLerpToIndex );

	TFORCEINLINE TUINT16* GetKey( size_t a_iKeyIndex )
	{
		return TREINTERPRETCAST( TUINT16*, a_iKeyIndex * m_iKeySize + TREINTERPRETCAST( TUINTPTR, m_pData ) );
	}

	TFORCEINLINE TUINT16 GetKeyCount() const
	{
		return m_iNumKeys;
	}

	TFORCEINLINE TBOOL IsTranslateAnimated() const
	{
		return m_eFlags & 1;
	}

	TFORCEINLINE TBOOL IsOverlayAnimated() const
	{
		return m_eFlags & 2;
	}

	TFORCEINLINE TUINT8 GetFlags() const
	{
		return m_eFlags;
	}

public:
	TUINT8   m_eFlags;
	TUINT8   m_iKeySize;
	TUINT16  m_iNumKeys;
	TBYTE*   m_pData;
};

class TSkeletonSequence
{
public:
	enum Flag : TUINT16
	{
		FLAG_Overlay = BITFLAG( 0 ),
	};

public:
	TBOOL IsBase() const { return !( m_eFlags & FLAG_Overlay ); }
	TBOOL IsOverlay() const { return ( m_eFlags & FLAG_Overlay ); }

	TSkeletonSequenceBone* GetBones() { return m_pSeqBones; }
	TSkeletonSequenceBone* GetBone( TINT a_iIndex ) { return &m_pSeqBones[ a_iIndex ]; }

	TFLOAT       GetDuration() const { return m_fDuration; }
	TUINT8       GetNameLength() const { return m_iNameLength; }
	const TCHAR* GetName() const { return m_szName; }
	TINT16       GetUnk2() const { return m_iUnk2; }

public:
	TUINT8                 m_iNameLength;
	TCHAR                  m_szName[ 31 ];
	Flag                   m_eFlags;
	TINT16                 m_iUnk2;
	TINT                   m_iNumUsedBones;
	TFLOAT                 m_fDuration;
	TSkeletonSequenceBone* m_pSeqBones;
};

class TSkeletonBone
{
public:
	TQuaternion&       GetRotation() { return m_Rotation; }
	const TQuaternion& GetRotation() const { return m_Rotation; }

	TMatrix44&       GetTransform() { return m_Transform; }
	const TMatrix44& GetTransform() const { return m_Transform; }

	TMatrix44&      GetTransformInv() { return m_TransformInv; }
	const TVector3& GetPosition() const { return m_Position; }

	TUINT8       GetNameLength() const { return m_iNameLength; }
	const TCHAR* GetName() const { return m_szName; }

	TINT GetParentBone() const { return m_iParentBone; }

public:
	TQuaternion m_Rotation;
	TMatrix44   m_Transform;
	TMatrix44   m_TransformInv;
	TUINT8      m_iNameLength;
	TCHAR       m_szName[ 31 ];
	TINT16      m_iParentBone;
	TVector3    m_Position;
};

class TSkeleton
{
public:
	using t_fnQuatLerp = void ( * )( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress );

	using QUATINTERP = TUINT32;
	enum QUATINTERP_
	{
		QUATINTERP_Default,
		QUATINTERP_Slerp,
		QUATINTERP_Nlerp
	};

	friend TSkeletonInstance;

public:
	TSkeleton();
	~TSkeleton();

	TSkeletonInstance* CreateInstance( TBOOL a_bSetBasePose );

	t_fnQuatLerp GetQInterpFn() const { return m_fnQuatLerp; }
	void         SetQInterpFn( QUATINTERP a_eQuatInterp );

	TSkeletonBone* GetBone( const TCHAR* a_cBoneName, TUINT32 a_iLength = 0 ) { return GetBone( GetBoneID( a_cBoneName, a_iLength ) ); }
	TINT           GetBoneID( const TCHAR* a_cBoneName, TUINT32 a_iLength = 0 );

	TKeyframeLibraryInstance& GetKeyLibraryInstance() { return m_KeyLibraryInstance; }
	TINT                      GetAnimationMaxCount() { return m_iAnimationMaxCount; }
	TSkeletonBone*            GetBone( TINT a_iBone ) { return &m_pBones[ a_iBone ]; }
	TSkeletonBone*            GetBones() { return m_pBones; }
	TINT                      GetInstanceCount() const { return m_iInstanceCount; }
	TINT                      GetBoneCount() const { return m_iBoneCount; }
	TINT                      GetAutoBoneCount() const { return m_iBoneCount - m_iManualBoneCount; }

	TINT               GetSequenceID( const TCHAR* a_sSequenceName, TUINT32 a_iLength = 0 );
	TSkeletonSequence* GetSequences() { return m_SkeletonSequences; }
	TSkeletonSequence* GetSequence( TINT a_iSequence ) { return &m_SkeletonSequences[ a_iSequence ]; }
	TSkeletonSequence* GetSequence( const TCHAR* a_sSequenceName, TUINT32 a_iLength = 0 ) { return GetSequence( GetSequenceID( a_sSequenceName, a_iLength ) ); }

	TINT16 GetSequenceCount() { return m_iSequenceCount; }

public:
	TINT16                   m_iBoneCount;         // 0x00
	TINT16                   m_iManualBoneCount;   // 0x02
	TINT16                   m_iSequenceCount;     // 0x04
	TINT16                   m_iAnimationMaxCount; // 0x06
	TINT16                   m_iInstanceCount;     // 0x08
	TKeyframeLibraryInstance m_KeyLibraryInstance; // 0x0C
	TSkeletonBone*           m_pBones;             // 0x34
	TSkeletonSequence*       m_SkeletonSequences;  // 0x38

	union // 0x3C
	{
		t_fnQuatLerp m_fnQuatLerp;
		QUATINTERP   m_eQuatLerpType;
	};
};

struct TSkeletonInstanceBone
{
	TQuaternion m_Rotation;
	TVector3    m_Position;
	TMatrix44   m_Transform;
};

class TSkeletonInstance
{
public:
	struct BoneCache
	{
		TQuaternion Rotation;
		TVector3    Position;
	};

	friend TSkeleton;

public:
	TSkeletonInstance();
	~TSkeletonInstance();

	TAnimation* AddAnimation( TUINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed );
	TAnimation* AddAnimationFull( TUINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TAnimation::Flags a_eFlags );
	TAnimation* GetAnimation( TUINT16 a_iSeqId );

	void RemoveAnimation( TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed );
	void RemoveAllAnimations();

	void UpdateTime( TFLOAT a_fDeltaTime );
	void UpdateState( TBOOL a_bForceUpdate );

	const TMatrix44& GetBoneTransformCurrent( TINT a_iBone );
	TMatrix44&       GetBoneTransformCurrent( TINT a_iBone, TMatrix44& a_rOutTransform );
	void             SetStateFromBasePose();

	void Delete();

	TSkeleton*                   GetSkeleton() { return m_pSkeleton; }
	const TSkeletonInstanceBone* GetBones() { return m_pBones; }
	const TSkeletonInstanceBone& GetBone( TINT a_uiIndex );

	TBOOL IsAnyBaseAnimationPlaying() const { return !m_BaseAnimations.IsEmpty(); }
	TBOOL IsAnyOverlayAnimationPlaying() const { return !m_OverlayAnimations.IsEmpty(); }
	TBOOL IsAnyAnimationPlaying() const { return !m_BaseAnimations.IsEmpty() || !m_OverlayAnimations.IsEmpty(); }

public:
	inline static TMatrix44 g_aForwardMatrices[ TANIMATION_MAXBONES ];
	inline static BoneCache g_aBonesCaches[ TANIMATION_MAXBONES ];

private:
	TINT                   m_eFlags;
	TUINT32                m_iSize;
	TSkeleton*             m_pSkeleton;
	TINT16                 m_iBaseAnimationCount;
	TINT16                 m_iOverlayAnimationCount;
	TQList<TAnimation>     m_BaseAnimations;
	TQList<TAnimation>     m_OverlayAnimations;
	TQList<TAnimation>     m_FreeAnimations;
	TSkeletonInstanceBone* m_pBones;
	TAnimation*            m_pAnimations;
	TFLOAT                 m_fTotalWeight;
	TINT                   m_iLastUpdateTimeFrame;
	TINT                   m_iLastUpdateStateFrame;
};

TOSHI_NAMESPACE_END
