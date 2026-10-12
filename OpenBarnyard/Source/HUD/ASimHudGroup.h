#pragma once

class AGameTimeHudElement;
class ASimPlayerMainHudElement;
class ASimPlayerPickUpHudElement;
class ATimeLeftHudElement;

//-----------------------------------------------------------------------------
// Purpose: the HUD elements of the sim (ASimState + 0xa8): the clock, the
// stamina and milk panel, the pick up counter and the timer that replaces the
// clock during timed quests.
// NOTE: not a TClass in retail, the name is made up.
//-----------------------------------------------------------------------------
class ASimHudGroup
{
public:
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE       = 0,
		FLAGS_SHOW_TIMER = BITFLAG( 0 ),
	};

public:
	ASimHudGroup();

	// NOTE: the destructor is nullsub_1 in retail, Destroy deletes the elements
	~ASimHudGroup() = default;

	// Creates the elements and adds them to AHudManager
	void Create();

	// Removes the elements from AHudManager and deletes them
	void Destroy();

	void Update( TFLOAT a_fDeltaTime );

	// Shows or hides every element (forced, AHudManager can't show them while hidden)
	void SetVisible( TBOOL a_bVisible );

	// Shows the timer instead of the clock (timed quests)
	void SetShowTimer( TBOOL a_bShowTimer );

	AGameTimeHudElement*        GetGameTimeElement() const { return m_pGameTime; }
	ASimPlayerMainHudElement*   GetMainElement() const { return m_pMain; }
	ASimPlayerPickUpHudElement* GetPickUpElement() const { return m_pPickUp; }
	ATimeLeftHudElement*        GetTimeLeftElement() const { return m_pTimeLeft; }

public:
	AGameTimeHudElement*        m_pGameTime; // 0x00
	ASimPlayerMainHudElement*   m_pMain;     // 0x04
	ASimPlayerPickUpHudElement* m_pPickUp;   // 0x08
	ATimeLeftHudElement*        m_pTimeLeft; // 0x0c
	FLAGS                       m_eFlags;    // 0x10
};

TSTATICASSERT( sizeof( ASimHudGroup ) == 0x14 );
