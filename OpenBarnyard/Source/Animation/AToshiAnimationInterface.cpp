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
static AToshiAnimationRef* GetAnimationRef( T2DList<AToshiAnimationRef>& a_rList, const TPString8& a_strAnimationName )
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
    , m_eFlags( 0 )
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
			m_eFlags |= 1;
		else
			m_eFlags &= ~1;
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
	TBOOL bHasTransition = GetAnimationTransition( m_llAnimRefs, a_strName, pAnimRef, pTransition );

	if ( bHasTransition )
	{
		// Transition of this animation already exists
		SetAnimationTransition( *pAnimRef, pTransition );
		pAnimRef->SetWeight( fWeight );

		StopAnim( *pAnimRef );
		return TNULL;
	}

	// There's currently no transition of this animation
	pAnimRef = m_llAnimRefs.Head();
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

	TFLOAT fBlendInTime = ( a_pTransition ) ? a_pTransition->GetBlendInTime() : a_pNamedAnimation->GetDefaultBlendInTime();
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

void AToshiAnimationInterface::UpdateAnimationBreakpoints( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList )
{
	ANamedAnimation* pNamedAnimation = a_rAnimRef.GetNamedAnimation();
	TVALIDPTR( pNamedAnimation );

	TINT        iCurrentBreakpoint = a_rAnimRef.m_iBreakpointIndex;
	TINT        iNumBreakpoints    = pNamedAnimation->GetBreakpoints().Size();
	TAnimation* pAnimation         = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

	// Reset breakpoint index if necessary
	if ( pAnimation && pAnimation->IsManaged() && a_rAnimRef.m_iBreakpointIndex >= iNumBreakpoints )
		a_rAnimRef.m_iBreakpointIndex = 0;

	TFLOAT fCurrentAnimTime  = ( !a_pAnimation ) ? pNamedAnimation->GetDuration() : a_pAnimation->GetSeqTime();
	TFLOAT fPreviousAnimTime = a_rAnimRef.m_fTime;

	if ( iCurrentBreakpoint < iNumBreakpoints || ( pAnimation && pAnimation->IsManaged() ) )
	{
		TFLOAT fAnimDeltaTime = fCurrentAnimTime - fPreviousAnimTime;

		// Normalize time if the animation restarted
		if ( fAnimDeltaTime < 0.0f )
			fAnimDeltaTime += pNamedAnimation->GetDuration();

		for ( TINT i = 0; iCurrentBreakpoint < iNumBreakpoints && i < iNumBreakpoints && ( !pAnimation || pAnimation->IsManaged() ); i++ )
		{
			AAnimationBreakpoint* pBreakpoint       = pNamedAnimation->GetBreakpoints()[ iCurrentBreakpoint ];
			TFLOAT                fTimeToBreakpoint = fCurrentAnimTime - pBreakpoint->GetTime();

			if ( a_pAnimation && fTimeToBreakpoint < 0.0f )
			{
				// If animation didn't start from the beginning, or we already fetched this breakpoint, skip...
				if ( !pAnimation || !pAnimation->IsManaged() || fCurrentAnimTime >= fPreviousAnimTime || fPreviousAnimTime >= pBreakpoint->GetTime() )
					break;

				// The animation did start from the beginning, so need to normalize the time value
				fTimeToBreakpoint += pNamedAnimation->GetDuration();
			}

			// If we already activated that breakpoint before, skip...
			if ( fAnimDeltaTime < fTimeToBreakpoint )
				break;

			// Create the event object
			AnimEvent& animEvent = a_rEventList.Push();
			animEvent.SetBreakpointEvent( pBreakpoint, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), TMath::Max( fTimeToBreakpoint, 0.0f ) );

			iCurrentBreakpoint++;
			if ( iCurrentBreakpoint >= iNumBreakpoints )
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

void AToshiAnimationInterface::UpdateAnimationBreakpointsReverse( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList )
{
	ANamedAnimation* pNamedAnimation = a_rAnimRef.GetNamedAnimation();
	TVALIDPTR( pNamedAnimation );

	TINT        iCurrentBreakpoint = a_rAnimRef.m_iBreakpointIndex;
	TINT        iNumBreakpoints    = pNamedAnimation->GetBreakpoints().Size();
	TAnimation* pAnimation         = m_pSkeletonInstance->GetAnimation( pNamedAnimation->GetSequenceId() );

	// Reset breakpoint index if necessary
	if ( pAnimation && pAnimation->IsManaged() && a_rAnimRef.m_iBreakpointIndex < 0 )
		a_rAnimRef.m_iBreakpointIndex = iNumBreakpoints - 1;

	TFLOAT fCurrentAnimTime  = ( !a_pAnimation ) ? 0.0f : a_pAnimation->GetSeqTime();
	TFLOAT fPreviousAnimTime = a_rAnimRef.m_fTime;

	if ( iCurrentBreakpoint > 0 || ( pAnimation && pAnimation->IsManaged() ) )
	{
		TFLOAT fAnimDeltaTime  = fPreviousAnimTime - fCurrentAnimTime;
		TINT   iPrevBreakpoint = iCurrentBreakpoint - 1;

		// Normalize time if the animation restarted
		if ( fAnimDeltaTime < 0.0f )
			fAnimDeltaTime += pNamedAnimation->GetDuration();

		TINT iBreakpoint = iPrevBreakpoint;
		if ( pAnimation && pAnimation->IsManaged() && iPrevBreakpoint <= -1 )
		{
			iPrevBreakpoint = iNumBreakpoints - 1;
			iBreakpoint     = iPrevBreakpoint;
		}

		for ( TINT i = 0; i < iNumBreakpoints && iPrevBreakpoint > -1 || ( pAnimation && pAnimation->IsManaged() ); i++ )
		{
			iPrevBreakpoint = iCurrentBreakpoint;

			AAnimationBreakpoint* pBreakpoint       = pNamedAnimation->GetBreakpoints()[ iBreakpoint ];
			TFLOAT                fTimeToBreakpoint = pBreakpoint->GetTime() - fCurrentAnimTime;

			if ( pAnimation && fTimeToBreakpoint < 0.0f )
			{
				// If animation didn't start from the beginning, or we already fetched this breakpoint, skip...
				if ( !pAnimation || !pAnimation->IsManaged() || fCurrentAnimTime <= fPreviousAnimTime || fPreviousAnimTime <= pBreakpoint->GetTime() )
					break;

				// The animation did start from the beginning, so need to normalize the time value
				fTimeToBreakpoint += pNamedAnimation->GetDuration();
			}

			// If we already activated that breakpoint before, skip...
			if ( fAnimDeltaTime < fTimeToBreakpoint )
				break;

			// Create the event object
			AnimEvent& animEvent = a_rEventList.Push();
			animEvent.SetBreakpointEvent( pBreakpoint, pNamedAnimation->IsOverlay(), pNamedAnimation->GetName(), TMath::Max( fTimeToBreakpoint, 0.0f ) );

			iCurrentBreakpoint--;

			if ( pAnimation && pAnimation->IsManaged() )
			{
				if ( iCurrentBreakpoint < 0 )
					iCurrentBreakpoint = iNumBreakpoints - 1;

				if ( iPrevBreakpoint == iNumBreakpoints - 1 && fCurrentAnimTime < fPreviousAnimTime )
					break;
			}
		}

		a_rAnimRef.m_iBreakpointIndex = iCurrentBreakpoint;
	}

	a_rAnimRef.m_fTime = fCurrentAnimTime;
}

// $Barnyard: FUNCTION 00582de0
TBOOL AToshiAnimationInterface::IsAllStatic()
{
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

	// It will never get here
	TASSERT( TFALSE );
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
	a_rAnimationRef.m_pTransition = a_pTransition;
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

// $Barnyard: FUNCTION 00470970
ANamedAnimationSetRef AToshiAnimationInterface::GetAnimationSet()
{
	return m_pAnimationSet;
}

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
	m_pNamedAnimation = TNULL;
	m_fWeight         = 1.0f;
	m_fTime       = 0.0f;
	m_pTransition     = TNULL;
	m_bWasEverUpdated = TFALSE;
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
	a_fTimeToBreakpoint = fTimeFromBreakpoint;
}

// $Barnyard: FUNCTION 00580280
void AToshiAnimationInterface::AnimEvent::SetBreakpointEvent( AAnimationBreakpoint* a_pBreakpoint, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeFromBreakpoint )
{
	eType                 = ANIMEVENT_TYPE_BREAKPOINT;
	bIsOverlay            = a_bIsOverlay;
	strAnimationName      = a_strAnimationName;
	pNextBreakpoint       = a_pBreakpoint;
	a_fTimeFromBreakpoint = fTimeFromBreakpoint;
}

// T2ObjectPool<AToshiAnimationRef, 100>::NewObject
// $Barnyard: FUNCTION 00582660
