#pragma once
#include "ANamedAnimation.h"
#include "AToshiAnimationRef.h"

#include <Toshi/TRefCounted.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>
#include <Render/TSkeleton.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class ANamedAnimationSet :
    public Toshi::TObject,
    public Toshi::TRefCounted
{
public:
	TDECLARE_CLASS( ANamedAnimationSet, Toshi::TObject );

public:
	// constructors/destructor
	ANamedAnimationSet();
	ANamedAnimationSet( Toshi::T2Allocator* a_pAllocator );
	~ANamedAnimationSet();

	TBOOL CreateFromProperties( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton );

	// required by Toshi::TRef
	void Delete();

	const Toshi::TPString8& GetName() const { return m_strName; }

private:
	ANamedAnimation* LoadNamedAnimation( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton );

private:
	Toshi::TPString8                                                               m_strName;
	Toshi::T2Map<Toshi::TPString8, ANamedAnimation*, Toshi::TPString8::Comparator> m_mapAnimations;
	Toshi::T2Allocator*                                                            m_pAllocator;
	TBOOL                                                                          m_bFlag;
};

using ANamedAnimationSetRef = Toshi::TRef<ANamedAnimationSet>;
