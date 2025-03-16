#pragma once
#include "ANamedAnimationSet.h"

#include <Toshi/T2DList.h>
#include <Render/TSkeleton.h>

class AAnimationBreakpoint;

class AToshiAnimationRef
    : public Toshi::T2DList<AToshiAnimationRef>::Node
{
public:
	friend class AToshiAnimationInterface;

public:
	AToshiAnimationRef();
	~AToshiAnimationRef();

	void SetWeight( TFLOAT a_fWeight ) { m_fWeight = a_fWeight; }

	TUINT16                    GetSequenceId() const { return m_pNamedAnimation->GetSequenceId(); }
	ANamedAnimation*           GetNamedAnimation() const { return m_pNamedAnimation; }
	ANamedAnimationTransition* GetTransition() const { return m_pTransition; }

	private:
	ANamedAnimation*           m_pNamedAnimation;
	TFLOAT                     m_fWeight;
	TFLOAT                     m_fTime;
	ANamedAnimationTransition* m_pTransition;
	TBOOL                      m_bWasEverUpdated;
	TINT                       m_iBreakpointIndex;
};

class AToshiAnimationInterface
{
public:
	static constexpr TSIZE MAX_NUM_ANIMATIONS = 20;

	using ANIMEVENT_TYPE = TINT32;
	enum ANIMEVENT_TYPE_ : ANIMEVENT_TYPE
	{
		ANIMEVENT_TYPE_START,
		ANIMEVENT_TYPE_END,
		ANIMEVENT_TYPE_BREAKPOINT
	};

	struct AnimEvent
	{
		ANIMEVENT_TYPE        eType;
		TBOOL                 bIsOverlay;
		Toshi::TPString8      strAnimationName;
		AAnimationBreakpoint* pNextBreakpoint;
		TFLOAT                fTimeFromBreakpoint;

		void SetSimpleEvent( ANIMEVENT_TYPE a_eType, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeToBreakpoint );
		void SetBreakpointEvent( AAnimationBreakpoint* a_pBreakpoint, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeToBreakpoint );
	};

	struct AnimEventList
	{
		TINT     iNumEvents = 0;
		AnimEvent aEvents[ MAX_NUM_ANIMATIONS ];

		AnimEvent& Head() { return aEvents[ 0 ]; }
		AnimEvent& Tail() { return aEvents[ iNumEvents ]; }
		TBOOL      IsFull() const { return iNumEvents == MAX_NUM_ANIMATIONS; }

		AnimEvent& Push()
		{
			TASSERT( IsFull() == TFALSE );
			return aEvents[ iNumEvents++ ];
		}
	};

public:
	AToshiAnimationInterface();
	~AToshiAnimationInterface();

	void Update( TFLOAT a_fDeltaTime, AnimEventList& a_rEventList );

	// Plays animation by it's name. Returns pointer to the animation ref object if created new one
	AToshiAnimationRef* PlayAnim( const Toshi::TPString8& a_strName, TFLOAT a_fWeight, TBOOL a_bCreateNewRef );

	// Stops an animation with blending settings applied. Returns TTRUE if succeeded
	TBOOL StopAnim( const AToshiAnimationRef& a_rAnimationRef );

	// Stops an animation with blending settings applied, allows custom transition settings. Returns TTRUE if succeeded
	TBOOL StopAnim( const AToshiAnimationRef& a_rAnimationRef, ANamedAnimationTransition* a_pTransition );

	// Destroys all related animations
	void KillAllAnimations();

	// Attaches transition to the animation
	TBOOL SetAnimationTransition( AToshiAnimationRef& a_rAnimationRef, ANamedAnimationTransition* a_pTransition );

	void SetSkeletonInstance( Toshi::TSkeletonInstance* a_pInstance ) { m_pSkeletonInstance = a_pInstance; }
	void SetAnimationSet( const ANamedAnimationSetRef& a_rcAnimationSet ) { m_pAnimationSet = a_rcAnimationSet; }

	// Creates new animation ref, adds it to the list, plays it and returns it
	AToshiAnimationRef* CreateAnimationRef( Toshi::T2DList<AToshiAnimationRef>& a_rList, ANamedAnimation* a_pAnimation, TFLOAT a_fWeight, ANamedAnimationTransition* a_pTransition );

	// Creates new overlay animation ref, adds it to the list, plays it and returns it
	AToshiAnimationRef* CreateOverlayAnimationRef( ANamedAnimation* a_pAnimation, TFLOAT a_fWeight );

	Toshi::TAnimation*    GetAnimation( const AToshiAnimationRef& a_rAnimRef );
	ANamedAnimationSetRef GetAnimationSet();

	AToshiAnimationRef* FindAnimationRef( const Toshi::TPString8& a_strName );

private:
	// Plays animation in the specified animation ref. Returns pointer to the created TAnimation object.
	Toshi::TAnimation* PlayAnimImpl( ANamedAnimation* a_pNamedAnimation, TFLOAT a_fWeight, ANamedAnimationTransition* a_pTransition, AToshiAnimationRef& a_rAnimationRef );

	// Updates all animations and adds info about anims that just started playing into a_rEventList
	void UpdateAnimations( Toshi::T2DList<AToshiAnimationRef>& a_rList, TFLOAT a_fDeltaTime, AnimEventList& a_rEventList );

	void UpdateAnimation( TFLOAT a_fDeltaTime, AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList );
	void UpdateAnimationBreakpoints( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList );
	void UpdateAnimationBreakpointsReverse( AToshiAnimationRef& a_rAnimRef, Toshi::TAnimation* a_pAnimation, AnimEventList& a_rEventList );

	TBOOL IsAllStatic();

	void DestroyAnimReferences();

private:
	Toshi::T2DList<AToshiAnimationRef> m_llAnimRefs;
	Toshi::T2DList<AToshiAnimationRef> m_llOverlayAnimRefs;
	ANamedAnimationSetRef              m_pAnimationSet;
	Toshi::TSkeletonInstance*          m_pSkeletonInstance;
	TFLOAT                             m_fTime;
	TINT                               m_Unk2;
	ANamedAnimationTransition          m_oAnimTransition;
	TUINT8                             m_eFlags;
};
