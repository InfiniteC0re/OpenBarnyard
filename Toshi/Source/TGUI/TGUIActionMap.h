#pragma once
#include <Toshi/TArray.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: maps the doodads (buttons) of the input devices to the input
// actions of the GUI, a bit mask (the navigation directions, accept,
// cancel...), each action having a label. The TGUIManager owns one.
//
// NOTE: the labels are TLStrings read from the locale by Create in JPOG,
// Nicktoons has no Create (and doesn't initialise the labels).
//-----------------------------------------------------------------------------
class TGUIActionMap : public TObject
{
public:
	TDECLARE_CLASS( TGUIActionMap, TObject );

	static constexpr TINT MAX_ACTIONS = 32;

	// The actions of the doodad (sorted by doodad)
	struct Mapping
	{
		TINT  iDoodad;   // 0x0
		TUINT uiActions; // 0x4
	};

public:
	TGUIActionMap();
	~TGUIActionMap();

	// Adds a_uiActions to the actions of a_iDoodad
	void AddActionMapping( TINT a_iDoodad, TUINT a_uiActions );

	// The actions of a_iDoodad, 0 if it has none
	TUINT TranslateDoodadToAction( TINT a_iDoodad ) const;

	// The label of the highest action of a_uiActions
	const TWCHAR* GetActionLabel( TUINT a_uiActions ) const;

	void SetActionLabel( TINT a_iAction, const TWCHAR* a_wszLabel ) { m_awszLabels[ a_iAction ] = a_wszLabel; }

private:
	// Index of the first mapping of a_iDoodad, -1 if there's none
	TINT FindMapping( TINT a_iDoodad ) const;

private:
	TArray<Mapping> m_aMappings;                 // 0x04
	const TWCHAR*   m_awszLabels[ MAX_ACTIONS ]; // 0x14 one per bit of the actions
};

TOSHI_NAMESPACE_END
