#pragma once
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>
#include <Math/TVector2.h>

// Aiming reticle of AAimedSquirtProjectileLauncher and of AMilkReticleManager: up to six
// rectangles (arrows) circling the position, closing in on it until the reticle is locked.
// The mode (SetMode) picks the texture, the number of arrows, the radius, the speeds and the colour.
// NOTE: the name is made up, the class has no RTTI (0x368 bytes, ctor 006491c0).
class ATargetReticle
{
public:
	static constexpr TINT NUM_RECTANGLES = 6;

public:
	// The look of the reticle, named after its users
	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_LOCKED         = 0,  // milk: locked on a target the milk gets to
		MODE_RELEASING      = 1,  // milk: a target is being released
		MODE_TARGET         = 2,  // cow tipping: aiming at a target
		MODE_BLOCKED        = 3,  // milk: locked on a target the milk doesn't get to (yellow)
		MODE_IDLE           = 4,  // no target
		MODE_TARGETS        = 5,  // milk: one of the targets
		MODE_CURRENT_TARGET = 6,  // milk: the current one of the targets (red)
		MODE_POWERUP_TARGET = 7,  // cow tipping: aiming at a target with the first power-up
		MODE_POWERUP_IDLE   = 8,  // cow tipping: no target with the first power-up
		MODE_GOLF           = 9,  // AGolfReticle
		MODE_NONE           = 11, // not set up yet
	};

public:
	ATargetReticle();
	~ATargetReticle();

	// Loads the texture sections and adds the rectangles to a_pParent (or the AGUI2 root element) at its centre.
	void Create( Toshi::T2GUIElement* a_pParent );

	// Shows the first m_iNumVisible rectangles while a_bActive is TTRUE, optionally setting the lock time.
	void SetActive( TBOOL a_bActive, TBOOL a_bSetLockTime, TFLOAT a_fLockTime );

	// Shrinks the rectangles as a_fLockTime grows.
	void UpdateSize( TFLOAT a_fLockTime );

	// Sets the colour (0-255 per component) the reticle is drawn with.
	void SetColour( TINT a_iA, TINT a_iR, TINT a_iG, TINT a_iB );

	// Clamped to positive coordinates.
	void SetPosition( const Toshi::TVector2& a_rcPosition );
	void SetPosition( TFLOAT a_fX, TFLOAT a_fY );
	void GetPosition( Toshi::TVector2& a_rOutPosition ) const;

	// Sets up the look of the reticle (0-9), see SetMode. Does nothing if it's already in that mode.
	void SetMode( MODE a_iMode );

	// Moves the radius towards the target radius, fades the colour and spins the arrows.
	void Update( TFLOAT a_fDeltaTime );

	// Places and rotates the arrows around m_vPosition.
	void UpdateRectangles();

	// The radius has reached the target radius
	TBOOL IsLocked() const;

	// $Barnyard: FUNCTION 00649160
	TBOOL IsActive() const { return m_bActive; }

	// $Barnyard: FUNCTION 00649170
	MODE GetMode() const { return m_iMode; }

private:
	// 0x007b50c8, "Target_Arrow", "Putt_Yel_Point", "right_arrow" and "Target_Circle"
	inline static Toshi::T2GUITextureSection* ms_apTextures[ 4 ];

private:
	Toshi::T2GUIRectangle m_aRectangles[ NUM_RECTANGLES ]; // 0x000
	Toshi::TVector2       m_vPosition;                     // 0x2d0
	Toshi::TVector2       m_aPositions[ NUM_RECTANGLES ];  // 0x2d8, of the arrows
	TFLOAT                m_fWidth;                        // 0x308
	TFLOAT                m_fHeight;                       // 0x30c
	TINT                  m_iNumVisible;                   // 0x310
	MODE                  m_iMode;                         // 0x314
	TINT                  m_iTexture;                      // 0x318, index of ms_apTextures
	TFLOAT                m_fAngle;                        // 0x31c, spin of the arrows
	TFLOAT                m_fScale;                        // 0x320, of the arrows
	TFLOAT                m_fRadius;                       // 0x324
	TFLOAT                m_fTargetRadius;                 // 0x328, locked once m_fRadius gets there
	TFLOAT                m_fSpinSpeed;                    // 0x32c, radians per second
	TFLOAT                m_fRadiusSpeed;                  // 0x330, per second
	TFLOAT                m_fColourSpeed;                  // 0x334
	TFLOAT                m_fColourProgress;               // 0x338, 0 to 1
	TFLOAT                m_fBaseAngle;                    // 0x33c
	TFLOAT                m_afColour[ 4 ];                 // 0x340, r, g, b, a (0-255)
	TFLOAT                m_afTargetColour[ 4 ];           // 0x350, r, g, b, a (0-255)
	TUINT32               m_uiColour;                      // 0x360, m_afColour packed as ARGB
	TBOOL                 m_bActive;                       // 0x364
	TBOOL                 m_bCreated;                      // 0x365
	TBOOL                 m_bStopSpinWhenLocked;           // 0x366
};

TSTATICASSERT( sizeof( ATargetReticle ) == 0x368 );
