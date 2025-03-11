#pragma once
#include "ANamedAnimationSet.h"

#include <Toshi/T2DList.h>
#include <Render/TSkeleton.h>

class AToshiAnimationRef
    : public Toshi::T2DList<AToshiAnimationRef>::Node
{
public:
private:
};

class AToshiAnimationInterface
{
public:
	AToshiAnimationInterface();
	~AToshiAnimationInterface();

	void KillAllAnimations();

	void SetSkeletonInstance( Toshi::TSkeletonInstance* a_pInstance ) { m_pSkeletonInstance = a_pInstance; }
	void SetAnimationSet( const ANamedAnimationSetRef& a_rcAnimationSet ) { m_pAnimationSet = a_rcAnimationSet; }

	ANamedAnimationSetRef GetAnimationSet();

private:
	void DestroyAnimReferences();

private:
	Toshi::T2DList<AToshiAnimationRef> m_AnimRefList1;
	Toshi::T2DList<AToshiAnimationRef> m_AnimRefList2;
	ANamedAnimationSetRef              m_pAnimationSet;
	Toshi::TSkeletonInstance*          m_pSkeletonInstance;
	// ...
};
