#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Dialog.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

//-----------------------------------------------------------------------------
// Purpose: The map of the farm ("map" inventory item and the Select button of
// the sim). Shows Map_EnvMain with icons of the player, quests, items, gates...
// and the place names from Data/MinigameData/Map/mapdata.trb, at two zoom
// levels. The zoomed in map can be scrolled.
// TClass 00781a0c, vtable 0071aa98.
// NOTE: ProcessInput (+0x0c) is 00445300, a jmp thunk to AGameState::ProcessInput,
// and +0x60/+0x64 are 004452e0/004452f0, thunks to AGameState::OnActivate/OnDeactivate.
// They are not overridden here.
//-----------------------------------------------------------------------------
class AMapState
    : public AGameState
{
public:
	TDECLARE_CLASS( AMapState, AGameState );

	static constexpr TINT MAX_ICONS  = 300;
	static constexpr TINT MAX_LABELS = 40;

	using STATE = TUINT32;
	enum STATE_ : STATE
	{
		STATE_START,   // waits for the terrain if it's still loading
		STATE_LOADING, // shows the loading screen until the terrain is loaded
		STATE_CREATED, // the GUI has just been created
		STATE_SHOWING, // the map is shown, input is processed
		STATE_CLOSING, // hides the map and restores the HUD
		STATE_REMOVING,
	};

	// Children of the map, one pair for each zoom level
	using LAYER = TUINT32;
	enum LAYER_ : LAYER
	{
		LAYER_ZOOMEDOUT,     // icons and places
		LAYER_ZOOMEDOUT_TOP, // the player and the bike
		LAYER_ZOOMEDIN,
		LAYER_ZOOMEDIN_TOP,
		LAYER_NUMOF
	};

	// Texture sections of the icons
	using ICON = TINT;
	enum ICON_ : ICON
	{
		ICON_PLAYER,
		ICON_CHALLENGE,
		ICON_ITEM,
		ICON_MISSION,
		ICON_BIKE,
		ICON_GATE,
		ICON_CABBAGE,
		ICON_GOPHER,
		ICON_SAVE,
		ICON_HONEY,
		ICON_SLEEP,
		ICON_APPLES,
		ICON_LEMON,
		ICON_CHERRY,
		ICON_WALNUT,
		ICON_PLAYER_BIKE,
		ICON_PING_RING,
		ICON_NUMOF
	};

	// Entries of mapdata.trb with a type >= LABEL_FIRST are place names
	static constexpr TINT LABEL_FIRST = 101;

public:
	AMapState();
	~AMapState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54

private:
	void CreateGUI();

	// Adds an icon at a_fX, a_fY (0..1 on the map) to the layer
	void AddIcon( ICON a_eIcon, TFLOAT a_fX, TFLOAT a_fY, LAYER a_eLayer, TFLOAT a_fRotation, TBOOL a_bCentred, TFLOAT a_fScale );

	// Adds the place name a_iString (-1 for the name of the player) at a_fX, a_fY (0..1 on the map) to the layer
	void AddLabel( TINT a_iString, TFLOAT a_fX, TFLOAT a_fY, TBOOL a_bBig, LAYER a_eLayer );

	// Shows the zoom level and scrolls the zoomed in map to a_fScrollX, a_fScrollY
	void SetZoom( TINT a_iZoom, TFLOAT a_fScrollX, TFLOAT a_fScrollY );

private:
	Toshi::T2GUIRectangle       m_oMap;                         // 0x0a4, "Map_EnvMain"
	AGUI2Dialog                 m_oDialog;                      // 0x11c
	Toshi::T2GUIElement         m_aLayers[ LAYER_NUMOF ];       // 0x5d4
	Toshi::T2GUIRectangle       m_aArrows[ 4 ];                 // 0x724, show where the zoomed in map can be scrolled
	TFLOAT                      m_fMapWidth;                    // 0x904
	TFLOAT                      m_fMapHeight;                   // 0x908
	TFLOAT                      m_fScrollMarginY;               // 0x90c
	TWCHAR                      m_wszClose[ 64 ];               // 0x910
	TWCHAR                      m_wszZoomOut[ 64 ];             // 0x990
	TWCHAR                      m_wszZoomIn[ 64 ];              // 0xa10
	AGUI2TextBox                m_oClose;                       // 0xa90
	AGUI2TextBox                m_oZoomOut;                     // 0xb08
	AGUI2TextBox                m_oZoomIn;                      // 0xb80
	TINT                        m_iZoom;                        // 0xbf8
	TFLOAT                      m_fScrollX;                     // 0xbfc
	TFLOAT                      m_fScrollY;                     // 0xc00
	Toshi::T2GUIRectangle       m_aIcons[ MAX_ICONS ];          // 0xc04
	TINT                        m_iNumIcons;                    // 0x98a4
	AGUI2TextBox                m_aLabels[ MAX_LABELS ];        // 0x98a8
	TINT                        m_iNumLabels;                   // 0xab68
	Toshi::T2GUITextureSection* m_pMapSection;                  // 0xab6c
	TUINT                       m_uiMapBlendState;              // 0xab70, restored by OnRemoval
	Toshi::T2GUITextureSection* m_apIconSections[ ICON_NUMOF ]; // 0xab74
	TFLOAT                      m_fPingScale;                   // 0xabb8
	Toshi::T2GUIRectangle       m_oPing;                        // 0xabbc, ring pulsing around the player
	TINT                        m_iPlayerIcon;                  // 0xac34, the player icon of the zoomed out map, the next one is the zoomed in one
	STATE                       m_eState;                       // 0xac38
	TFLOAT                      m_fLoadTime;                    // 0xac3c
	TUINT32                     m_uiHUDMask;                    // 0xac40, HUD of the sim, restored when closing
};

TSTATICASSERT( sizeof( AMapState ) == 0xac44 );
