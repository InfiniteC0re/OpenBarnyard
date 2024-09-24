#pragma once
#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>
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

public:
	// constructors/destructor
	ANamedAnimation( Toshi::T2Allocator* a_pAllocator );
	~ANamedAnimation();

	TBOOL Create( const PBProperties* a_pProperties, Toshi::TSkeleton* a_pSkeleton );
	void  Reset();

	const Toshi::TPString8& GetName() const { return m_strName; }
	const Toshi::TPString8& GetExportedName() const { return m_strExportedName; }

private:
	Toshi::TPString8 m_strExportedName;
	Toshi::TPString8 m_strName;
	TINT16           m_iSequenceId;
	FLAGS            m_eFlags;
	// ...
	TFLOAT                        m_fDefaultBlendOutTime;
	TFLOAT                        m_fDefaultBlendInTime;
	TFLOAT                        m_fDefaultWeight;
	TFLOAT                        m_fSpeed;
	TFLOAT                        m_fDuration;
	Toshi::T2Allocator*           m_pAllocator;
	Toshi::T2DynamicVector<void*> m_vecUnk;
};
