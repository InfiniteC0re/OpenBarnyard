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

	// $Barnyard: FUNCTION 00582210
	TFLOAT GetWeight() const { return m_fWeight; }

	TUINT16                    GetSequenceId() const { return m_pNamedAnimation->GetSequenceId(); }
	ANamedAnimation*           GetNamedAnimation() const { return m_pNamedAnimation; }
	ANamedAnimationTransition* GetTransition() const { return m_pTransition; }
	TINT                       GetBreakpointIndex() const { return m_iBreakpointIndex; }

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

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_ALL_STATIC    = BITFLAG( 0 ), // every playing animation is static, the skeleton state isn't recalculated
		FLAGS_ALWAYS_UPDATE = BITFLAG( 1 ), // updates the skeleton state even if FLAGS_ALL_STATIC is set
	};

	using ANIMEVENT_TYPE = TINT32;
	enum ANIMEVENT_TYPE_ : ANIMEVENT_TYPE
	{
		ANIMEVENT_TYPE_START,
		ANIMEVENT_TYPE_END,
		ANIMEVENT_TYPE_BREAKPOINT,
		ANIMEVENT_TYPE_NONE
	};

	struct AnimEvent
	{
		ANIMEVENT_TYPE        eType;
		TBOOL                 bIsOverlay;
		Toshi::TPString8      strAnimationName;
		AAnimationBreakpoint* pNextBreakpoint;
		TFLOAT                fTimeFromBreakpoint;

		// $Barnyard: FUNCTION 005802d0
		AnimEvent()
		    : eType( ANIMEVENT_TYPE_NONE ), bIsOverlay( TFALSE ), pNextBreakpoint( TNULL ), fTimeFromBreakpoint( 0.0f )
		{}

		// $Barnyard: FUNCTION 0057c630
		~AnimEvent() {}

		void SetSimpleEvent( ANIMEVENT_TYPE a_eType, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeToBreakpoint );
		void SetBreakpointEvent( AAnimationBreakpoint* a_pBreakpoint, TBOOL a_bIsOverlay, const Toshi::TPString8& a_strAnimationName, TFLOAT a_fTimeToBreakpoint );
	};

	struct AnimEventList
	{
		TINT      iNumEvents = 0;
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

	// Stops the main or overlay animation called a_strName. Returns TFALSE if it's not playing
	TBOOL StopAnim( const Toshi::TPString8& a_strName );

	// Removes the main or overlay animation called a_strName right away (no blending) with its ref. Returns TFALSE if it's not playing
	TBOOL KillAnim( const Toshi::TPString8& a_strName );

	// Removes the animation of a_pAnimRef right away (no blending) and destroys the ref
	void KillAnim( AToshiAnimationRef* a_pAnimRef );

	// Sets playback speed of the animation (negated for reversed animations)
	TBOOL SetAnimSpeed( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fSpeed );

	// Destroys all related animations
	void KillAllAnimations();

	// Removes every base animation (and its ref)
	void KillBaseAnimations();

	// Sets the speed of every animation (negated for reversed animations)
	void SetAllAnimSpeeds( TFLOAT a_fSpeed );

	// Attaches transition to the animation
	TBOOL SetAnimationTransition( AToshiAnimationRef& a_rAnimationRef, ANamedAnimationTransition* a_pTransition );

	void SetSkeletonInstance( Toshi::TSkeletonInstance* a_pInstance ) { m_pSkeletonInstance = a_pInstance; }
	// $Barnyard: FUNCTION 0048cf40
	void SetAnimationSet( const ANamedAnimationSetRef& a_rcAnimationSet ) { m_pAnimationSet = a_rcAnimationSet; }

	// Flag 1: all playing animations are static
	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

	TFLOAT GetTime() const { return m_fTime; }

	// Creates new animation ref, adds it to the list, plays it and returns it
	AToshiAnimationRef* CreateAnimationRef( Toshi::T2DList<AToshiAnimationRef>& a_rList, ANamedAnimation* a_pAnimation, TFLOAT a_fWeight, ANamedAnimationTransition* a_pTransition );

	// Creates new overlay animation ref, adds it to the list, plays it and returns it
	AToshiAnimationRef* CreateOverlayAnimationRef( ANamedAnimation* a_pAnimation, TFLOAT a_fWeight );

	Toshi::TAnimation*    GetAnimation( const AToshiAnimationRef& a_rAnimRef );
	ANamedAnimationSetRef GetAnimationSet();

	AToshiAnimationRef* FindAnimationRef( const Toshi::TPString8& a_strName );

	// Returns the weight of the animation the ref plays, 0 if it doesn't play one
	TFLOAT GetAnimWeight( AToshiAnimationRef* a_pAnimRef );

	// Sets the weight of the ref and of the animation it plays (without blending)
	TBOOL SetAnimWeight( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fWeight );

	// Returns the sequence time of the animation the ref plays, 0 if it doesn't play one
	TFLOAT GetAnimTime( AToshiAnimationRef* a_pAnimRef );
	TBOOL  SetAnimTime( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fTime );

	// Returns TTRUE if the animation plays backwards (negative speed)
	TBOOL IsAnimPlayingBackwards( AToshiAnimationRef* a_pAnimRef );

	// Returns the speed of the animation the ref plays, 1 if it doesn't play one
	TFLOAT GetAnimSpeed( AToshiAnimationRef* a_pAnimRef );

	// Returns the sequence time of the animation the ref plays relative to the sequence duration (0..1), 1 if it doesn't play one
	TFLOAT GetAnimProgress( AToshiAnimationRef* a_pAnimRef );

	// Returns the duration of the sequence the ref plays, -1 if it doesn't play one
	TFLOAT GetAnimDuration( AToshiAnimationRef* a_pAnimRef );

	// Sets the time of the animation the ref plays to a_fProgress (0..1) of the sequence duration
	TBOOL SetAnimProgress( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fProgress );

	// Returns TTRUE if the animation the ref plays isn't managed (it isn't removed when it ends, it loops)
	TBOOL IsAnimLooping( AToshiAnimationRef* a_pAnimRef );

	// Returns TTRUE if the looping animation the ref plays wraps around within a_fDeltaTime (or if it doesn't play one)
	TBOOL IsAnimLoopEnding( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fDeltaTime );

	// Returns TTRUE if the animation the ref plays finishes blending out within a_fDeltaTime (or if it doesn't play one)
	TBOOL IsAnimFinishing( AToshiAnimationRef* a_pAnimRef, TFLOAT a_fDeltaTime );

	// Returns TTRUE if the animation is in either list. With a_bDetailedCheck, also checks that the
	// playing animation exports the same sequence as the one in the current animation set
	TBOOL IsAnimPlaying( const Toshi::TPString8& a_strName, TBOOL a_bDetailedCheck );

	// TTRUE if the overlay animation a_strName is playing
	// NOTE: the name is invented
	// $Barnyard: FUNCTION 0050e520
	TBOOL IsOverlayAnimPlaying( const Toshi::TPString8& a_strName ) { return GetAnimationRef( m_llOverlayAnimRefs, a_strName ) != TNULL; }

	// Finds the ref of the animation called a_strAnimationName in a_rList
	static AToshiAnimationRef* GetAnimationRef( Toshi::T2DList<AToshiAnimationRef>& a_rList, const Toshi::TPString8& a_strAnimationName );

	Toshi::T2DList<AToshiAnimationRef>& GetAnimRefs() { return m_llAnimRefs; }
	Toshi::T2DList<AToshiAnimationRef>& GetOverlayAnimRefs() { return m_llOverlayAnimRefs; }

	// Returns the first (main) animation ref or TNULL if there are none
	// $Barnyard: FUNCTION 00582370
	AToshiAnimationRef* GetAnim() { return m_llAnimRefs.IsEmpty() ? TNULL : m_llAnimRefs.Head().Get(); }

	// Returns name of the animation the ref is going to transit to (empty if none)
	Toshi::TPString8 GetTransitionName( AToshiAnimationRef* a_pAnimRef );

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
	FLAGS                              m_eFlags;
};
