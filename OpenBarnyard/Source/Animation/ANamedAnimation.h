#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>
#include <Toshi/T2SortedVector.h>
#include <Render/TSkeleton.h>
#include <Plugins/PPropertyParser/PBProperties.h>

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
