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

void TSkeleton::Delete()
{
	if ( m_KeyLibraryInstance.GetLibrary() != TNULL )
	{
		auto& instance = GetKeyLibraryInstance();
		instance.SetSCount( instance.GetSCount() - 1 );
		instance.SetLibrary( TNULL );
	}
}

TBOOL TSkeleton::Create( TUINT32 param_1 )
{
	TIMPLEMENT();
	return TFALSE;
}

// $Barnyard: FUNCTION 006ca760
TSkeletonInstance* TSkeleton::CreateInstance( TBOOL a_bSetBasePose )
{
	// Set quaternion interpolation method if it's not already set
	if ( (QUATINTERP)m_fnQuatLerp <= QUATINTERP_Nlerp )
	{
		SetQInterpFn( (QUATINTERP)m_fnQuatLerp );
	}

	auto               iAutoBoneCount = GetAutoBoneCount();
	TSIZE              iAnimationSize = iAutoBoneCount * sizeof( TAnimationBone ) + TAlignNumDown( sizeof( TAnimation ) );
	TSIZE              iInstanceSize  = sizeof( TSkeletonInstance ) + sizeof( TSkeletonInstanceBone ) * iAutoBoneCount + iAnimationSize * GetAnimationMaxCount();
	TSkeletonInstance* pInstance;

	pInstance = TSTATICCAST( TSkeletonInstance, TMalloc( iInstanceSize ) );

	new ( pInstance ) TSkeletonInstance();
	pInstance->m_pSkeleton              = this;
	pInstance->m_iSize                  = iInstanceSize;
	pInstance->m_iBaseAnimationCount    = 0;
	pInstance->m_iOverlayAnimationCount = 0;
	pInstance->m_eFlags                 = 0;
	pInstance->m_pBones                 = TREINTERPRETCAST( TSkeletonInstanceBone*, this + 1 );
	pInstance->m_pAnimations            = TREINTERPRETCAST( TAnimation*, this + 1 ) + iAutoBoneCount;
	pInstance->m_fTotalWeight           = 0.0f;
	pInstance->m_iLastUpdateStateFrame  = 0;
	pInstance->m_iLastUpdateTimeFrame   = 0;

	for ( TINT i = 0; i < GetAnimationMaxCount(); i++ )
	{
		TAnimation* pAnimation = TREINTERPRETCAST(
		    TAnimation*,
		    TREINTERPRETCAST( uintptr_t, pInstance->m_pAnimations ) + i * iAnimationSize
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

// $Barnyard: FUNCTION 00610e40
TINT TSkeleton::GetBoneID( const TCHAR* a_szBoneName, TUINT32 a_iLength )
{
	if ( a_iLength == 0 )
		a_iLength = TStringManager::String8Length( a_szBoneName );

	for ( short i = 0; i < m_iBoneCount; i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_pBones[ i ].GetName(), a_szBoneName, a_iLength ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

// $Barnyard: FUNCTION 006c9d80
TINT TSkeleton::GetSequenceID( const TCHAR* a_szSequenceName, TUINT32 a_iLength )
{
	if ( a_iLength == 0 )
		a_iLength = TStringManager::String8Length( a_szSequenceName );

	for ( short i = 0; i < m_iSequenceCount; i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_SkeletonSequences[ i ].GetName(), a_szSequenceName, a_iLength ) == 0 )
		{
			return i;
		}
	}

	return -1;
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
				if ( it->GetWeight() != 0 )
				{
					auto pSeq     = m_pSkeleton->GetSequence( it->GetSequence() );
					auto pSeqBone = pSeq->GetBone( i );

					if ( !pSeqBone->Is2() && pSeqBone->GetKeyCount() != 0 )
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
		}

		for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
		{
			auto& rBoneCache = g_aBonesCaches[ i ];
			auto& rMatrix    = g_aForwardMatrices[ i ];

			auto pBone       = m_pSkeleton->GetBone( i );
			auto iParentBone = pBone->GetParentBone();

			if ( iParentBone == -1 )
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
TMatrix44* TSkeletonInstance::GetBoneTransformCurrent( TINT a_iBone, TMatrix44& a_rMatrix )
{
	if ( a_iBone < m_pSkeleton->GetAutoBoneCount() )
	{
		auto pBone = m_pSkeleton->GetBone( a_iBone );
		a_rMatrix.Multiply( m_pBones[ a_iBone ].m_Transform, pBone->GetTransform() );
		return &a_rMatrix;
	}

	a_rMatrix = m_pSkeleton->GetBone( a_iBone )->GetTransform();
	return &a_rMatrix;
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
		a_pAnimation->m_eState          = TAnimation::STATE_BLENDING_OUT;
		return;
	}

	if ( a_pAnimation->m_eFlags & TAnimation::Flags_UpdateStateOnRemove )
	{
		a_pAnimation->m_eFlags &= ~TAnimation::Flags_UpdateStateOnRemove;
		a_pAnimation->m_fDestWeight    = 1.0f;
		a_pAnimation->m_fBlendOutSpeed = 1.0f;
		a_pAnimation->m_eState         = TAnimation::STATE_BLENDING_OUT;
		m_eFlags |= 1;
		return;
	}

	if ( a_pAnimation->GetSequencePtr()->IsOverlay() )
		m_iOverlayAnimationCount -= 1;
	else
		m_iBaseAnimationCount -= 1;

	a_pAnimation->m_eFlags = TAnimation::Flags_None;

	// Unlink the animation and add it to the list of free animations
	a_pAnimation->Remove();
	m_FreeAnimations.PushFront( a_pAnimation );
}

void TSkeletonInstance::SetStateFromBasePose()
{
	for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
	{
		m_pSkeleton->GetBone( i );
		m_pBones[ i ].m_Transform.Identity();
	}
}

void TSkeletonInstance::Delete()
{
	m_pSkeleton->m_iInstanceCount--;
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
