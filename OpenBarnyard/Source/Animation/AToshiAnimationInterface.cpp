#include "pch.h"
#include "AToshiAnimationInterface.h"
#include "AAnimatableObjectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00582690
AToshiAnimationRef* AToshiAnimationInterface::GetAnimationRef( T2DList<AToshiAnimationRef>& a_rList, const TPString8& a_strAnimationName )
{
	T2_FOREACH( a_rList, it )
	{
		if ( it->GetNamedAnimation()->GetName() == a_strAnimationName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00582730
static TBOOL GetAnimationTransition( T2DList<AToshiAnimationRef>& a_rList, const TPString8& a_strAnimationName, AToshiAnimationRef*& a_rpAnimRef, ANamedAnimationTransition*& a_rpTransition )
{
	T2_FOREACH( a_rList, it )
	{
		ANamedAnimationTransition* pTransition = it->GetNamedAnimation()->GetTransitionSet().FindTransition( a_strAnimationName );

		if ( pTransition )
		{
			a_rpAnimRef    = it;
			a_rpTransition = pTransition;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00583360
AToshiAnimationInterface::AToshiAnimationInterface()
    : m_pSkeletonInstance( TNULL )
    , m_fTime( 0.0f )
    , m_Unk2( 0 )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 00583390
AToshiAnimationInterface::~AToshiAnimationInterface()
{
	DestroyAnimReferences();
}

// $Barnyard: FUNCTION 00583440
void AToshiAnimationInterface::Update( TFLOAT a_fDeltaTime, AnimEventList& a_rEventList )
{
	TFLOAT fOldTime = m_fTime;
	m_fTime += a_fDeltaTime;

	UpdateAnimations( m_llAnimRefs, a_fDeltaTime, a_rEventList );
	UpdateAnimations( m_llOverlayAnimRefs, a_fDeltaTime, a_rEventList );

	if ( fOldTime == 0.0f || m_fTime == 0.0f )
	{
		// Some animation has just started playing, update flags

		if ( IsAllStatic() )
			m_eFlags |= FLAGS_ALL_STATIC;
		else
			m_eFlags &= ~FLAGS_ALL_STATIC;
	}
}

// $Barnyard: FUNCTION 00583780
AToshiAnimationRef* AToshiAnimationInterface::PlayAnim( const TPString8& a_strName, TFLOAT a_fWeight, TBOOL a_bStartNewAnimation )
{
	// Check if this animation is not already playing
	AToshiAnimationRef* pAnimRef = FindAnimationRef( a_strName );
	pAnimRef                     = FindAnimationRef( a_strName ); // ???

	if ( pAnimRef )
	{
		// Get internal (exported) name of the playing animation
		TPString8        strPlayingName  = pAnimRef->GetNamedAnimation()->GetExportedName();
		ANamedAnimation* pNamedAnimation = GetAnimationSet()->GetNamedAnimation( a_strName );
		TVALIDPTR( pNamedAnimation );

		TPString8 strAnimationName = pNamedAnimation->GetExportedName();

		if ( strAnimationName == strPlayingName )
		{
			// This animation is already playing
			return pAnimRef;
		}
	}

	// Get info about the animation and play it
	ANamedAnimation* pNamedAnimation = GetAnimationSet()->GetNamedAnimation( a_strName );
	TVALIDPTR( pNamedAnimation );

	// Adjust playing parameters
	TFLOAT fWeight = ( a_fWeight < 0.0f ) ? pNamedAnimation->GetDefaultWeight() : a_fWeight;

	// Create overlay animation
	if ( pNamedAnimation->IsOverlay() )
		return CreateOverlayAnimationRef( pNamedAnimation, fWeight );

	// Create new animation if there are no other animations
	if ( m_llAnimRefs.IsEmpty() || m_llAnimRefs.Begin() == TNULL )
		return CreateAnimationRef( m_llAnimRefs, pNamedAnimation, fWeight, TNULL );

	// Create new animation if client asked
	if ( a_bStartNewAnimation )
		return CreateAnimationRef( m_llAnimRefs, pNamedAnimation, fWeight, TNULL );

	// Client didn't require starting new animation and some animations are already playing...
	ANamedAnimationTransition* pTransition;
	TBOOL                      bHasTransition = GetAnimationTransition( m_llAnimRefs, a_strName, pAnimRef, pTransition );

	if ( bHasTransition )
	{
		// Transition of this animation already exists
		SetAnimationTransition( *pAnimRef, pTransition );
		pAnimRef->SetWeight( fWeight );

		StopAnim( *pAnimRef );
		return TNULL;
	}

	// There's currently no transition of this animation
	pAnimRef                           = m_llAnimRefs.Head();
	ANamedAnimation* pCurrentAnimation = pAnimRef->GetNamedAnimation();

	// Set transition settings
	m_oAnimTransition.SetManaged( !pCurrentAnimation->IsFinishManual() );
	m_oAnimTransition.SetBlendInTime( pNamedAnimation->GetDefaultBlendInTime() );
	m_oAnimTransition.SetBlendOutTime( pCurrentAnimation->GetDefaultBlendOutTime() );
	m_oAnimTransition.SetAnimationName( pNamedAnimation->GetName() );
	SetAnimationTransition( *pAnimRef, &m_oAnimTransition );

	pAnimRef->SetWeight( fWeight );
	StopAnim( *pAnimRef );

	return TNULL;
}

// $Barnyard: FUNCTION 00582d30
TAnimation* AToshiAnimationInterface::PlayAnimImpl( ANamedAnimation* a_pNamedAnimation, TFLOAT a_fWeight, ANamedAnimationTransition* a_pTransition, AToshiAnimationRef& a_rAnimationRef )
{
	TVALIDPTR( a_pNamedAnimation );

	m_fTime = 0.0f;

	TFLOAT fBlendInTime  = ( a_pTransition ) ? a_pTransition->GetBlendInTime() : a_pNamedAnimation->GetDefaultBlendInTime();
	TFLOAT fBlendOutTime = a_pNamedAnimation->GetDefaultBlendOutTime();

	ANamedAnimationTransition* pDestTransition = a_pNamedAnimation->GetTransitionSet().GetDestTransition();
	if ( pDestTransition ) fBlendOutTime = pDestTransition->GetBlendOutTime();

	TAnimation* pAnimation = m_pSkeletonInstance->AddAnimation( a_pNamedAnimation->GetSequenceId(), a_fWeight, fBlendInTime );
	TVALIDPTR( pAnimation );

	pAnimation->SetBlendOutSpeed( fBlendOutTime );
	pAnimation->SetSpeed( a_pNamedAnimation->GetSpeed() * ( ( a_pNamedAnimation->IsReversed() ) ? -1.0f : 1.0f ) );

	if ( !a_pNamedAnimation->IsLooped() )
		pAnimation->ChangeToManaged( pAnimation->GetBlendOutSpeed() );

	a_rAnimationRef.m_pNamedAnimation  = a_pNamedAnimation;
	a_rAnimationRef.m_pTransition      = TNULL;
	a_rAnimationRef.m_bWasEverUpdated  = TFALSE;
	a_rAnimationRef.m_iBreakpointIndex = 0;
	a_rAnimationRef.m_fWeight          = a_fWeight;

	return pAnimation;
}

// $Barnyard: FUNCTION 00583080
void AToshiAnimationInterface::UpdateAnimations( Toshi::T2DList<AToshiAnimationRef>& a_rList, TFLOAT a_fDeltaTime, AnimEventList& a_rEventList )
{
	auto it = a_rList.Begin();

	while ( TTRUE )
	{
		// Look for an animation that is about to end/is already over
		while ( TTRUE )
		{
			if ( it == a_rList.End() ) return;

			TAnimation* pAnimation = ( !it->GetNamedAnimation() ) ? TNULL : m_pSkeletonInstance->GetAnimation( it->GetNamedAnimation()->GetSequenceId() );
			UpdateAnimation( a_fDeltaTime, *it, pAnimation, a_rEventList );

			// Has this animation ended?
			if ( !pAnimation || pAnimation->GetState() == TAnimation::STATE_BLENDING_OUT )
				break;

			// It hasn't ended, go for a next one
			++it;
		}

		// Found an animation that is over
		m_fTime = 0.0f;

		AToshiAnimationRef* pAnimRef        = it;
		ANamedAnimation*    pNamedAnimation = pAnimRef->GetNamedAnimation();
		TVALIDPTR( pNamedAnimation );

		// Create the event object
		AnimEvent& animEvent = a_rEventList.Push();
		animEvent.SetSimpleEvent( ANIMEVENT_TYPE_END, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), 0.0f );

		// Start transition if any is set
		if ( pNamedAnimation )
		{
			ANamedAnimationTransition* pTransition = pAnimRef->GetTransition();

			// If no transition is set to this animref, use default transition specified for this animation
			if ( !pTransition )
				pTransition = pAnimRef->GetNamedAnimation()->GetTransitionSet().GetDestTransition();

			if ( pTransition )
			{
				ANamedAnimation* pTransitionAnimation = m_pAnimationSet->GetNamedAnimation( pTransition->GetAnimationName() );
				TVALIDPTR( pTransitionAnimation );

				if ( !( pTransitionAnimation->GetFlags() & ANamedAnimation::FLAGS_NO_ANIM_REF ) )
				{
					// Do the transition
					AToshiAnimationRef* pAnimRef   = g_oAnimationRefPool.NewObject();
					TAnimation*         pAnimation = PlayAnimImpl( pTransitionAnimation, pAnimRef->m_fWeight, pTransition, *pAnimRef );
					pAnimation->UpdateTime( a_fDeltaTime );
					a_rList.PushBack( pAnimRef );
				}
			}
		}

		// Destroy this animation and remove it from the list
		it = a_rList.Erase( it );
		g_oAnimationRefPool.DeleteObject( pAnimRef );
	}
}

// $Barnyard: FUNCTION 005827a0
void AToshiAnimationInterface::UpdateAnimation( TFLOAT a_fDeltaTime, AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList )
{
	ANamedAnimation* pNamedAnimation = a_rAnimRef.GetNamedAnimation();
	TVALIDPTR( pNamedAnimation );

	if ( !a_rAnimRef.m_bWasEverUpdated )
	{
		// Create initial ANIMEVENT_TYPE_STARTED event for this animation since it's the first time updating it

		AnimEvent& animEvent = a_rEventList.Push();
		animEvent.SetSimpleEvent( ANIMEVENT_TYPE_START, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), 0.0f );

		a_rAnimRef.m_bWasEverUpdated = TTRUE;
	}

	// Don't want to update animations with zero progression
	if ( a_pAnimation && a_pAnimation->GetSpeed() == 0.0f )
		return;

	// Update breakpoints information
	TBOOL bIsReverse = ( a_pAnimation != TNULL && a_pAnimation->GetSpeed() < 0.0f );

	if ( !bIsReverse )
	{
		// Normal playing
		UpdateAnimationBreakpoints( a_rAnimRef, a_pAnimation, a_rEventList );
	}
	else
	{
		// Reverse playing
		UpdateAnimationBreakpointsReverse( a_rAnimRef, a_pAnimation, a_rEventList );
	}
}

// NOTE: inlined into UpdateAnimation (005827a0)
void AToshiAnimationInterface::UpdateAnimationBreakpoints( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList )
{
	ANamedAnimation* pNamedAnimation = a_rAnimRef.GetNamedAnimation();
	TVALIDPTR( pNamedAnimation );

	TINT        iCurrentBreakpoint = a_rAnimRef.m_iBreakpointIndex;
	TINT        iNumBreakpoints    = pNamedAnimation->GetBreakpoints().Size();
	TAnimation* pAnimation         = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

	// Animations that aren't managed (removed when they end) are looped
	TBOOL bLooped = pAnimation && !pAnimation->IsManaged();

	// Reset breakpoint index if necessary
	if ( bLooped && iCurrentBreakpoint >= iNumBreakpoints )
	{
		// NOTE: the original only resets the stored index and keeps using the old one
		// in this update, which would read past the breakpoints
		iCurrentBreakpoint            = 0;
		a_rAnimRef.m_iBreakpointIndex = 0;
	}

	TFLOAT fCurrentAnimTime  = ( !a_pAnimation ) ? pNamedAnimation->GetDuration() : a_pAnimation->GetSeqTime();
	TFLOAT fPreviousAnimTime = a_rAnimRef.m_fTime;

	if ( iCurrentBreakpoint < iNumBreakpoints || bLooped )
	{
		TFLOAT fAnimDeltaTime = fCurrentAnimTime - fPreviousAnimTime;

		// Normalize time if the animation restarted
		if ( fAnimDeltaTime < 0.0f )
			fAnimDeltaTime += pNamedAnimation->GetDuration();

		for ( TINT i = 0; ( iCurrentBreakpoint < iNumBreakpoints || bLooped ) && i < iNumBreakpoints; i++ )
		{
			AAnimationBreakpoint* pBreakpoint       = pNamedAnimation->GetBreakpoints()[ iCurrentBreakpoint ];
			TFLOAT                fTimeToBreakpoint = fCurrentAnimTime - pBreakpoint->GetTime();

			if ( a_pAnimation && fTimeToBreakpoint < 0.0f )
			{
				// The breakpoint is ahead, unless the looped animation has restarted and it was skipped
				if ( !bLooped || fCurrentAnimTime >= fPreviousAnimTime || fPreviousAnimTime >= pBreakpoint->GetTime() )
					break;

				fTimeToBreakpoint += pNamedAnimation->GetDuration();
			}

			// The breakpoint hasn't been reached during this update
			if ( fAnimDeltaTime < fTimeToBreakpoint )
				break;

			// Create the event object
			AnimEvent& animEvent = a_rEventList.Push();
			animEvent.SetBreakpointEvent( pBreakpoint, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), TMath::Max( fTimeToBreakpoint, 0.0f ) );

			iCurrentBreakpoint++;

			if ( bLooped && iCurrentBreakpoint >= iNumBreakpoints )
			{
				iCurrentBreakpoint = 0;

				if ( fPreviousAnimTime < fCurrentAnimTime )
					break;
			}
		}

		a_rAnimRef.m_iBreakpointIndex = iCurrentBreakpoint;
	}

	a_rAnimRef.m_fTime = fCurrentAnimTime;
}

// NOTE: inlined into UpdateAnimation (005827a0)
void AToshiAnimationInterface::UpdateAnimationBreakpointsReverse( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList )
{
	ANamedAnimation* pNamedAnimation = a_rAnimRef.GetNamedAnimation();
	TVALIDPTR( pNamedAnimation );

	TINT        iCurrentBreakpoint = a_rAnimRef.m_iBreakpointIndex;
	TINT        iNumBreakpoints    = pNamedAnimation->GetBreakpoints().Size();
	TAnimation* pAnimation         = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

	// Animations that aren't managed (removed when they end) are looped
	TBOOL bLooped = pAnimation && !pAnimation->IsManaged();

	// Reset breakpoint index if necessary
	if ( bLooped && iCurrentBreakpoint < 0 )
	{
		// NOTE: the original only resets the stored index and keeps using the old one in this update
		iCurrentBreakpoint            = iNumBreakpoints - 1;
		a_rAnimRef.m_iBreakpointIndex = iCurrentBreakpoint;
	}

	TFLOAT fCurrentAnimTime  = ( !a_pAnimation ) ? 0.0f : a_pAnimation->GetSeqTime();
	TFLOAT fPreviousAnimTime = a_rAnimRef.m_fTime;

	if ( iCurrentBreakpoint > 0 || bLooped )
	{
		TFLOAT fAnimDeltaTime = fPreviousAnimTime - fCurrentAnimTime;

		// Normalize time if the animation restarted
		if ( fAnimDeltaTime < 0.0f )
			fAnimDeltaTime += pNamedAnimation->GetDuration();

		TINT iBreakpoint = iCurrentBreakpoint - 1;

		if ( bLooped && iBreakpoint < 0 )
			iBreakpoint = iNumBreakpoints - 1;

		for ( TINT i = 0; ( iBreakpoint >= 0 || bLooped ) && i < iNumBreakpoints; i++ )
		{
			AAnimationBreakpoint* pBreakpoint       = pNamedAnimation->GetBreakpoints()[ iBreakpoint ];
			TFLOAT                fTimeToBreakpoint = pBreakpoint->GetTime() - fCurrentAnimTime;

			if ( a_pAnimation && fTimeToBreakpoint < 0.0f )
			{
				// The breakpoint is behind, unless the looped animation has restarted and it was skipped
				if ( !bLooped || fPreviousAnimTime >= fCurrentAnimTime || pBreakpoint->GetTime() >= fPreviousAnimTime )
					break;

				fTimeToBreakpoint += pNamedAnimation->GetDuration();
			}

			// The breakpoint hasn't been reached during this update
			if ( fAnimDeltaTime < fTimeToBreakpoint )
				break;

			// Create the event object
			AnimEvent& animEvent = a_rEventList.Push();
			animEvent.SetBreakpointEvent( pBreakpoint, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), TMath::Max( fTimeToBreakpoint, 0.0f ) );

			// NOTE: the original continues with the breakpoint at the old index (not the previous one)
			iBreakpoint = iCurrentBreakpoint--;

			if ( bLooped )
			{
				if ( iCurrentBreakpoint < 0 )
					iCurrentBreakpoint = iNumBreakpoints - 1;

				if ( iBreakpoint == iNumBreakpoints - 1 && fPreviousAnimTime > fCurrentAnimTime )
					break;
			}

			// NOTE: not in the original, keeps the index in range
			if ( iBreakpoint >= iNumBreakpoints )
				break;
		}

		a_rAnimRef.m_iBreakpointIndex = iCurrentBreakpoint;
	}

	a_rAnimRef.m_fTime = fCurrentAnimTime;
}

// $Barnyard: FUNCTION 00582de0
TBOOL AToshiAnimationInterface::IsAllStatic()
{
	if ( m_Unk2 != 0 )
		return TFALSE;

	T2_FOREACH( m_llAnimRefs, it )
	{
		if ( it->GetNamedAnimation() && !it->GetNamedAnimation()->IsStatic() )
			return TFALSE;
	}

	T2_FOREACH( m_llOverlayAnimRefs, it )
	{
		if ( it->GetNamedAnimation() && !it->GetNamedAnimation()->IsStatic() )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00583520
TBOOL AToshiAnimationInterface::StopAnim( const AToshiAnimationRef& a_rcAnimationRef )
{
	ANamedAnimation* pNamedAnimation = a_rcAnimationRef.m_pNamedAnimation;
	if ( !pNamedAnimation ) return TFALSE;

	TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_rcAnimationRef.GetSequenceId() );
	if ( !pAnimation ) return TFALSE;

	ANamedAnimationTransition* pTransition = a_rcAnimationRef.m_pTransition;

	if ( !pNamedAnimation || ( !pTransition && ( pTransition = pNamedAnimation->GetTransitionSet().GetDestTransition(), pTransition == TNULL ) ) )
	{
		if ( !pNamedAnimation->IsFinishManual() )
		{
			pAnimation->ChangeToManaged( pAnimation->GetBlendOutSpeed() );
			return TTRUE;
		}
	}
	else
	{
		if ( pTransition->IsManaged() )
		{
			pAnimation->ChangeToManaged( pAnimation->GetBlendOutSpeed() );
			return TTRUE;
		}

		// Yeah, that's how this thing is in the original.
		if ( pTransition->IsManaged() != TFALSE )
		{
			return TTRUE;
		}
	}

	// Manually finished animations (or unmanaged transitions) are removed right away
	pAnimation->RemoveAnimation( pAnimation->GetBlendOutSpeed() );
	return TTRUE;
}

// $Barnyard: FUNCTION 00582520
TBOOL AToshiAnimationInterface::StopAnim( const AToshiAnimationRef& a_rcAnimationRef, ANamedAnimationTransition* a_pTransition )
{
	ANamedAnimation* pNamedAnimation = a_rcAnimationRef.m_pNamedAnimation;
	if ( !pNamedAnimation ) return TFALSE;

	if ( TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_rcAnimationRef.GetSequenceId() ) )
	{
		if ( a_pTransition->IsManaged() )
			pAnimation->ChangeToManaged( pAnimation->GetBlendOutSpeed() );
		else
			pAnimation->RemoveAnimation( pAnimation->GetBlendOutSpeed() );

		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005836e0
void AToshiAnimationInterface::KillAllAnimations()
{
	DestroyAnimReferences();

	TVALIDPTR( m_pSkeletonInstance );
	m_pSkeletonInstance->RemoveAllAnimations();
}

// $Barnyard: FUNCTION 005835c0
TBOOL AToshiAnimationInterface::SetAnimationTransition( AToshiAnimationRef& a_rAnimationRef, ANamedAnimationTransition* a_pTransition )
{
	a_rAnimationRef.m_pTransition    = a_pTransition;
	ANamedAnimation* pNamedAnimation = a_rAnimationRef.m_pNamedAnimation;

	if ( pNamedAnimation )
	{
		if ( TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_rAnimationRef.GetSequenceId() ) )
		{
			pAnimation->SetBlendOutSpeed( a_pTransition ? a_pTransition->GetBlendOutTime() : 0.0f );
			return TTRUE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005834d0
TPString8 AToshiAnimationInterface::GetTransitionName( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->m_pNamedAnimation )
	{
		ANamedAnimationTransition* pTransition = a_pAnimRef->m_pTransition;

		if ( !pTransition )
			pTransition = a_pAnimRef->m_pNamedAnimation->GetTransitionSet().GetDestTransition();

		if ( pTransition )
			return pTransition->GetAnimationName();
	}

	return TPString8();
}

// $Barnyard: FUNCTION 00583660
TBOOL AToshiAnimationInterface::StopAnim( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = GetAnimationRef( m_llAnimRefs, a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = GetAnimationRef( m_llOverlayAnimRefs, a_strName );

		if ( !pAnimRef )
			return TFALSE;
	}

	return StopAnim( *pAnimRef );
}

// $Barnyard: FUNCTION 00583740
TBOOL AToshiAnimationInterface::KillAnim( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = GetAnimationRef( m_llAnimRefs, a_strName );

	if ( !pAnimRef )
	{
		pAnimRef = GetAnimationRef( m_llOverlayAnimRefs, a_strName );

		if ( !pAnimRef )
			return TFALSE;
	}

	KillAnim( pAnimRef );
	return TTRUE;
}

// $Barnyard: FUNCTION 00583240
void AToshiAnimationInterface::KillAnim( AToshiAnimationRef* a_pAnimRef )
{
	// NOTE: retail returns an iterator to the next ref
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			pAnimation->RemoveAnimation( 0.0f );
	}

	for ( auto it = m_llAnimRefs.Begin(); it != m_llAnimRefs.End(); it++ )
	{
		if ( it == a_pAnimRef )
		{
			m_llAnimRefs.Erase( it );
			g_oAnimationRefPool.DeleteObject( a_pAnimRef );
			return;
		}
	}

	// NOTE: retail walks m_llAnimRefs again until the end of m_llOverlayAnimRefs (and erases from m_llAnimRefs),
	// which never ends if the ref is an overlay. KillAnim( const TPString8& ) is only used for main animations.
#ifdef BARNYARD_COMMUNITY_PATCH
	for ( auto it = m_llOverlayAnimRefs.Begin(); it != m_llOverlayAnimRefs.End(); it++ )
	{
		if ( it == a_pAnimRef )
		{
			m_llOverlayAnimRefs.Erase( it );
			g_oAnimationRefPool.DeleteObject( a_pAnimRef );
			return;
		}
	}
#else
	TASSERT( TFALSE && "KillAnim of an overlay animation never returns in retail" );
#endif
}

// $Barnyard: FUNCTION 005826e0
TBOOL AToshiAnimationInterface::SetAnimSpeed( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fSpeed )
{
	ANamedAnimation* pNamedAnimation = a_pAnimRef->GetNamedAnimation();

	if ( pNamedAnimation )
	{
		if ( pNamedAnimation->IsReversed() )
			a_fSpeed *= -1.0f;

		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

		if ( pAnimation )
		{
			pAnimation->SetSpeed( a_fSpeed );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00470970
ANamedAnimationSetRef AToshiAnimationInterface::GetAnimationSet()
{
	return m_pAnimationSet;
}

// $Barnyard: FUNCTION 004a6bf0
AToshiAnimationRef* AToshiAnimationInterface::FindAnimationRef( const TPString8& a_strName )
{
	AToshiAnimationRef* pAnimRef = TNULL;

	pAnimRef = GetAnimationRef( m_llAnimRefs, a_strName );
	if ( !pAnimRef ) pAnimRef = GetAnimationRef( m_llOverlayAnimRefs, a_strName );

	return pAnimRef;
}

// $Barnyard: FUNCTION 005831d0
AToshiAnimationRef* AToshiAnimationInterface::CreateAnimationRef( T2DList<AToshiAnimationRef>& a_rList, ANamedAnimation* a_pAnimation, TFLOAT a_fWeight, ANamedAnimationTransition* a_pTransition )
{
	if ( !GetAnimationRef( m_llAnimRefs, a_pAnimation->GetName() ) &&
	     !GetAnimationRef( m_llOverlayAnimRefs, a_pAnimation->GetName() ) &&
	     !( a_pAnimation->GetFlags() & ANamedAnimation::FLAGS_NO_ANIM_REF ) )
	{
		AToshiAnimationRef* pAnimRef = g_oAnimationRefPool.NewObject();
		PlayAnimImpl( a_pAnimation, a_fWeight, a_pTransition, *pAnimRef );
		a_rList.PushBack( pAnimRef );

		return pAnimRef;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00583610
AToshiAnimationRef* AToshiAnimationInterface::CreateOverlayAnimationRef( ANamedAnimation* a_pAnimation, TFLOAT a_fWeight )
{
	AToshiAnimationRef*        pAnimRef;
	ANamedAnimationTransition* pTransition = TNULL;
	GetAnimationTransition( m_llAnimRefs, a_pAnimation->GetName(), pAnimRef, pTransition );

	return CreateAnimationRef( m_llOverlayAnimRefs, a_pAnimation, a_fWeight, pTransition );
}

// $Barnyard: FUNCTION 00582220
TAnimation* AToshiAnimationInterface::GetAnimation( const AToshiAnimationRef& a_rAnimRef )
{
	return ( a_rAnimRef.GetNamedAnimation() ) ? m_pSkeletonInstance->GetAnimation( a_rAnimRef.GetSequenceId() ) : TNULL;
}

void AToshiAnimationInterface::DestroyAnimReferences()
{
	while ( !m_llAnimRefs.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_llAnimRefs.Begin();
		m_llAnimRefs.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}

	while ( !m_llOverlayAnimRefs.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_llOverlayAnimRefs.Begin();
		m_llOverlayAnimRefs.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}
}

AToshiAnimationRef::AToshiAnimationRef()
{
	m_pNamedAnimation  = TNULL;
	m_fWeight          = 1.0f;
	m_fTime            = 0.0f;
	m_pTransition      = TNULL;
	m_bWasEverUpdated  = TFALSE;
	m_iBreakpointIndex = 0;
}

AToshiAnimationRef::~AToshiAnimationRef()
{
}

// $Barnyard: FUNCTION 005802f0
void AToshiAnimationInterface::AnimEvent::SetSimpleEvent( ANIMEVENT_TYPE a_eType, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeToBreakpoint )
{
	eType               = a_eType;
	bIsOverlay          = a_bIsOverlay;
	strAnimationName    = a_strAnimationName;
	pNextBreakpoint     = TNULL;
	fTimeFromBreakpoint = a_fTimeToBreakpoint;
}

// $Barnyard: FUNCTION 00580280
void AToshiAnimationInterface::AnimEvent::SetBreakpointEvent( AAnimationBreakpoint* a_pBreakpoint, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeFromBreakpoint )
{
	eType               = ANIMEVENT_TYPE_BREAKPOINT;
	bIsOverlay          = a_bIsOverlay;
	strAnimationName    = a_strAnimationName;
	pNextBreakpoint     = a_pBreakpoint;
	fTimeFromBreakpoint = a_fTimeFromBreakpoint;
}

// T2ObjectPool<AToshiAnimationRef, 100>::NewObject
// $Barnyard: FUNCTION 00582660

// $Barnyard: FUNCTION 00582380
TBOOL AToshiAnimationInterface::IsAnimLooping( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return !pAnimation->IsManaged();
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005825b0
TBOOL AToshiAnimationInterface::IsAnimLoopEnding( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fDeltaTime )
{
	TFLOAT fDuration = -1.0f;

	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			fDuration = pAnimation->GetSequencePtr()->GetDuration();
	}

	TFLOAT fTime = 0.0f;

	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			fTime = pAnimation->GetSeqTime();
	}

	return TFLOAT( fmod( fTime / fDuration, 1.0 ) ) * fDuration + a_fDeltaTime >= fDuration;
}

// $Barnyard: FUNCTION 005823b0
TFLOAT AToshiAnimationInterface::GetAnimSpeed( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->GetSpeed();
	}

	return 1.0f;
}

// $Barnyard: FUNCTION 00582480
TFLOAT AToshiAnimationInterface::GetAnimProgress( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->GetSeqTime() / pAnimation->GetSequencePtr()->GetDuration();
	}

	return 1.0f;
}

// $Barnyard: FUNCTION 005824d0
TBOOL AToshiAnimationInterface::SetAnimProgress( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fProgress )
{
	if ( !a_pAnimRef->GetNamedAnimation() )
		return TFALSE;

	TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

	if ( !pAnimation )
		return TFALSE;

	pAnimation->SetTime( a_fProgress * pAnimation->GetSequencePtr()->GetDuration() );
	return TTRUE;
}

// $Barnyard: FUNCTION 00582240
TBOOL AToshiAnimationInterface::IsAnimFinishing( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fDeltaTime )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->IsFinishing( a_fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005823e0
TFLOAT AToshiAnimationInterface::GetAnimWeight( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->GetWeight();
	}

	return 0.0f;
}

// $Barnyard: FUNCTION 00582570
TBOOL AToshiAnimationInterface::SetAnimWeight( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fWeight )
{
	a_pAnimRef->SetWeight( a_fWeight );

	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			pAnimation->SetDestWeight( a_fWeight, 0.0f );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00582410
TFLOAT AToshiAnimationInterface::GetAnimTime( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->GetSeqTime();
	}

	return 0.0f;
}

// $Barnyard: FUNCTION 005822b0
TBOOL AToshiAnimationInterface::IsAnimPlayingBackwards( AToshiAnimationRef* a_pAnimRef )
{
	if ( !a_pAnimRef->GetNamedAnimation() )
		return TFALSE;

	TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

	return pAnimation && pAnimation->GetSpeed() < 0.0f;
}

// $Barnyard: FUNCTION 00582440
TBOOL AToshiAnimationInterface::SetAnimTime( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fTime )
{
	if ( !a_pAnimRef->GetNamedAnimation() )
		return TFALSE;

	TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

	if ( !pAnimation )
		return TFALSE;

	pAnimation->SetTime( a_fTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 00582f00
TBOOL AToshiAnimationInterface::IsAnimPlaying( const TPString8& a_strName, TBOOL a_bDetailedCheck )
{
	AToshiAnimationRef* pAnimRef = GetAnimationRef( m_llAnimRefs, a_strName );

	if ( !pAnimRef )
		pAnimRef = GetAnimationRef( m_llOverlayAnimRefs, a_strName );

	if ( !a_bDetailedCheck || !pAnimRef )
		return pAnimRef != TNULL;

	TPString8 strPlayingName = pAnimRef->GetNamedAnimation()->GetExportedName();
	TPString8 strSetName     = GetAnimationSet()->GetNamedAnimation( a_strName )->GetExportedName();

	return strPlayingName == strSetName;
}

// $Barnyard: FUNCTION 00582270
TFLOAT AToshiAnimationInterface::GetAnimDuration( AToshiAnimationRef* a_pAnimRef )
{
	if ( a_pAnimRef->GetNamedAnimation() )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( a_pAnimRef->GetSequenceId() );

		if ( pAnimation )
			return pAnimation->GetSequencePtr()->GetDuration();
	}

	return -1.0f;
}

// $Barnyard: FUNCTION 00582e80
void AToshiAnimationInterface::SetAllAnimSpeeds( TFLOAT a_fSpeed )
{
	T2_FOREACH( m_llAnimRefs, it )
	{
		ANamedAnimation* pNamedAnimation = it->GetNamedAnimation();

		if ( pNamedAnimation )
		{
			TFLOAT fSpeed = a_fSpeed;

			if ( pNamedAnimation->IsReversed() )
				fSpeed = a_fSpeed * -1.0f;

			TAnimation* pAnimation = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

			if ( pAnimation )
				pAnimation->SetSpeed( fSpeed );
		}
	}
}

// $Barnyard: FUNCTION 005836a0
void AToshiAnimationInterface::KillBaseAnimations()
{
	while ( !m_llAnimRefs.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_llAnimRefs.Begin();
		m_llAnimRefs.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}

	TQList<TAnimation>& rAnimations = m_pSkeletonInstance->GetBaseAnimations();

	while ( !rAnimations.IsEmpty() )
		m_pSkeletonInstance->RemoveAnimation( rAnimations.Begin(), 0.0f );
}
