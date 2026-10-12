#include "ToshiPCH.h"
#include "TAnimation.h"
#include "TSkeleton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006caea0
TBOOL TAnimation::UpdateTime( TFLOAT a_fDeltaTime )
{
	TSkeleton*         pSkeleton = m_pSkeletonInstance->GetSkeleton();
	TSkeletonSequence* pSeq      = pSkeleton->GetSequence( m_iSeqID );

	TFLOAT fSeqDuration = pSeq->GetDuration();

	// The total time always grows, even when playing backwards
	m_fTotalTime += TMath::Abs( m_fSpeed * a_fDeltaTime );
	m_fSeqTime += m_fSpeed * a_fDeltaTime;

	if ( pSeq->GetMode() == TSkeletonSequence::MODE_LOOPED && !HASANYFLAG( m_eFlags, Flags_Managed ) )
	{
		if ( 0.0f <= m_fSpeed || 0.0f <= m_fSeqTime )
		{
			if ( fSeqDuration <= m_fSeqTime )
			{
				// Make m_fSeqTime less than animation duration
				m_fSeqTime -= TMath::FloorToInt( m_fSeqTime / fSeqDuration ) * fSeqDuration;

				// Reset states of all bones
				for ( TINT i = 0; i < pSkeleton->GetAutoBoneCount(); i++ )
					*GetBone( i ) = 0;
			}
		}
		else
		{
			m_fSeqTime = fSeqDuration;
		}
	}
	else
	{
		if ( m_fSeqTime < 0.0f && m_fSpeed < 0.0f )
			m_fSeqTime = fSeqDuration;

		if ( m_fSeqTime >= 0.0f )
		{
			if ( fSeqDuration < m_fSeqTime )
			{
				m_fSeqTime = fSeqDuration;
			}
		}
		else
		{
			m_fSeqTime = 0.0f;
		}
	}

	if ( m_eState == STATE_BLENDING_IN )
	{
		m_fWeight = ( m_fBlendInSpeed == 0.0f ) ? m_fDestWeight : m_fWeight + ( 1.0f / m_fBlendInSpeed ) * a_fDeltaTime;

		if ( m_fDestWeight <= m_fWeight )
		{
			m_fWeight = m_fDestWeight;
			m_eState  = STATE_PLAYING;
		}
	}
	else if ( m_eState == STATE_PLAYING )
	{
		if ( HASANYFLAG( m_eFlags, Flags_Managed ) && fSeqDuration <= m_fTotalTime )
		{
			m_eState = STATE_BLENDING_OUT;
			return TTRUE;
		}
	}
	else if ( m_eState == STATE_BLENDING_OUT )
	{
		m_fWeight = ( m_fBlendOutSpeed == 0.0f ) ? -1.0f : m_fWeight - ( 1.0f / m_fBlendOutSpeed ) * a_fDeltaTime;

		if ( m_fWeight <= 0.0f )
		{
			m_pSkeletonInstance->RemoveAnimation( this, 0.0f );
			return HASANYFLAG( m_eFlags, Flags_UpdateStateOnRemove );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006cb0c0
void TAnimation::RemoveAnimation( TFLOAT a_fVal )
{
	if ( m_pSkeletonInstance )
		m_pSkeletonInstance->RemoveAnimation( this, a_fVal );
}

// $Barnyard: FUNCTION 006ca2d0
TFLOAT TAnimation::SetDestWeight( TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed )
{
	TFLOAT fOldValue = m_fDestWeight;

	m_fDestWeight   = a_fDestWeight;
	m_fBlendInSpeed = a_fBlendInSpeed;

	if ( 0.0f < a_fBlendInSpeed )
	{
		m_eState = STATE_BLENDING_IN;
	}
	else
	{
		m_fWeight = a_fDestWeight;
		m_eState  = STATE_PLAYING;
	}

	return fOldValue;
}

TSkeletonSequence* TAnimation::GetSequencePtr() const
{
	return m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID );
}

TAnimation::TAnimation()
{
}

TAnimation::~TAnimation()
{
}

// $Barnyard: FUNCTION 006ca230
void TAnimation::ChangeToManaged( TFLOAT a_fBlendOutSpeed )
{
	if ( IsActive() && !IsManaged() )
	{
		TFLOAT fSeqDuration = m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID )->GetDuration();

		m_eFlags |= Flags_Managed;
		m_fBlendOutSpeed = a_fBlendOutSpeed;
		m_fSeqTime       = m_fSeqTime - TMath::FloorToInt( m_fSeqTime / fSeqDuration ) * fSeqDuration;
		m_fTotalTime     = m_fSeqTime;
	}
}


// $Barnyard: FUNCTION 006ca1d0
TBOOL TAnimation::IsFinishing( TFLOAT a_fDeltaTime ) const
{
	if ( !IsManaged() || m_eState != STATE_BLENDING_OUT )
		return TFALSE;

	if ( 0.0f == m_fBlendOutSpeed )
		return TTRUE;

	return m_fWeight - 1.0f / m_fBlendOutSpeed * a_fDeltaTime <= 0.0f;
}

// $Barnyard: FUNCTION 006ca310
TFLOAT TAnimation::SetTime( TFLOAT a_fTime )
{
	TFLOAT             fOldTime  = m_fTotalTime;
	TSkeletonSequence* pSequence = m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID );

	m_fTotalTime = a_fTime;
	m_fSeqTime   = a_fTime;

	if ( pSequence->GetMode() != TSkeletonSequence::MODE_LOOPED || IsManaged() )
	{
		if ( a_fTime >= 0.0f )
		{
			if ( a_fTime > pSequence->GetDuration() )
				m_fSeqTime = pSequence->GetDuration();
		}
		else
		{
			m_fSeqTime = 0.0f;
		}
	}
	else
	{
		TFLOAT fDuration = pSequence->GetDuration();
		m_fSeqTime       = m_fSeqTime - TMath::FloorToInt( a_fTime / fDuration ) * fDuration;
	}

	return fOldTime;
}

TOSHI_NAMESPACE_END
