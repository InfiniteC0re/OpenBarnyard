#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "AGameState.h"

#  include <TGUI/TGUIEvents.h>
#  include <Toshi/TEvent.h>
#  include <Math/TVector2.h>
#  include <Math/TVector4.h>
#  include <Math/TMatrix44.h>

class ACamera;
class ACameraHelper;
class AInteriorNetwork;
class PGUIButton;
class PGUISliderBar;

namespace Toshi
{
class TGUIScreen;
class TGUIWidget;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: level editor of the interior networks (the pathfinding networks of
// AInteriorNetworkManager, Data/Networks/*.trb): a free camera, the mouse
// pointer picks and drags the corners of the areas and the goals, the dialogs
// AInteriorNetworkPlacementDialog / AInteriorNetworkPlacementDialog2 of
// Data/GUI/main.gui (legacy TOSHI GUI, hosted by AGUISystem) hold the tools.
//-----------------------------------------------------------------------------
class AInteriorNetworkPlacementState
    : public AGameState
{
public:
	TDECLARE_CLASS( AInteriorNetworkPlacementState, AGameState );

	// NOTE: Nicktoons modes: 0 select, 1 drag (move the selected node), 2 new link, 3 new node, 5 flood check.
	// Barnyard has no counterpart of the graph tools, only the select and drag modes are used.
	enum MODE : TINT
	{
		MODE_SELECT     = 0,
		MODE_DRAG       = 1,
		MODE_NEW_LINK   = 2, // Nicktoons only
		MODE_NEW_NODE   = 3, // Nicktoons only
		MODE_FLOOD_FILL = 5, // Nicktoons only
	};

	// What the pointer can pick (a Nicktoons node)
	enum ELEMENT : TINT
	{
		ELEMENT_NONE,
		ELEMENT_POINT, // corner m_iIndex of area m_iArea
		ELEMENT_GOAL,  // goal m_iIndex
	};

	struct Element
	{
		ELEMENT eType  = ELEMENT_NONE;
		TINT    iArea  = -1;
		TINT    iIndex = -1;

		TBOOL IsValid() const { return eType != ELEMENT_NONE; }
		TBOOL operator==( const Element& a_rcOther ) const { return eType == a_rcOther.eType && iArea == a_rcOther.iArea && iIndex == a_rcOther.iIndex; }
		TBOOL operator!=( const Element& a_rcOther ) const { return !( *this == a_rcOther ); }
	};

	// The tabs of the main dialog (the Options, Other, Area and Point buttons)
	enum PANEL : TINT
	{
		PANEL_OPTIONS,
		PANEL_OTHER,
		PANEL_AREA,
		PANEL_POINT,
		PANEL_NUMOF
	};

	// The texts of the labels and text boxes (their parameters keep the pointer, not a copy)
	enum LABEL : TINT
	{
		LABEL_POSITION,
		LABEL_AREA,
		LABEL_COST,
		LABEL_NETWORK,
		LABEL_NUMOF
	};

	static constexpr TINT   LABEL_LENGTH        = 64;
	static constexpr TINT   NUM_POINTER_BUTTONS = 2;
	static constexpr TINT   NUM_HELD_COMMANDS   = 10; // HELDCOMMAND_ bits
	static constexpr TINT   MAX_BUTTONS         = 64;
	static constexpr TINT   MAX_EDITS           = 64;
	static constexpr TINT   MAX_TOUCHED_AREAS   = 32;
	static constexpr TFLOAT DRAG_HOLD_TIME      = 0.2f; // 0x2ae6c0
	static constexpr TFLOAT CAMERA_SPEED        = 6.0f; // flt_4882e0
	static constexpr TFLOAT CAMERA_TURN_SPEED   = 2.0f; // flt_4882e4
	static constexpr TFLOAT NODE_PICK_SIZE      = 0.12f;
	static constexpr TFLOAT NODE_CLOSE_DISTANCE = 0.3f;   // flt_4882e8
	static constexpr TFLOAT GRID_SIZE           = 4.0f;   // flt_4882f4, the default of the GridSlider
	static constexpr TFLOAT PICK_DISTANCE       = 100.0f; // length of the pointer ray
	static constexpr TFLOAT DRAW_DISTANCE       = 40.0f;  // NOTE: ours, times the DrawDist text box
	static constexpr TFLOAT POINT_SNAP_DISTANCE = 0.5f;   // NOTE: ours, the Points snap toggle

	using ButtonListener = Toshi::TListener<PGUIButton, Toshi::TGUIActionEvent, AInteriorNetworkPlacementState>;
	using PaintListener  = Toshi::TListener<Toshi::TGUIScreen, Toshi::TGUIScreenPaintEvent, AInteriorNetworkPlacementState>;

public:
	AInteriorNetworkPlacementState();
	~AInteriorNetworkPlacementState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) override;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnInsertion() override;
	virtual void  OnRemoval() override;
	virtual void  OnActivate() override;
	virtual void  OnDeactivate() override;

	AInteriorNetwork* GetNetwork() const { return m_pNetwork; }
	MODE              GetMode() const { return m_eMode; }

	// The selection, drawn by AWorldDebugDraw
	const Element& GetSelected() const { return m_oSelected; }
	const Element& GetHover() const { return m_oHover; }
	TINT           GetSelectedArea() const { return m_iSelectedArea; }
	TINT           GetHoverArea() const { return m_iHoverArea; }
	TFLOAT         GetDrawDistance() const { return DRAW_DISTANCE * m_fDrawDistance; }
	TBOOL          IsCostHighlighted() const { return m_bCostHighlight; }

	// World position of a corner (at the height of its area) or a goal
	Toshi::TVector4 GetElementPosition( const Element& a_rcElement );

	// World height (y) of a corner, resolved like the network does for the units (kept until the corner moves)
	TFLOAT GetCornerHeight( TINT a_iArea, TINT a_iCorner );

private:
	void SetMode( MODE a_eMode );

	// Dialogs (Nicktoons creates its buttons here)
	void CreateDialogs();
	void DestroyDialogs();
	void ShowMainDialog( TBOOL a_bShow );
	void ShowPanel( PANEL a_ePanel );

	// Pointer
	TBOOL PickGround( Toshi::TVector4& a_rOutPosition, Toshi::TVector4* a_pOutRayStart, Toshi::TVector4* a_pOutRayDirection );
	void  SnapToGrid( Toshi::TVector4& a_rPosition );
	void  UpdateHover();
	void  OnPointerButtonDown( TINT a_iButton );
	void  OnPointerButtonHeld( TINT a_iButton );
	void  OnPointerButtonUp( TINT a_iButton );

	// Selection of an area (Nicktoons: the link's labels)
	void SelectArea( TINT a_iArea );
	void UpdateSelectionInfo();
	void UpdatePositionLabel( const Toshi::TVector4* a_pPosition );

	// Network
	void              SetNetwork( AInteriorNetwork* a_pNetwork );
	void              InvalidateCornerHeights( TINT a_iArea );
	void              CreateCornerHeights();
	void              MoveElement( const Element& a_rcElement, TFLOAT a_fX, TFLOAT a_fZ, TBOOL a_bRecord );
	void              MovePoints( TFLOAT a_fFromX, TFLOAT a_fFromZ, TFLOAT a_fToX, TFLOAT a_fToZ );
	void              UpdateArea( TINT a_iArea );
	void              UpdateCornerNormals( TFLOAT a_fX, TFLOAT a_fZ );
	void              UpdateAreaCornerNormals( TINT a_iArea );
	TBOOL             DeletePoint( const Element& a_rcElement );
	TBOOL             SaveNetwork();
	void              RecordEdit( const Element& a_rcElement, const Toshi::TVector2& a_rcFrom, const Toshi::TVector2& a_rcTo );
	void              UndoEdit();
	void              RedoEdit();

	// Camera (the fly camera of the input map)
	void UpdateCamera( TFLOAT a_fDeltaTime );
	void TurnCamera( TFLOAT a_fYaw, TFLOAT a_fPitch );

	// Draws the network under the dialogs (the TGUI screen paints them)
	void Render( Toshi::TGUIScreen* a_pScreen );

	// Dialog helpers
	Toshi::TGUIWidget* FindWidget( const TCHAR* a_szName ) const;
	void               SetWidgetLabel( Toshi::TGUIWidget* a_pWidget, const TWCHAR* a_wszLabel );
	void               SetWidgetLabel( const TCHAR* a_szName, const TWCHAR* a_wszLabel );
	TBOOL              GetWidgetLabel( const TCHAR* a_szName, TCHAR* a_szOutText, TINT a_iMaxLength ) const;
	TBOOL              IsToggled( const TCHAR* a_szName ) const;

	static TBOOL __stdcall FindButtonsCallback( Toshi::TGUIWidget* a_pWidget, void* a_pUserData );
	static TBOOL __stdcall OnButtonAction( AInteriorNetworkPlacementState* a_pState, PGUIButton* a_pButton, Toshi::TGUIActionEvent* a_pEvent );
	static TBOOL __stdcall OnScreenPaint( AInteriorNetworkPlacementState* a_pState, Toshi::TGUIScreen* a_pScreen, Toshi::TGUIScreenPaintEvent* a_pEvent );

	// The buttons of the dialogs (Nicktoons: one callback per button)
	void OnQuit();
	void OnDeleteSelected();
	void OnSave();
	void OnToggleSnapToGrid();
	void OnLoadNetwork();
	void OnChangeCost( TFLOAT a_fSign );
	void OnGoalFacing();

private:
	// NOTE: the offsets are the ones of Nicktoons' AAINetworkCreationState
	MODE            m_eMode;                                     // 0x58
	ACamera*        m_pCamera;                                   // 0x5c
	Toshi::TVector4 m_vCameraForward;                            // 0x60, normalised forward of the camera
	Element         m_oHover;                                    // 0x70, node under the pointer
	TINT            m_iHoverArea;                                // 0x74, Nicktoons: link under the pointer
	Element         m_oSelected;                                 // 0x78, selected node
	TINT            m_iSelectedArea;                             // 0x7c, Nicktoons: selected link
	TBOOL           m_abPointerButton[ NUM_POINTER_BUTTONS ];    // 0x80, 0x88
	TFLOAT          m_afPointerButtonTime[ NUM_POINTER_BUTTONS ]; // 0x84, 0x8c, how long the button is held
	TBOOL           m_bSnapToGrid;                               // 0x94
	TBOOL           m_bFreeCamera;                               // 0x98, KEY_C: the camera takes the input
	TBOOL           m_bDirty;                                    // 0x104, the network was changed
	TBOOL           m_bQuit;                                     // 0x108

	// NOTE: Barnyard additions
	AInteriorNetwork* m_pNetwork;
	ACameraHelper*    m_pOldCameraHelper;
	Toshi::TMatrix44  m_oOldCameraMatrix;
	Toshi::TVector4   m_vCursor; // ground under the pointer
	TBOOL             m_bCursorValid;
	Element           m_oDragged;
	Toshi::TVector2   m_vDragStart;
	TINT              m_iHeldCommands; // bits of the camera commands that are down
	// The key that set each bit of m_iHeldCommands: the input map sends no command for the up events, so
	// UpdateCamera asks the device whether the key is still down
	Toshi::TInputDevice* m_apHeldDevices[ NUM_HELD_COMMANDS ];
	TINT                 m_aiHeldDoodads[ NUM_HELD_COMMANDS ];
	TFLOAT            m_fGridSize;
	TFLOAT            m_fDrawDistance;
	TFLOAT            m_fCostStep;
	TBOOL             m_bCostHighlight; // the CostHighlight toggle

	Toshi::TGUIWidget* m_pDialog;  // AInteriorNetworkPlacementDialog
	Toshi::TGUIWidget* m_pDialog2; // AInteriorNetworkPlacementDialog2 (Show UI)
	PANEL              m_ePanel;
	TINT               m_iNumButtons;
	TINT               m_iNumMainButtons;
	TINT               m_aiTouchedAreas[ MAX_TOUCHED_AREAS ];
	TINT               m_iNumTouchedAreas;
	TWCHAR             m_awszLabels[ LABEL_NUMOF ][ LABEL_LENGTH ];
	TFLOAT*            m_pfCornerHeights; // per corner of the network, TMath::MAXFLOAT until resolved
	TINT*              m_piCornerOffsets; // per area, its first corner in m_pfCornerHeights
	ButtonListener     m_aButtonListeners[ MAX_BUTTONS ];
	PaintListener      m_oPaintListener;

	struct Edit
	{
		Element         oElement;
		Toshi::TVector2 vFrom;
		Toshi::TVector2 vTo;
	};

	Edit m_aEdits[ MAX_EDITS ];
	TINT m_iNumEdits;
	TINT m_iNumUndone;
};

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
