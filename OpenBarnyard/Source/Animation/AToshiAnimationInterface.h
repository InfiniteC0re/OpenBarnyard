#pragma once
#include "ANamedAnimationSet.h"

#include <Toshi/T2DList.h>
#include <Render/TSkeleton.h>

class AToshiAnimationRef
    : public Toshi::T2DList<AToshiAnimationRef>::Node
{
public:
	friend class AToshiAnimationInterface;

public:
	AToshiAnimationRef();
	~AToshiAnimationRef();

	void SetWeight( TFLOAT a_fWeight ) { m_fWeight = a_fWeight; }

	TUINT16          GetSequenceId() const { return m_pNamedAnimation->GetSequenceId(); }
	ANamedAnimation* GetNamedAnimation() const { return m_pNamedAnimation; }

	private:
	ANamedAnimation*           m_pNamedAnimation;
	TFLOAT                     m_fWeight;
	TINT                       m_Unk1;
	ANamedAnimationTransition* m_pTransition;
	TBOOL                      m_Unk2;
	TINT                       m_Unk3;
};

class AToshiAnimationInterface
{
public:
	AToshiAnimationInterface();
	~AToshiAnimationInterface();

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

	void DestroyAnimReferences();

private:
	Toshi::T2DList<AToshiAnimationRef> m_llAnimRefs;
	Toshi::T2DList<AToshiAnimationRef> m_llOverlayAnimRefs;
	ANamedAnimationSetRef              m_pAnimationSet;
	Toshi::TSkeletonInstance*          m_pSkeletonInstance;
	TINT                               m_Unk1;
	TINT                               m_Unk2;
	ANamedAnimationTransition          m_oAnimTransition;
	TBOOL                              m_Unk3;
};
