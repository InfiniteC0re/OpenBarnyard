#pragma once

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
#  include <Toshi/TSingleton.h>
#  include <Math/TVector4.h>

class ACamera;
class AInteriorNetwork;
class AInteriorNetworkPlacementState;

//-----------------------------------------------------------------------------
// Purpose: development drawing in the world (not in Final builds) with the
// line shader (ALineShader, created by retail but never used). Each layer is
// a toggle of the debug menu (ADebugMenuState):
//  - the interior networks: the edges of the areas (outer cyan, shared blue),
//    the edge and corner normals (green), the goals (green posts with their
//    facing)
//  - the steering of the units: the radius and the heading (red), the next
//    point (magenta) and the destination (blue) of their path
//  - the terrain locators (yellow crosses with their names)
// Drawn with depth at the end of the main pass of ARenderer::RenderMainScene.
//-----------------------------------------------------------------------------
class AWorldDebugDraw
    : public Toshi::TSingleton<AWorldDebugDraw>
{
public:
	enum LAYER
	{
		LAYER_NETWORKS,
		LAYER_STEERING,
		LAYER_LOCATORS,
		LAYER_NUMOF
	};

	static constexpr TUINT32 OUTER_EDGE_COLOUR    = 0xff00ffff;
	static constexpr TUINT32 SHARED_EDGE_COLOUR   = 0xff2850ff;
	static constexpr TUINT32 NORMAL_COLOUR        = 0xff00ff40;
	static constexpr TUINT32 GOAL_COLOUR          = 0xff00ff40;
	static constexpr TUINT32 SELECTED_COLOUR      = 0xffffffff;
	static constexpr TUINT32 HOVER_COLOUR         = 0xffffe600;
	static constexpr TUINT32 UNIT_COLOUR          = 0xffff0000;
	static constexpr TUINT32 PATH_COLOUR          = 0xffff00ff;
	static constexpr TUINT32 DESTINATION_COLOUR   = 0xff2850ff;
	static constexpr TUINT32 LOCATOR_COLOUR       = 0xffffff00;
	static constexpr TFLOAT  NETWORK_DISTANCE     = 40.0f; // around the camera
	static constexpr TFLOAT  STEERING_DISTANCE    = 40.0f;
	static constexpr TFLOAT  LOCATOR_DISTANCE     = 30.0f;
	static constexpr TFLOAT  LOCATOR_NAME_DISTANCE = 12.0f;
	static constexpr TFLOAT  LINE_LIFT            = 0.05f; // above the ground (-Y is up)
	static constexpr TINT    MAX_NETWORKS         = 16;
	static constexpr TINT    HEIGHTS_PER_FRAME    = 256; // corner heights resolved per frame (rays)

public:
	AWorldDebugDraw();
	~AWorldDebugDraw();

	// Draws the layers that are on (ARenderer, main pass)
	void Render();

	void  SetLayerEnabled( LAYER a_eLayer, TBOOL a_bEnabled ) { m_abLayers[ a_eLayer ] = a_bEnabled; }
	TBOOL IsLayerEnabled( LAYER a_eLayer ) const { return m_abLayers[ a_eLayer ]; }
	TBOOL IsAnyLayerEnabled() const;

	static const TCHAR* GetLayerName( LAYER a_eLayer );

	// The network editor draws its network on the network layer (whether the layer is on or not) with its selection
	void SetNetworkEditor( AInteriorNetworkPlacementState* a_pEditor ) { m_pEditor = a_pEditor; }

	// Forgets the heights of the corners of a_pNetwork (TNULL: every network)
	void InvalidateNetworkHeights( AInteriorNetwork* a_pNetwork );

private:
	struct NetworkHeights
	{
		AInteriorNetwork* pNetwork;
		void*             pINetwork;
		TFLOAT*           pfHeights; // per corner, TMath::MAXFLOAT until resolved
		TINT*             piOffsets; // per area, its first corner in pfHeights
		TINT              iNumAreas;
	};

	void RenderNetworks( ACamera* a_pCamera );
	void RenderNetwork( AInteriorNetwork* a_pNetwork, const Toshi::TVector4& a_rcCamera, TFLOAT a_fDistance );
	void RenderSteering( ACamera* a_pCamera );
	void RenderLocators( ACamera* a_pCamera );

	NetworkHeights* GetNetworkHeights( AInteriorNetwork* a_pNetwork );
	TBOOL           GetCornerHeight( AInteriorNetwork* a_pNetwork, NetworkHeights* a_pHeights, TINT a_iArea, TINT a_iCorner, TFLOAT& a_rfHeight );

	void AddLine( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcEnd, TUINT32 a_uiColour );
	void AddArrow( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcEnd, TUINT32 a_uiColour );
	void AddCircle( const Toshi::TVector4& a_rcCentre, TFLOAT a_fRadius, TUINT32 a_uiColour );
	void AddCross( const Toshi::TVector4& a_rcCentre, TFLOAT a_fSize, TUINT32 a_uiColour );

private:
	TBOOL                           m_abLayers[ LAYER_NUMOF ];
	AInteriorNetworkPlacementState* m_pEditor;
	NetworkHeights                  m_aHeights[ MAX_NETWORKS ];
	TINT                            m_iNumHeights;
	TINT                            m_iHeightBudget; // corner heights that can still be resolved this frame
	TINT                            m_iNumLines;
};

#endif // !TOSHI_FINAL && TRENDERINTERFACE_DX8
