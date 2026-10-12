#pragma once
#include "AAnimatedUnit.h"

// An animated unit driven by an ACharacterMovementComponent (walk/run desires).
// It fades out while the camera is inside it (see SetFadeOut) and back in once the
// camera has left.
//
// TClass 0x0077e6f8 (version 1.1, size 0xa0, runtime creatable), vtable 0x007181b0,
// secondary (AGoalCollisionUpdateInterface) vtable 0x007181a0.
class ACharacterUnit : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( ACharacterUnit, AAnimatedUnit );

	using FADEFLAGS = TUINT8;
	enum FADEFLAGS_ : FADEFLAGS
	{
		FADEFLAGS_NONE     = 0,
		FADEFLAGS_FADE_OUT = BITFLAG( 0 ),
		FADEFLAGS_FADE_IN  = BITFLAG( 1 ),
	};

public:
	ACharacterUnit();
	~ACharacterUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable() override;                     // +0x0c
	virtual TBOOL           Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void            OnDestroy() override;                            // +0x60
	virtual void            UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68, fades the model
	virtual TBOOL           Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	// Starts fading out (TTRUE) or back in (TFALSE, only while fading)
	void SetFadeOut( TBOOL a_bFadeOut );

	// FSM natives
	TBOOL           StartConversationWithTarget();
	ACharacterUnit* GetNearbyCharacter();
	ACharacterUnit* GetNearbyAvailableCharacter();
	ACharacterUnit* GetNearbyCharacterOfType( const Toshi::TPString8& a_rcType );
	TBOOL           IsCloseToUnit( AUnit* a_pUnit, TFLOAT a_fDistance );
	void            SetDesireWalk();
	void            SetDesireRun();
	void            SetDesiredSpeed( TFLOAT a_fSpeed );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	TFLOAT    m_fOpacity;   // 0x98
	FADEFLAGS m_eFadeFlags; // 0x9c
};

TSTATICASSERT( sizeof( ACharacterUnit ) == 0xa0 );
