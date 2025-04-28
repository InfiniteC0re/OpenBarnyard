#include "ToshiPCH.h"
#include "TSkeleton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TSkeleton::TSkeleton()
{
	m_iBoneCount         = 0;
	m_iManualBoneCount   = 0;
	m_iSequenceCount     = 0;
	m_iAnimationMaxCount = 0;
	m_iInstanceCount     = 0;
	m_pBones             = TNULL;
	m_SkeletonSequences  = TNULL;
}

TSkeleton::~TSkeleton()
{
}

// $Barnyard: FUNCTION 006ca760
TSkeletonInstance* TSkeleton::CreateInstance( TBOOL a_bSetBasePose )
{
	// Set quaternion interpolation method if it's not already set
	if ( m_eQuatLerpType <= QUATINTERP_Nlerp )
		SetQInterpFn( m_eQuatLerpType );

	auto               iAutoBoneCount = GetAutoBoneCount();
	TSIZE              iAnimationSize = TAlignNumUp( iAutoBoneCount * sizeof( TAnimationBone ) + sizeof( TAnimation ) );
	TSIZE              iInstanceSize  = sizeof( TSkeletonInstance ) + sizeof( TSkeletonInstanceBone ) * iAutoBoneCount + iAnimationSize * GetAnimationMaxCount();
	TSkeletonInstance* pInstance;

	pInstance = TSTATICCAST( TSkeletonInstance, TMalloc( iInstanceSize ) );

	new ( pInstance ) TSkeletonInstance();
	pInstance->m_pSkeleton              = this;
	pInstance->m_iSize                  = iInstanceSize;
	pInstance->m_iBaseAnimationCount    = 0;
	pInstance->m_iOverlayAnimationCount = 0;
	pInstance->m_eFlags                 = 0;
	pInstance->m_pBones                 = TREINTERPRETCAST( TSkeletonInstanceBone*, pInstance + 1 );
	pInstance->m_pAnimations            = TREINTERPRETCAST( TAnimation*, pInstance->m_pBones + iAutoBoneCount );
	pInstance->m_fTotalWeight           = 0.0f;
	pInstance->m_iLastUpdateStateFrame  = 0;
	pInstance->m_iLastUpdateTimeFrame   = 0;

	for ( TINT i = 0; i < GetAnimationMaxCount(); i++ )
	{
		TAnimation* pAnimation = TREINTERPRETCAST(
		    TAnimation*,
		    TREINTERPRETCAST( TUINTPTR, pInstance->m_pAnimations ) + i * iAnimationSize
		);

		new ( pAnimation ) TAnimation();
		pInstance->m_FreeAnimations.PushFront( pAnimation );
	}

	if ( a_bSetBasePose )
		pInstance->SetStateFromBasePose();

	m_iInstanceCount++;
	return pInstance;
}

void TSkeleton::SetQInterpFn( QUATINTERP a_eQuatInterp )
{
	if ( a_eQuatInterp == QUATINTERP_Slerp )
		m_fnQuatLerp = TQuaternion::Slerp;
	else
		m_fnQuatLerp = TQuaternion::Nlerp;
}

// $Barnyard: FUNCTION 006c9d10
TINT TSkeleton::GetBoneID( const TCHAR* a_szBoneName, TUINT32 a_iLength )
{
	if ( a_iLength == 0 )
		a_iLength = TStringManager::String8Length( a_szBoneName );

	for ( short i = 0; i < m_iBoneCount; i++ )
	{
		if ( m_pBones[ i ].GetNameLength() == a_iLength &&
		     TStringManager::String8CompareNoCase( m_pBones[ i ].GetName(), a_szBoneName, a_iLength ) == 0 )
		{
			return i;
		}
	}

	return TBONE_INVALID;
}

// $Barnyard: FUNCTION 006c9d80
TINT TSkeleton::GetSequenceID( const TCHAR* a_szSequenceName, TUINT32 a_iLength )
{
	if ( a_iLength == 0 )
		a_iLength = TStringManager::String8Length( a_szSequenceName );

	for ( short i = 0; i < m_iSequenceCount; i++ )
	{
		if ( m_SkeletonSequences[ i ].GetNameLength() == a_iLength &&
		     TStringManager::String8CompareNoCase( m_SkeletonSequences[ i ].GetName(), a_szSequenceName, a_iLength ) == 0 )
		{
			return i;
		}
	}

	return TSEQ_INVALID;
}

// $Barnyard: FUNCTION 006cb420
void TSkeletonInstance::UpdateTime( TFLOAT a_fDeltaTime )
{
	if ( !m_BaseAnimations.IsEmpty() || ( !m_OverlayAnimations.IsEmpty() && m_iLastUpdateTimeFrame != g_oSystemManager.GetFrameCount() ) )
	{
		m_iLastUpdateTimeFrame = g_oSystemManager.GetFrameCount();
		m_fTotalWeight         = 0.0f;

		// Update base animations
		T2_FOREACH( m_BaseAnimations, pAnim )
		{
			if ( pAnim->UpdateTime( a_fDeltaTime ) )
			{
				m_fTotalWeight += pAnim->GetWeight();
			}
		}

		// Update overlay animations
		T2_FOREACH( m_OverlayAnimations, pAnim )
		{
			pAnim->UpdateTime( a_fDeltaTime );
		}

		if ( HASANYFLAG( m_eFlags, 1 ) )
		{
			m_eFlags &= ~1;
			UpdateState( TFALSE );
		}
	}
}

// $Barnyard: FUNCTION 006ca960
// Note: the current implementation is taken from de Blob
void TSkeletonInstance::UpdateState( TBOOL a_bForceUpdate )
{
	if ( ( a_bForceUpdate || m_iLastUpdateStateFrame != g_oSystemManager.GetFrameCount() ) &&
	     m_pSkeleton->GetKeyLibraryInstance().GetLibrary() != TNULL )
	{
		m_iLastUpdateStateFrame = g_oSystemManager.GetFrameCount();

		const auto QInterpFn           = m_pSkeleton->GetQInterpFn();
		TFLOAT     fOneOverTotalWeight = 1.0f;

		if ( 1.0f < m_fTotalWeight )
			fOneOverTotalWeight = 1.0f / m_fTotalWeight;

		TASSERT( m_pSkeleton->GetAutoBoneCount() < TANIMATION_MAXBONES );

		for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
		{
			TASSERT( i < TANIMATION_MAXBONES );

			auto& rBoneCache = g_aBonesCaches[ i ];
			auto  pBone      = m_pSkeleton->GetBone( i );

			if ( m_BaseAnimations.IsEmpty() )
			{
				rBoneCache.Rotation = pBone->GetRotation();
				rBoneCache.Position = pBone->GetPosition();
			}
			else
			{
				TFLOAT fWeightTotalRatio = 0.0f;
				TBOOL  bBoneHasState     = TFALSE;

				T2_FOREACH( m_BaseAnimations, it )
				{
					auto pSeq     = m_pSkeleton->GetSequence( it->GetSequence() );
					auto pSeqBone = pSeq->GetBone( i );

					TINT iCurrentKeyframePos = TINT( ( it->GetSeqTime() / pSeq->GetDuration() ) * 65535 );

					TUINT16 iLerpFromIndex;
					TUINT16 iLerpToIndex;
					TFLOAT  fLerpProgress = pSeqBone->GetKeyPair( iCurrentKeyframePos, *it->GetBone( i ), iLerpFromIndex, iLerpToIndex );

					TFLOAT fWeightRatio = it->GetWeight() * fOneOverTotalWeight;

					if ( fWeightRatio > 0.0f && pSeqBone->GetKeyCount() != 0 )
					{
						auto pFromKey = pSeqBone->GetKey( iLerpFromIndex );
						auto pToKey   = pSeqBone->GetKey( iLerpToIndex );

						auto& rKeyLibrary = m_pSkeleton->GetKeyLibraryInstance();
						auto  pFromQuat   = rKeyLibrary.GetQ( pFromKey[ 1 ] );
						auto  pToQuat     = rKeyLibrary.GetQ( pToKey[ 1 ] );

						if ( it == m_BaseAnimations.Head() )
						{
							bBoneHasState     = TTRUE;
							fWeightTotalRatio = fWeightRatio;
							QInterpFn( rBoneCache.Rotation, *pFromQuat, *pToQuat, fLerpProgress );

							if ( pSeqBone->IsTranslateAnimated() )
							{
								auto pFromTranslation = rKeyLibrary.GetT( pFromKey[ 2 ] );
								auto pToTranslation   = rKeyLibrary.GetT( pToKey[ 2 ] );

								rBoneCache.Position.Lerp( *pFromTranslation, *pToTranslation, fLerpProgress );
							}
							else
							{
								rBoneCache.Position = pBone->GetPosition();
							}
						}
						else
						{
							TVector4    position;
							TQuaternion rotation;

							QInterpFn( rotation, *pFromQuat, *pToQuat, fLerpProgress );

							if ( pSeqBone->IsTranslateAnimated() )
							{
								auto pFromTranslation = rKeyLibrary.GetT( pFromKey[ 2 ] );
								auto pToTranslation   = rKeyLibrary.GetT( pToKey[ 2 ] );

								position.Lerp3( *pFromTranslation, *pToTranslation, fLerpProgress );
							}

							fWeightTotalRatio += fWeightRatio;
							QInterpFn( rBoneCache.Rotation, rBoneCache.Rotation, rotation, fWeightRatio / fWeightTotalRatio );

							const TVector3* pLerpToVec = pSeqBone->IsTranslateAnimated() ? &position.AsVector3() : &pBone->GetPosition();
							rBoneCache.Position.Lerp( *pLerpToVec, fWeightRatio / fWeightTotalRatio );
						}
					}
				}

				if ( !bBoneHasState )
				{
					rBoneCache.Rotation = pBone->GetRotation();
					rBoneCache.Position = pBone->GetPosition();
				}
			}

			T2_FOREACH( m_OverlayAnimations, it )
			{
				if ( it->GetWeight() != 0.0f )
				{
					auto pSeq     = m_pSkeleton->GetSequence( it->GetSequence() );
					auto pSeqBone = pSeq->GetBone( i );

					if ( !pSeqBone->IsOverlayAnimated() && pSeqBone->GetKeyCount() != 0 )
					{
						TINT iCurrentKeyframePos = TINT( ( it->GetSeqTime() / pSeq->GetDuration() ) * 65535 );

						TUINT16 iLerpFromIndex;
						TUINT16 iLerpToIndex;
						TFLOAT  fLerpProgress = pSeqBone->GetKeyPair( iCurrentKeyframePos, *it->GetBone( i ), iLerpFromIndex, iLerpToIndex );

						auto pFromKey = pSeqBone->GetKey( iLerpFromIndex );
						auto pToKey   = pSeqBone->GetKey( iLerpToIndex );

						auto& rKeyLibrary = m_pSkeleton->GetKeyLibraryInstance();
						auto  pFromQuat   = rKeyLibrary.GetQ( pFromKey[ 1 ] );
						auto  pToQuat     = rKeyLibrary.GetQ( pToKey[ 1 ] );

						TVector4    position;
						TQuaternion rotation;

						QInterpFn( rotation, rBoneCache.Rotation, *pToQuat, fLerpProgress );
						QInterpFn( rBoneCache.Rotation, rBoneCache.Rotation, rotation, it->GetWeight() );

						const TVector3* pLerpToVec;
						if ( pSeqBone->IsTranslateAnimated() )
						{
							auto pFromTranslation = rKeyLibrary.GetT( pFromKey[ 2 ] );
							auto pToTranslation   = rKeyLibrary.GetT( pToKey[ 2 ] );

							position.Lerp3( *pFromTranslation, *pToTranslation, fLerpProgress );
							pLerpToVec = &position.AsVector3();
						}
						else
						{
							pLerpToVec = &pBone->GetPosition();
						}

						rBoneCache.Position.Lerp( *pLerpToVec, it->GetWeight() );
					}
				}
			}

			// Save the transform
			auto& rMatrix     = g_aForwardMatrices[ i ];
			auto  iParentBone = pBone->GetParentBone();

			if ( iParentBone == TBONE_INVALID )
			{
				// No parent bone
				rMatrix.SetFromQuaternion( rBoneCache.Rotation );
				rMatrix.SetTranslation( rBoneCache.Position );
			}
			else
			{
				// Has parent bone
				TMatrix44 boneTransform;
				boneTransform.SetFromQuaternion( rBoneCache.Rotation );
				boneTransform.SetTranslation( rBoneCache.Position );
				rMatrix.Multiply( g_aForwardMatrices[ iParentBone ], boneTransform );
			}

			m_pBones[ i ].m_Transform.Multiply( rMatrix, pBone->GetTransformInv() );
		}
	}
}

// $Barnyard: FUNCTION 006c9ec0
TMatrix44& TSkeletonInstance::GetBoneTransformCurrent( TINT a_iBone, TMatrix44& a_rOutTransform )
{
	if ( a_iBone < m_pSkeleton->GetAutoBoneCount() )
	{
		auto pBone = m_pSkeleton->GetBone( a_iBone );
		a_rOutTransform.Multiply( m_pBones[ a_iBone ].m_Transform, pBone->GetTransform() );

		return a_rOutTransform;
	}

	a_rOutTransform = m_pSkeleton->GetBone( a_iBone )->GetTransform();

	return a_rOutTransform;
}

// $Barnyard: FUNCTION 006c9f20
const TMatrix44& TSkeletonInstance::GetBoneTransformCurrent( TINT a_iBone )
{
	static TMatrix44 s_Transform;

	GetBoneTransformCurrent( a_iBone, s_Transform );
	return s_Transform;
}

// $Barnyard: FUNCTION 006ca860
void TSkeletonInstance::RemoveAnimation( TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed )
{
	if ( !a_pAnimation )
		return;

	TASSERT( TTRUE == a_pAnimation->IsActive() );

	if ( a_fBlendOutSpeed > 0.0f )
	{
		a_pAnimation->m_fDestWeight    = 0.0f;
		a_pAnimation->m_fBlendOutSpeed = a_fBlendOutSpeed;
		a_pAnimation->m_eState         = TAnimation::STATE_BLENDING_OUT;
	}
	else if ( a_pAnimation->m_eFlags & TAnimation::Flags_UpdateStateOnRemove )
	{
		a_pAnimation->m_eFlags &= ~TAnimation::Flags_UpdateStateOnRemove;
		a_pAnimation->m_fDestWeight    = 1.0f;
		a_pAnimation->m_fBlendOutSpeed = 1.0f;
		a_pAnimation->m_eState         = TAnimation::STATE_BLENDING_OUT;
		m_eFlags |= 1;
	}
	else
	{
		if ( a_pAnimation->GetSequencePtr()->IsOverlay() )
			m_iOverlayAnimationCount -= 1;
		else
			m_iBaseAnimationCount -= 1;

		a_pAnimation->m_eFlags = TAnimation::Flags_None;

		// Unlink the animation and add it to the list of free animations
		a_pAnimation->Remove();
		m_FreeAnimations.PushFront( a_pAnimation );
	}
}

// $Barnyard: FUNCTION 006c9e20
void TSkeletonInstance::SetStateFromBasePose()
{
	for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
	{
		m_pBones[ i ].m_Rotation = m_pSkeleton->GetBone( i )->GetRotation();
		m_pBones[ i ].m_Position = m_pSkeleton->GetBone( i )->GetPosition();
		m_pBones[ i ].m_Transform.Identity();
	}
}

void TSkeletonInstance::Delete()
{
	delete this;
}

// $Barnyard: FUNCTION 006cb3a0
void TSkeletonInstance::RemoveAllAnimations()
{
	while ( !m_BaseAnimations.IsEmpty() )
		RemoveAnimation( m_BaseAnimations.Begin(), 0.0f );

	while ( !m_OverlayAnimations.IsEmpty() )
		RemoveAnimation( m_OverlayAnimations.Begin(), 0.0f );
}

TSkeletonInstance::~TSkeletonInstance()
{
	m_pSkeleton->m_iInstanceCount--;
}

// $Barnyard: FUNCTION 006ca920
TAnimation* TSkeletonInstance::GetAnimation( TUINT16 a_iSeqId )
{
	T2_FOREACH( m_BaseAnimations, it )
	{
		if ( it->GetSequence() == a_iSeqId )
			return it;
	}

	T2_FOREACH( m_OverlayAnimations, it )
	{
		if ( it->GetSequence() == a_iSeqId )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006cb190
TAnimation* TSkeletonInstance::AddAnimationFull( TUINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TAnimation::Flags a_eFlags )
{
	if ( m_pSkeleton->GetSequenceCount() <= a_iSequenceIndex )
		return TNULL;

	TAnimation* pAnimation = TNULL;

	// Check if this animation is already playing as a base animation
	for ( auto it = m_BaseAnimations.Begin(); pAnimation != TNULL && it != m_BaseAnimations.End(); ++it )
	{
		if ( it->GetSequence() == a_iSequenceIndex )
			pAnimation = it;
	}

	// Check if this animation is already playing as an overlay animation
	for ( auto it = m_BaseAnimations.Begin(); pAnimation != TNULL && it != m_BaseAnimations.End(); ++it )
	{
		if ( it->GetSequence() == a_iSequenceIndex )
			pAnimation = it;
	}

	const TBOOL bIsNewAnim = pAnimation == TNULL;

	if ( bIsNewAnim )
	{
		// Get a free animation from the list
		pAnimation = m_FreeAnimations.IsEmpty() ? TNULL : m_FreeAnimations.Begin();

		// If the free list is empty, replace the most unimportant animation with it
		if ( !pAnimation )
		{
			TQList<TAnimation>* pIterateList = &m_BaseAnimations;

			// If no base animations are currently playing, get one from the overlay animations list
			if ( m_BaseAnimations.IsEmpty() )
				pIterateList = &m_OverlayAnimations;

			TASSERT( !pIterateList->IsEmpty() );
			pAnimation = pIterateList->Begin();

			// Find an animation that has the most least weight
			for ( auto it = pIterateList->Begin().Next(); it != pIterateList->End(); it++ )
			{
				if ( it->GetWeight() < pAnimation->GetWeight() )
					pAnimation = it;
			}

			// Instantly remove the animation
			if ( TSkeletonInstance* pSkeletonInstance = pAnimation->GetSkeletonInstance() )
				pSkeletonInstance->RemoveAnimation( pAnimation, 0.0f );
		}

		// Remove animation from any list
		pAnimation->TQList<TAnimation>::TNode::Remove();

		if ( m_pSkeleton->GetSequence( a_iSequenceIndex )->IsBase() )
		{
			// Base Animation
			m_BaseAnimations.PushFront( pAnimation );
			m_iBaseAnimationCount++;
		}
		else
		{
			// Overlay Animation
			m_OverlayAnimations.PushFront( pAnimation );
			m_iOverlayAnimationCount++;
		}
	}

	// Clamp values
	TMath::Clip( a_fDestWeight, 0.0f, 1.0f );

	// Setup animation
	TVALIDPTR( pAnimation );
	pAnimation->m_iSeqID = a_iSequenceIndex;
	pAnimation->m_eFlags = a_eFlags | TAnimation::Flags_Active;
	pAnimation->m_iUnk3  = 0;
	pAnimation->m_fSpeed = 1.0f;

	if ( bIsNewAnim ) pAnimation->m_fWeight = 0.0f;
	TMath::Clip( pAnimation->m_fWeight, 0.0f, 1.0f );

	pAnimation->m_fBlendOutSpeed = a_fBlendOutSpeed;
	pAnimation->m_fDestWeight    = a_fDestWeight;
	pAnimation->m_fTotalTime     = 0.0f;
	pAnimation->m_fSeqTime       = 0.0f;
	pAnimation->m_fBlendInSpeed  = a_fBlendInSpeed;

	if ( a_fBlendInSpeed <= 0.0f )
	{
		pAnimation->m_fWeight = a_fDestWeight;
		pAnimation->m_eState  = TAnimation::STATE_PLAYING;
	}
	else
	{
		pAnimation->m_eState = TAnimation::STATE_BLENDING_IN;
	}

	// Clear auto bones data
	TUtil::MemSet( pAnimation->GetBones(), 0, m_pSkeleton->GetAutoBoneCount() * sizeof( TAnimationBone ) );

	pAnimation->m_pSkeletonInstance = this;
	return pAnimation;
}

// $Barnyard: FUNCTION 006cb4d0
TAnimation* TSkeletonInstance::AddAnimation( TUINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed )
{
	return AddAnimationFull( a_iSequenceIndex, a_fDestWeight, a_fBlendInSpeed, 0.0f, 0 );
}

const TSkeletonInstanceBone& TSkeletonInstance::GetBone( TINT a_uiIndex )
{
	TASSERT( a_uiIndex < m_pSkeleton->GetAutoBoneCount() );
	return m_pBones[ a_uiIndex ];
}

TSkeletonInstance::TSkeletonInstance()
{
}

// $Barnyard: FUNCTION 006c9f70
TFLOAT TSkeletonSequenceBone::GetKeyPair( TINT a_iCurrentAnimTime, TUINT16& a_rCurrentKeyIndex, TUINT16& a_rLerpFromIndex, TUINT16& a_rLerpToIndex )
{
	auto pFirstKeyTime = *GetKey( 0 );

	if ( a_iCurrentAnimTime < pFirstKeyTime || a_iCurrentAnimTime == pFirstKeyTime )
	{
		// Animation haven't reached it's first frame
		a_rCurrentKeyIndex = 0;
		a_rLerpFromIndex   = 0;
		a_rLerpToIndex     = 0;
		return 0.0f;
	}

	auto iLastKeyIndex = m_iNumKeys - 1;
	auto pLastKeyTime  = *GetKey( iLastKeyIndex );

	if ( pLastKeyTime <= a_iCurrentAnimTime )
	{
		// Animation is over
		a_rCurrentKeyIndex = iLastKeyIndex;
		a_rLerpFromIndex   = iLastKeyIndex;
		a_rLerpToIndex     = iLastKeyIndex;
		return 0.0f;
	}

	auto pCurrentKeyTime = *GetKey( a_rCurrentKeyIndex );

	if ( pCurrentKeyTime < a_iCurrentAnimTime )
	{
		// Current key is currently lerping
		auto iNextIndex   = a_rCurrentKeyIndex + 1;
		auto iNextKeyTime = *GetKey( iNextIndex );

		while ( iNextKeyTime <= a_iCurrentAnimTime )
		{
			// Skip keys that are already over
			a_rCurrentKeyIndex = iNextIndex++;
			iNextKeyTime       = *GetKey( iNextIndex );
		}

		a_rLerpFromIndex = a_rCurrentKeyIndex;
		a_rLerpToIndex   = iNextIndex;
	}
	else
	{
		if ( pCurrentKeyTime == a_iCurrentAnimTime )
		{
			// Current time is right at the end of keys transition
			a_rLerpFromIndex = a_rCurrentKeyIndex;
			a_rLerpToIndex   = a_rCurrentKeyIndex;
			return 0.0f;
		}

		// The animation is playing backwards?
		auto iPrevIndex   = a_rCurrentKeyIndex - 1;
		auto iPrevKeyTime = *GetKey( iPrevIndex );

		while ( a_iCurrentAnimTime < iPrevKeyTime || a_iCurrentAnimTime == iPrevKeyTime )
		{
			a_rCurrentKeyIndex = iPrevIndex--;
			iPrevKeyTime       = *GetKey( iPrevIndex );
		}

		a_rLerpFromIndex = iPrevIndex;
		a_rLerpToIndex   = a_rCurrentKeyIndex;
	}

	auto iLerpFromTime = *GetKey( a_rLerpFromIndex );
	auto iLerpToTime   = *GetKey( a_rLerpToIndex );

	return ( ( a_iCurrentAnimTime - iLerpFromTime ) * ( 1.0f / 65535 ) ) / ( ( iLerpToTime - iLerpFromTime ) * ( 1.0f / 65535 ) );
}

TOSHI_NAMESPACE_END
