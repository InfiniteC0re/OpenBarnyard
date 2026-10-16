#pragma once
#include "ASimplePlayerControlComponent.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>

class AUnit;
class AInteriorArea;
class AInteractionSequence;
class AInteraction;
class APlayerInventoryCallbacks;

//-----------------------------------------------------------------------------
// Purpose: Base of objects that listen to the unit actions thrown by
// AAnimationBreakpointUnitAction (the global animation event list at
// 0x0077e35c). The constructor is inlined into the derived classes.
// NOTE: vtable 0x007201d4, shared by several unrelated classes.
//-----------------------------------------------------------------------------
class AUnitActionReceiver
{
public:
	AUnitActionReceiver();

	virtual void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent ) = 0; // +0x00
	virtual ~AUnitActionReceiver();                                                   // +0x04

private:
	// Forwards the event to OnUnitAction
	static void __stdcall UnitActionCallback( void* a_pCaller, AAnimationBreakpointUnitAction::Event* a_pEvent );

private:
	AAnimationBreakpointUnitAction::Listener m_oListener; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: Control component of the player in the sim (unit type "player").
// The movement comes from ASimplePlayerControlComponent, on top of it this
// handles the context action: it looks for the nearest unit, interior area or
// interaction the player can use (talk, pick up, use the item in the "Hand"
// inventory group, milk, fix leaks, open gates...), shows its text in the
// AInteractionHudElement and starts it with the action button.
// NOTE: the TClass parent is AControlComponent (0x0078a0f0) although the class
// derives from ASimplePlayerControlComponent in C++.
// TClass 0x0078a2a8 (version 0.1, size 0x1b0), vtables 0x00728b38 and 0x00728b84
// (AUnitActionReceiver).
//-----------------------------------------------------------------------------
class APlayerControlComponent
    : public ASimplePlayerControlComponent
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( APlayerControlComponent, AControlComponent );

public:
	APlayerControlComponent();
	~APlayerControlComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual void UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x2c

	//-----------------------------------------------------------------------------
	// AControlComponent
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent ) OVERRIDE; // +0x3c
	virtual void  OnDisable() OVERRIDE;                                                                                    // +0x40
	virtual void  OnEnable() OVERRIDE;                                                                                     // +0x44

	//-----------------------------------------------------------------------------
	// AUnitActionReceiver
	//-----------------------------------------------------------------------------
	virtual void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent ) OVERRIDE; // +0x00 (0x00728b84)

	// Stops the facial component from looking at things while there's no target
	void UpdateLookAt( AUnit* a_pOwner );

	// Releases the context interaction (m_pInteraction)
	TBOOL ReleaseInteraction();

	TBOOL CanUseHand();

	void SetContextEnabled( TBOOL a_bEnabled ) { m_bContextEnabled = a_bEnabled; }
	void SetOverrideText( const TWCHAR* a_wszText ) { m_wszOverrideText = a_wszText; }

	// Set by AQuestInstance while the player is locked by the "Sleepy Cows" quest
	void SetInventoryDisabled( TBOOL a_bValue ) { m_bInventoryDisabled = a_bValue; }
	void SetUnitInteractions( TBOOL a_bEnabled ) { m_bUnitInteractions = a_bEnabled; }
	void SetUseItemCooldown( TFLOAT a_fValue ) { m_fUseItemCooldown = a_fValue; }

#ifdef BARNYARD_COMMUNITY_PATCH
	// Dev: -devwalk feeds this fake left stick to the movement every update
	static void SetDevStick( TFLOAT a_fX, TFLOAT a_fY )
	{
		ms_bDevStick   = TTRUE;
		ms_vDevStick.x = a_fX;
		ms_vDevStick.y = a_fY;
	}

	// Dev: -devtalk presses the action button once on the next update
	static void DevInteract() { ms_bDevInteract = TTRUE; }

	AUnit* GetTargetUnit() const { return m_pTargetUnit; }

	// NOTE: not in retail
	inline static TBOOL           ms_bDevStick = TFALSE;
	inline static Toshi::TVector2 ms_vDevStick;
	inline static TBOOL           ms_bDevInteract = TFALSE;
#endif

#ifdef BARNYARD_LATEST
	// Latest version (Wii 0x806b08f0, set by the constructor): while it and AGameLoader::IsPigApplePieReturn are set, the
	// commands are ignored and UpdateAI presses the action button once the Pig is the target
	inline static TBOOL ms_bPigApplePieInteract = TFALSE;
#endif

public:
	// Also used by ABarnExploreMinigameState (root's interaction)
	AInteractionSequence* FindUnitInteraction( AUnit* a_pUnit, TBOOL a_bAnyState );
	TBOOL                 ReleaseInteraction( AInteractionSequence** a_ppSequence );

	// Also used by ASuperSquirtMicroGame
	TBOOL IsCarryingTool();

private:
	void                  UpdateInventoryGroups( AUnit* a_pOwner );
	TBOOL                 IsInteractionAllowed( AUnit* a_pOwner );
	TBOOL                 IsInteracting();
	AInteractionSequence* GetConversationSequence();
	TBOOL                 SelectConversationTarget();
	TBOOL                 UseLauncher();
	TBOOL                 IsActionAllowed( AUnit* a_pOwner );
	void                  UpdateBushes( TFLOAT a_fDeltaTime );
	TBOOL                 IsAreaInReach( AInteriorArea* a_pArea );
	void*                 GetAreaInteraction();
	void*                 GetRegrowthInteraction();
	TBOOL                 HasFlowersForBen();
	AUnit*                GetHandItem();
	AInteractionSequence* StartUnitInteraction( AUnit* a_pUnit, const TCHAR* a_szName );
	void*                 GetGateInteraction();
	TBOOL                 UseHandItem();
	void                  PutDownHandItem( AAnimationBreakpointUnitAction::Event* a_pEvent );
	TBOOL                 DropHeavyItem();
	TBOOL                 FixLeak();
	void                  StartAutoInteraction( AInteriorArea* a_pArea );
	void*                 GetMilkingInteraction();
	void*                 GetPresentInteraction();
	void                  UpdateContextText();
	void                  FindNearestTarget();
	void                  UpdateContext();
	void                  UpdateContextTimer( TFLOAT a_fDeltaTime );
	void                  UpdateCurrentArea( TFLOAT a_fDeltaTime );

	// The hud text of a_pInteraction, TNULL if it has none
	static const TWCHAR* GetHudText( AInteraction* a_pInteraction );

private:
	AInteriorArea*             m_pAutoArea;           // 0x50, area of the "auto" goal the player stands in
	AInteriorArea*             m_pArea;               // 0x54, area the player stands in
	AInteriorArea*             m_pContextArea;        // 0x58
	AUnit*                     m_pTargetUnit;         // 0x5c
	AUnit*                     m_pConversationUnit;   // 0x60
	TFLOAT                     m_fContextTime;        // 0x64
	TFLOAT                     m_fTargetDistanceSq;   // 0x68
	void*                      m_pRegrowth;           // 0x6c, the regrowth object in front of the player (ARegrowthManager)
	void*                      m_pGate;               // 0x70
	void*                      m_pLeak;               // 0x74, the leak in front of the player
	TINT                       m_iGateOrRegrowth;     // 0x78
	APlayerInventoryCallbacks* m_pInventoryCallbacks; // 0x7c, callbacks of the inventory groups of the owner
	AInteractionSequence*      m_pInteraction;        // 0x80, the interaction the context action starts
	TFLOAT                     m_fBushTime;           // 0x84
	TBOOL                      m_bContextEnabled;     // 0x88
	TBOOL                      m_bMoved;              // 0x89, the position changed during the last update
	TBOOL                      m_bAreaInteraction;    // 0x8a
	TBOOL                      m_bTargetCanTalk;      // 0x8b, m_pTargetUnit can be talked to
	TBOOL                      m_bFacingHint;         // 0x8c
	const TWCHAR*              m_wszOverrideText;     // 0x90
	const TWCHAR*              m_wszContextText;      // 0x94
	TWCHAR                     m_wszText[ 128 ];      // 0x98
	Toshi::TVector4            m_vLastPosition;       // 0x198
	TBOOL                      m_bLookAt;             // 0x1a8
	TBOOL                      m_bInventoryDisabled;  // 0x1a9
	TBOOL                      m_bUnitInteractions;   // 0x1aa
	TFLOAT                     m_fUseItemCooldown;    // 0x1ac
};

TSTATICASSERT( sizeof( APlayerControlComponent ) == 0x1b0 );
