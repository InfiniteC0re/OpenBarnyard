#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>
#include <Toshi/T2SortedVector.h>
#include <Toshi/T2Map.h>
#include <Render/TSkeleton.h>
#include <Plugins/PPropertyParser/PBProperties.h>

class ANamedAnimation;

//-----------------------------------------------------------------------------
// Purpose: Represents a single transition of a named animation
//-----------------------------------------------------------------------------
class ANamedAnimationTransition
{
public:
	friend class ANamedAnimation;
	friend class ANamedAnimationTransitionSet;

public:
	ANamedAnimationTransition();
	~ANamedAnimationTransition();

	void SetTransitionName( const Toshi::TPString8& a_strName );
	void SetAnimationName( const Toshi::TPString8& a_strName );

	void SetBlendInTime( TFLOAT a_fTime );
	void SetBlendOutTime( TFLOAT a_fTime );
	void SetAutoTransition( TBOOL a_bAutoTransition );
	void SetManaged( TBOOL a_bManaged );

	TFLOAT GetBlendInTime() const;
	TFLOAT GetBlendOutTime() const;
	TBOOL  IsAutoTransition() const;
	TBOOL  IsManaged() const;

private:
	Toshi::TPString8 m_strTransitionName;
	TFLOAT           m_fBlendInTime;
	TFLOAT           m_fBlendOutTime;
	Toshi::TPString8 m_strAnimationName;
	TBOOL            m_bAutoTransition;
	TBOOL            m_bManaged;
};

//-----------------------------------------------------------------------------
// Purpose: Holds map of transitions applied to a named animation
//-----------------------------------------------------------------------------
class ANamedAnimationTransitionSet
{
public:
	friend class ANamedAnimation;

public:
	ANamedAnimationTransitionSet( ANamedAnimation* a_pNamedAnimation, Toshi::T2Allocator* a_pAllocator );
	~ANamedAnimationTransitionSet();

	void                       AddTransition( ANamedAnimation* a_pAnimation, const Toshi::TPString8& a_strTransitionName, TFLOAT a_fBlendInTime, TFLOAT a_fBlendOutTime, TBOOL a_bIsAutoTransition, TBOOL a_bManaged );
	ANamedAnimationTransition* FindTransition( const Toshi::TPString8& a_strName );

private:
	ANamedAnimation* m_pAnimation;

	using TransitionMap = Toshi::T2Map<Toshi::TPString8, ANamedAnimationTransition, Toshi::TPString8::Comparator>;
	TransitionMap m_mapTransitions;

	Toshi::TPString8 m_strAnimationName;
	TBOOL            m_bUnk3;
	Toshi::TPString8 m_strUnk4;
};

class ANamedAnimation
{
public:
	using FLAGS = TUINT16;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE               = 0,
		FLAGS_FINISH_MANUAL      = BITFLAG( 0 ),
		FLAGS_FINISH_ON_ANIM_END = BITFLAG( 1 ),
		FLAGS_OVERLAY            = BITFLAG( 2 ),
		FLAGS_STATIC             = BITFLAG( 3 ),
		FLAGS_UNK2               = BITFLAG( 4 ),
		FLAGS_LOOP               = BITFLAG( 5 ),
		FLAGS_REVERSE            = BITFLAG( 6 ),
	};

	using FINISHTYPE = TINT;
	enum FINISHTYPE_ : FINISHTYPE
	{
		FINISHTYPE_MANUAL,
		FINISHTYPE_AUTO
	};

	struct AAnimationBreakpointSortResult
	{
		// $Barnyard: FUNCTION 0057fcd0
		// $Barnyard: FUNCTION 006b6930
		TINT operator()( AAnimationBreakpoint* const& a_rcVal1, AAnimationBreakpoint* const& a_rcVal2 ) const
		{
			TFLOAT fDiff = a_rcVal1->GetTime() - a_rcVal2->GetTime();

			if ( 0.0f < fDiff )
				return 1;

			if ( fDiff < 0.0f )
				return -1;

			return 0;
		}
	};

	using BreakpointList       = Toshi::T2DynamicVector<AAnimationBreakpoint*>;
	using SortedBreakpointList = Toshi::T2SortedVector<AAnimationBreakpoint*, BreakpointList, AAnimationBreakpointSortResult>;

public:
	// constructors/destructor
	ANamedAnimation( Toshi::T2Allocator* a_pAllocator );
	~ANamedAnimation();

	TBOOL Create( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton );
	void  Reset();

	void AttachBreakpoint( AAnimationBreakpoint* a_pBreakpoint );

	TBOOL IsOverlay() const { return m_eFlags & FLAGS_OVERLAY; }

	const Toshi::TPString8& GetName() const { return m_strName; }
	const Toshi::TPString8& GetExportedName() const { return m_strExportedName; }

	static FINISHTYPE GetFinishType( const Toshi::TPString8& a_rcFinishType );

private:
	Toshi::TPString8 m_strExportedName;
	Toshi::TPString8 m_strName;
	TINT16           m_iSequenceId;
	FLAGS            m_eFlags;
	// ...
	TFLOAT               m_fDefaultBlendOutTime;
	TFLOAT               m_fDefaultBlendInTime;
	TFLOAT               m_fDefaultWeight;
	TFLOAT               m_fSpeed;
	TFLOAT               m_fDuration;
	Toshi::T2Allocator*  m_pAllocator;
	SortedBreakpointList m_vecBreakpoints;
};
