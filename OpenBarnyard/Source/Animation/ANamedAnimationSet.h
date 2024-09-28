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

	using OVERLAYTYPE = TUINT8;
	enum OVERLAYTYPE_ : OVERLAYTYPE
	{
		OVERLAYTYPE_NONE       = 0,
		OVERLAYTYPE_BLINK      = BITFLAG( 0 ),
		OVERLAYTYPE_LOOK_LEFT  = BITFLAG( 1 ),
		OVERLAYTYPE_LOOK_RIGHT = BITFLAG( 2 ),
		OVERLAYTYPE_LOOK_UP    = BITFLAG( 3 ),
		OVERLAYTYPE_LOOK_DOWN  = BITFLAG( 4 ),
	};

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
	TBOOL            LoadAnimationTransition( const PBProperties* a_pProperties );

private:
	Toshi::TPString8                                                               m_strName;
	Toshi::T2Map<Toshi::TPString8, ANamedAnimation*, Toshi::TPString8::Comparator> m_mapAnimations;
	Toshi::T2Allocator*                                                            m_pAllocator;
	OVERLAYTYPE                                                                    m_eOverlayType;
};

using ANamedAnimationSetRef = Toshi::TRef<ANamedAnimationSet>;
