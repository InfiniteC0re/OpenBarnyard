#pragma once
#include "GameInterface/AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "HUD/AButtonSlamHudElement.h"
#include "HUD/ATimeLeftHudElement.h"

#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AUnit;
class ASteer;
class AInteractionHudElement;

//-----------------------------------------------------------------------------
// Purpose: the player picks up a milk can ("milkcan" regrowth object in front of
// the player) and fills a milk carton from it. The can can then be churned by
// pressing interact repeatedly before the time runs out, once into cream and once
// more into butter; jump puts it down and keeps what was made so far. A failed
// churn loses the product. The product is collected once the can is put down.
// Added as a child state by name ("CreateAndAddChildState").
// TClass 00783a54, vtable 00723730, size 0x8f8.
//-----------------------------------------------------------------------------
class AMilkCanMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( AMilkCanMicroGame, AGameState );

	enum STATE : TINT
	{
		STATE_START         = 0, // plays urn_fill
		STATE_FILLING       = 1,
		STATE_FINISHED      = 4,  // the state is removed
		STATE_PUT_DOWN      = 5,  // plays urn_putdown
		STATE_PICKING_UP    = 6,  // urn_pickup, takes the can from the world
		STATE_HOLDING       = 7,  // plays urn_holdidle
		STATE_UNUSED_8      = 8,  // waits for urn_pickup to end, never entered
		STATE_PUTTING_DOWN  = 9,  // puts the can back to the world
		STATE_PUT_DOWN_WAIT = 10, // collects the product
		STATE_CHOOSING      = 11, // churn (interact) or put down (jump)
		STATE_HOLD_WAIT     = 12, // waits 2 seconds before showing the choice
		STATE_CHURN_WAIT    = 15, // waits for the first interact press
		STATE_CHURN_START   = 16, // urn_shakestart
		STATE_CHURNING      = 17, // urn_shakeloop
		STATE_CHURN_END     = 18, // urn_shakeend, then the result
	};

	enum PRODUCT : TINT
	{
		PRODUCT_MILK,
		PRODUCT_CREAM,
		PRODUCT_BUTTER,
	};

	enum STRING : TINT
	{
		STRING_putaway,
		STRING_NormalBarnyard,
		STRING_milkcarton,
		STRING_milkcan,
		STRING_butter,
		STRING_cream,
		STRING_Hand,
		STRING_urn_fill,
		STRING_urn_pickup,
		STRING_urn_shakestart,
		STRING_urn_shakeloop,
		STRING_urn_shakeend,
		STRING_urn_putdown,
		STRING_urn_holdidle,
		STRING_NUMOF
	};

public:
	AMilkCanMicroGame();
	~AMilkCanMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	// Runs the states
	void UpdateGame( TFLOAT a_fDeltaTime );

	// Shows the locale string a_iString, -1 fades out the shown one
	void ShowText( TINT a_iString );

	// Gives the player a unit of a_rcType spawned at a_rcPosition
	void CollectProduct( const Toshi::TPString8& a_rcType, const Toshi::TVector4& a_rcPosition );

private:
	static TFLOAT ms_fScreenWidth; // 0x00783a50, width of the root GUI element

private:
	TBOOL                   m_bCollect;                 // 0xa4, the product is collected when the can is put down
	Toshi::TVector4         m_vecDirection;             // 0xa8, towards the can
	ASteer*                 m_pSteer;                   // 0xb8
	AUnit*                  m_pPlayer;                  // 0xbc
	PRODUCT                 m_eProduct;                 // 0xc0
	STATE                   m_eState;                   // 0xc4
	TFLOAT                  m_fUnkC8;                   // 0xc8, -1.0f, never read
	AButtonSlamHudElement   m_oPowerBar;                // 0xcc
	TFLOAT                  m_fTime;                    // 0x598
	TFLOAT                  m_fChurnTime;               // 0x59c, the churn stops when it runs out
	TFLOAT                  m_fPower;                   // 0x5a0
	TFLOAT                  m_fUnk5A4;                  // 0x5a4, never used
	TBOOL                   m_bInteract;                // 0x5a8, pressed this frame
	TBOOL                   m_bJump;                    // 0x5a9, pressed this frame
	TBOOL                   m_bUnk5AA;                  // 0x5aa, TTRUE, never read
	AGUI2TextBox            m_oText;                    // 0x5ac
	Toshi::T2GUIFont*       m_pFont;                    // 0x624, "rekord26"
	TCHAR                   PADDING1[ 0x20 ];           // 0x628
	Toshi::TPString8        m_strProduct;               // 0x648
	TINT                    m_iRegrowthType;            // 0x64c
	void*                   m_pCan;                     // 0x650, ARegrowthManager::GrowthObject*
	TBOOL                   m_bTextFading;              // 0x654
	ATimeLeftHudElement     m_oTimeLeftHud;             // 0x658
	AInteractionHudElement* m_pInteractionHud;          // 0x88c
	Toshi::TPString8        m_aStrings[ STRING_NUMOF ]; // 0x890
	TCHAR                   PADDING2[ 0x10 ];           // 0x8c8
	TFLOAT                  m_fCameraAngle;             // 0x8d8, degrees
	TFLOAT                  m_fCameraSign;              // 0x8dc
	TFLOAT                  m_fCameraTime;              // 0x8e0
	TBOOL                   m_bCameraTurning;           // 0x8e4
	TCHAR                   PADDING3[ 0xf ];            // 0x8e5
	TBOOL                   m_bTextWasVisible;          // 0x8f4
};

TSTATICASSERT( sizeof( AMilkCanMicroGame ) == 0x8f8 );
