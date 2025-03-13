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
    , m_Unk1( 0 )
    , m_Unk2( 0 )
    , m_Unk3( TFALSE )
{
}

// $Barnyard: FUNCTION 00583390
AToshiAnimationInterface::~AToshiAnimationInterface()
{
	DestroyAnimReferences();
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

	m_Unk1 = 0;

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

	a_rAnimationRef.m_pNamedAnimation = a_pNamedAnimation;
	a_rAnimationRef.m_pTransition     = TNULL;
	a_rAnimationRef.m_Unk2            = TFALSE;
	a_rAnimationRef.m_Unk3            = 0;
	a_rAnimationRef.m_fWeight         = a_fWeight;

	return pAnimation;
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
	m_Unk1            = 0;
	m_pTransition     = TNULL;
	m_Unk2            = TFALSE;
	m_Unk3            = 0;
}

AToshiAnimationRef::~AToshiAnimationRef()
{
}

// T2ObjectPool<AToshiAnimationRef, 100>::NewObject
// $Barnyard: FUNCTION 00582660
