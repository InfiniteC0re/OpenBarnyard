#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Plays "Animation" on the object, on its parent or on one of its attached
// objects (found by "ModelName" or, if it's empty, by "BoneName").
//-----------------------------------------------------------------------------
class AAnimationBreakpointAnimation
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointAnimation, AAnimationBreakpoint );

	// "Mode", order of the name checks in Create
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_PLAY_ANIMATION,        // "PlayAnimation", the object itself
		MODE_PLAY_CHILD_ANIMATION,  // "PlayChildAnimation"
		MODE_PLAY_PARENT_ANIMATION, // "PlayParentAnimation"
	};

public:
	AAnimationBreakpointAnimation();
	~AAnimationBreakpointAnimation();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;             // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE; // +0x14

private:
	// Returns the attached object of a_pObject called m_strModelName, or attached to m_strBoneName
	AAnimatableObject* FindChild( AAnimatableObject* a_pObject );

private:
	MODE             m_eMode;        // 0x08
	Toshi::TPString8 m_strModelName; // 0x0c, "ModelName", only with MODE_PLAY_CHILD_ANIMATION
	Toshi::TPString8 m_strBoneName;  // 0x10, "BoneName", only with MODE_PLAY_CHILD_ANIMATION
	Toshi::TPString8 m_strAnimation; // 0x14, "Animation"
};

TSTATICASSERT( sizeof( AAnimationBreakpointAnimation ) == 0x18 );
