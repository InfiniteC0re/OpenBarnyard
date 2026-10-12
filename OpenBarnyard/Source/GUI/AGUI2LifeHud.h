#pragma once
#include "GUI/T2GUIString.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextureSection.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Lives counter: a wooden backing with a spinning sprite and a number
// (Mud Jumpers shows one per player). Plain T2GUIRectangle (no TClass).
// vtable 0073a000: +0x00 dtor 0063dd50, +0x04 Tick 0063e160, +0x0c Render 0063d5c0.
//-----------------------------------------------------------------------------
class AGUI2LifeHud
    : public Toshi::T2GUIRectangle
{
public:
	static constexpr TINT   MAX_LIVES           = 20;
	static constexpr TINT   NUM_GLOWS           = 3;
	static constexpr TFLOAT POP_DURATION        = 1.0f;  // DAT_00779864
	static constexpr TFLOAT SPIN_PER_LIFE       = 3.0f;  // a full turn of the sprite frames is 1.0
	static constexpr TFLOAT MAX_SPIN            = 12.0f; // m_fSpinTarget is kept within +-MAX_SPIN
	static constexpr TFLOAT MAX_WOBBLE          = 2.0f;
	static constexpr TINT   NUM_DEFAULT_SPRITES = 8;

public:
	AGUI2LifeHud();
	~AGUI2LifeHud();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;

	// Creates the element and attaches it to a_pParent. a_ppSprites/a_iNumSprites
	// select the spinning sprite frames (TNULL uses the default 8 "Loading_N" frames).
	// a_bNumberBehind adds the number before the sprite so it's drawn behind it.
	void Create( Toshi::T2GUIElement* a_pParent, const TCHAR** a_ppSprites, TINT a_iNumSprites, const TCHAR* a_szBacking, Toshi::T2GUIATTACHMENT a_eAnchor, Toshi::T2GUIATTACHMENT a_ePivot, TFLOAT a_fScale, TBOOL a_bNumberBehind );

	// Detaches the element and frees the sprite frames.
	void Destroy();

	// Shows/hides the element and resets the sprite to its first frame.
	void SetVisible( TBOOL a_bVisible );

	// Sets the counter without animating.
	void SetLives( TINT a_iLives );

	// Adds (positive) or removes (negative) lives one by one, animating each change.
	void AddLives( TINT a_iCount );

	// Adds one life (up to MAX_LIVES), spinning the sprite and playing the life sounds.
	void AddLife();

	// Adds one life collected at a_rcScreenPos.
	void AddLife( const Toshi::TVector4& a_rcScreenPos );

	// Spins the sprite by a_fAmount turns.
	void AddSpin( TFLOAT a_fAmount );

	// Makes the sprite wobble with the given strength (up to MAX_WOBBLE).
	void SetWobble( TFLOAT a_fStrength );

	// Shows the three glows behind the sprite with the a_szTexture texture for a_fDuration seconds.
	// TNULL hides them.
	void StartGlow( const TCHAR* a_szTexture, TFLOAT a_fDuration );

	// Plays a_ppLoopFrames looped (a_ppIntroFrames first, once) instead of the spin animation.
	void  SetFrameAnimations( Toshi::T2GUITextureSection** a_ppLoopFrames, Toshi::T2GUITextureSection** a_ppIntroFrames, TINT a_iNumLoopFrames, TINT a_iNumIntroFrames, TFLOAT a_fLoopSpeed, TFLOAT a_fIntroSpeed );
	void  SetFrameAnimation( Toshi::T2GUITextureSection** a_ppFrames, TINT a_iNumFrames, TFLOAT a_fSpeed );
	TBOOL IsPlayingFrameAnimation( Toshi::T2GUITextureSection** a_ppFrames ) const;

	TINT GetLives() const { return m_iLives; }

private:
	void UpdateNumber( TFLOAT a_fDeltaTime );
	void UpdateSpriteFrame( TBOOL a_bPulse );
	void UpdateSpin( TFLOAT a_fDeltaTime );
	void UpdateWobble( TFLOAT a_fDeltaTime );
	void UpdateFrameAnimation( TFLOAT a_fDeltaTime );
	void UpdateGlows( TFLOAT a_fDeltaTime );

	static void PlayLifeGainedSounds();

public:
	TBOOL                        m_bCreated;                      // 0x78
	Toshi::T2GUIRectangle        m_aGlows[ NUM_GLOWS ];           // 0x7c
	Toshi::T2GUIElement          m_oNumberContainer;              // 0x1e4
	Toshi::T2GUIRectangle        m_oSprite;                       // 0x238
	Toshi::T2GUIString           m_oNumberString;                 // 0x2b0
	TINT                         m_iLives;                        // 0x314
	TINT                         m_iDisplayedLives;               // 0x318, m_oNumberString shows this one
	TINT                         m_iNumSprites;                   // 0x31c
	TINT                         m_iNumLoopFrames;                // 0x320
	TINT                         m_iNumIntroFrames;               // 0x324
	TUINT8                       PADDING1[ 4 ];                   // 0x328
	TFLOAT                       m_fSpin;                         // 0x32c, in turns, the sprite frame is picked from its fraction
	TFLOAT                       m_fSpinTarget;                   // 0x330, the change animation is over once m_fSpin equals it
	TFLOAT                       m_fPopTime;                      // 0x334, the number pops while > 0
	TFLOAT                       m_fScale;                        // 0x338
	TFLOAT                       m_fPulse;                        // 0x33c, extra sprite scale while spinning
	TFLOAT                       m_fRotation;                     // 0x340, sprite rotation
	TFLOAT                       m_fWobble;                       // 0x344
	TFLOAT                       m_fWobbleTarget;                 // 0x348
	TFLOAT                       m_fWobblePhase;                  // 0x34c
	TFLOAT                       m_afGlowAngle[ NUM_GLOWS ];      // 0x350
	TFLOAT                       m_afGlowScalePhase[ NUM_GLOWS ]; // 0x35c
	TFLOAT                       m_afGlowAlphaPhase[ NUM_GLOWS ]; // 0x368
	TFLOAT                       m_fGlowFadeIn;                   // 0x374
	TFLOAT                       m_fGlowDuration;                 // 0x378
	TFLOAT                       m_fGlowTimeLeft;                 // 0x37c
	TFLOAT                       m_fIntroSpeed;                   // 0x380
	TFLOAT                       m_fLoopSpeed;                    // 0x384
	TFLOAT                       m_fSpriteOffsetX;                // 0x388
	TBOOL                        m_bWobbleSpin;                   // 0x38c
	Toshi::T2GUITextureSection** m_ppSpriteSections;              // 0x390, [ m_iNumSprites ], TMalloc'd by Create
	Toshi::T2GUITextureSection** m_ppLoopFrames;                  // 0x394
	Toshi::T2GUITextureSection** m_ppIntroFrames;                 // 0x398
	TWCHAR                       m_wszLives[ 4 ];                 // 0x39c
	const TCHAR**                m_ppSprites;                     // 0x3a4
};

TSTATICASSERT( sizeof( AGUI2LifeHud ) == 0x3a8 );
