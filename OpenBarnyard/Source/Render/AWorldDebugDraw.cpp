#include "pch.h"
#include "AWorldDebugDraw.h"

#if !defined( TOSHI_FINAL ) && defined( TRENDERINTERFACE_DX8 )
#  include <Render/TCameraObject.h>
#  include "ADebugOverlay.h"
#  include "ALineShader/ALineShader.h"
#  include "Cameras/ACamera.h"
#  include "Cameras/ACameraManager.h"
#  include "Terrain/ATerrainInterface.h"
#  include "Terrain/ATerrainLocator.h"
#  include "Units/AUnit.h"
#  include "Units/AUnitManager.h"
#  include "UnitMovement/Steering/ASteer.h"
#  include "UnitMovement/Steering/AInteriorSteer.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#  ifdef BARNYARD_COMMUNITY_PATCH
#    include "GameInterface/AInteriorNetworkPlacementState.h"
#  endif

#  include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static const TCHAR* s_aLayerNames[ AWorldDebugDraw::LAYER_NUMOF ] = {
	"Interior networks",
	"Unit steering",
	"Terrain locators",
};

AWorldDebugDraw::AWorldDebugDraw()
    : m_pEditor( TNULL )
    , m_iNumHeights( 0 )
    , m_iHeightBudget( 0 )
    , m_iNumLines( 0 )
{
	for ( TINT i = 0; i < LAYER_NUMOF; i++ )
		m_abLayers[ i ] = TFALSE;
}

AWorldDebugDraw::~AWorldDebugDraw()
{
	InvalidateNetworkHeights( TNULL );
}

TBOOL AWorldDebugDraw::IsAnyLayerEnabled() const
{
	for ( TINT i = 0; i < LAYER_NUMOF; i++ )
	{
		if ( m_abLayers[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

const TCHAR* AWorldDebugDraw::GetLayerName( LAYER a_eLayer )
{
	return s_aLayerNames[ a_eLayer ];
}

void AWorldDebugDraw::InvalidateNetworkHeights( AInteriorNetwork* a_pNetwork )
{
	for ( TINT i = m_iNumHeights - 1; i >= 0; i-- )
	{
		if ( a_pNetwork && m_aHeights[ i ].pNetwork != a_pNetwork )
			continue;

		delete[] m_aHeights[ i ].pfHeights;
		delete[] m_aHeights[ i ].piOffsets;
		m_aHeights[ i ] = m_aHeights[ --m_iNumHeights ];
	}
}

void AWorldDebugDraw::Render()
{
	TBOOL bEditor = TFALSE;

#  ifdef BARNYARD_COMMUNITY_PATCH
	bEditor = m_pEditor && m_pEditor->GetNetwork();
#  endif

	if ( !IsAnyLayerEnabled() && !bEditor )
		return;

	ACamera*     pCamera     = ACameraManager::IsSingletonCreated() ? ACameraManager::GetSingleton()->GetCurrentCamera() : TNULL;
	ALineShader* pLineShader = ALineShader::IsSingletonCreated() ? ALineShader::GetSingleton() : TNULL;

	if ( !pCamera || !pLineShader )
		return;

	m_iHeightBudget = HEIGHTS_PER_FRAME;
	m_iNumLines     = 0;

	if ( m_abLayers[ LAYER_NETWORKS ] || bEditor )
		RenderNetworks( pCamera );

	if ( m_abLayers[ LAYER_STEERING ] )
		RenderSteering( pCamera );

	if ( m_abLayers[ LAYER_LOCATORS ] )
		RenderLocators( pCamera );

	// The line shader draws in the space of the model view matrix of the context (of the main viewport): the world
	TRenderD3DInterface* pRender    = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice    = pRender->GetDirect3DDevice();
	TRenderContext*      pContext   = pRender->GetCurrentContext();
	TMatrix44            oOldMatrix = pContext->GetModelViewMatrix();

	pContext->SetModelViewMatrix( pContext->GetWorldViewMatrix() );

	DWORD uiLighting, uiFog, uiZEnable, uiZWrite, uiAlphaBlend, uiAlphaTest;
	pDevice->GetRenderState( D3DRS_LIGHTING, &uiLighting );
	pDevice->GetRenderState( D3DRS_FOGENABLE, &uiFog );
	pDevice->GetRenderState( D3DRS_ZENABLE, &uiZEnable );
	pDevice->GetRenderState( D3DRS_ZWRITEENABLE, &uiZWrite );
	pDevice->GetRenderState( D3DRS_ALPHABLENDENABLE, &uiAlphaBlend );
	pDevice->GetRenderState( D3DRS_ALPHATESTENABLE, &uiAlphaTest );

	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );

	pLineShader->FlushLines();

	pDevice->SetRenderState( D3DRS_LIGHTING, uiLighting );
	pDevice->SetRenderState( D3DRS_FOGENABLE, uiFog );
	pDevice->SetRenderState( D3DRS_ZENABLE, uiZEnable );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, uiZWrite );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, uiAlphaBlend );
	pDevice->SetRenderState( D3DRS_ALPHATESTENABLE, uiAlphaTest );

	pContext->SetModelViewMatrix( oOldMatrix );
}

void AWorldDebugDraw::RenderNetworks( ACamera* a_pCamera )
{
	if ( !AInteriorNetworkManager::IsSingletonCreated() )
		return;

	const TVector4& rcCamera = a_pCamera->GetMatrix().GetTranslation();

#  ifdef BARNYARD_COMMUNITY_PATCH
	// The editor's network only, as far as its Draw Distance
	if ( m_pEditor && m_pEditor->GetNetwork() )
	{
		RenderNetwork( m_pEditor->GetNetwork(), rcCamera, m_pEditor->GetDrawDistance() );
		return;
	}
#  endif

	auto& rNetworks = AInteriorNetworkManager::GetSingleton()->GetNetworks();

	for ( auto it = rNetworks.Begin(); it != rNetworks.End(); it++ )
		RenderNetwork( it, rcCamera, NETWORK_DISTANCE );
}

void AWorldDebugDraw::RenderNetwork( AInteriorNetwork* a_pNetwork, const TVector4& a_rcCamera, TFLOAT a_fDistance )
{
	AInteriorNetwork::INetwork* pINetwork = a_pNetwork->m_pINetwork;

	if ( !pINetwork )
		return;

	NetworkHeights* pHeights        = GetNetworkHeights( a_pNetwork );
	TINT            iSelectedArea   = -1;
	TINT            iHoverArea      = -1;
	TBOOL           bCostHighlight  = TFALSE;

#  ifdef BARNYARD_COMMUNITY_PATCH
	if ( m_pEditor && m_pEditor->GetNetwork() == a_pNetwork )
	{
		iSelectedArea  = m_pEditor->GetSelectedArea();
		iHoverArea     = m_pEditor->GetHoverArea();
		bCostHighlight = m_pEditor->IsCostHighlighted();

		// The selected corner's area is selected too
		if ( m_pEditor->GetSelected().eType == AInteriorNetworkPlacementState::ELEMENT_POINT )
			iSelectedArea = m_pEditor->GetSelected().iArea;
	}
#  endif

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

		// Distance to the bounds of the area
		TFLOAT fDX = TMath::Max( 0.0f, TMath::Max( rcArea.m_fMinX - a_rcCamera.x, a_rcCamera.x - rcArea.m_fMaxX ) );
		TFLOAT fDZ = TMath::Max( 0.0f, TMath::Max( rcArea.m_fMinZ - a_rcCamera.z, a_rcCamera.z - rcArea.m_fMaxZ ) );

		if ( fDX * fDX + fDZ * fDZ > a_fDistance * a_fDistance )
			continue;

		// The cost (the heuristic scale of the path search) from green (1 or less) to red (4 or more)
		TUINT32 uiCostColour = 0;

		if ( bCostHighlight )
		{
			TFLOAT fCost = TMath::Min( TMath::Max( ( rcArea.m_fPathHeuristicScale - 1.0f ) / 3.0f, 0.0f ), 1.0f );
			uiCostColour = 0xff000000 | ( TUINT32( fCost * 255.0f ) << 16 ) | ( TUINT32( ( 1.0f - fCost ) * 255.0f ) << 8 );
		}

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		{
			const AInteriorArcBounds& rcA   = rcArea.m_pArcBounds[ i ];
			TINT                      iNext = ( i + 1 ) % rcArea.m_iNumArcBounds;
			const AInteriorArcBounds& rcB   = rcArea.m_pArcBounds[ iNext ];

			TFLOAT fHeightA, fHeightB;

			if ( !GetCornerHeight( a_pNetwork, pHeights, iArea, i, fHeightA ) || !GetCornerHeight( a_pNetwork, pHeights, iArea, iNext, fHeightB ) )
				continue;

			TVector4 vA( rcA.m_fX, fHeightA - LINE_LIFT, rcA.m_fZ, 1.0f );
			TVector4 vB( rcB.m_fX, fHeightB - LINE_LIFT, rcB.m_fZ, 1.0f );

			TUINT32 uiColour = ( rcA.m_iNeighborArea == -1 ) ? OUTER_EDGE_COLOUR : SHARED_EDGE_COLOUR;

			if ( iArea == iSelectedArea )
				uiColour = SELECTED_COLOUR;
			else if ( iArea == iHoverArea )
				uiColour = HOVER_COLOUR;
			else if ( bCostHighlight )
				uiColour = uiCostColour;

			AddLine( vA, vB, uiColour );

			if ( rcA.m_iNeighborArea == -1 )
			{
				// The normal of the outer edge, from its middle (points inside of the area)
				TVector4 vMiddle( ( vA.x + vB.x ) * 0.5f, ( vA.y + vB.y ) * 0.5f, ( vA.z + vB.z ) * 0.5f, 1.0f );
				TVector4 vNormal( vMiddle.x + rcA.m_fNormalX * 0.75f, vMiddle.y, vMiddle.z + rcA.m_fNormalZ * 0.75f, 1.0f );
				AddArrow( vMiddle, vNormal, NORMAL_COLOUR );

				// The normal of the corner
				if ( rcA.m_fCornerNormalX != 0.0f || rcA.m_fCornerNormalZ != 0.0f )
					AddLine( vA, TVector4( vA.x + rcA.m_fCornerNormalX * 0.4f, vA.y, vA.z + rcA.m_fCornerNormalZ * 0.4f, 1.0f ), NORMAL_COLOUR );
			}
		}
	}

	// The goals: a post and the facing
	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
	{
		AInteriorGoal& rGoal = pINetwork->m_pGoals[ i ];
		TFLOAT         fDX   = rGoal.m_fPositionX - a_rcCamera.x;
		TFLOAT         fDZ   = rGoal.m_fPositionZ - a_rcCamera.z;

		if ( fDX * fDX + fDZ * fDZ > a_fDistance * a_fDistance )
			continue;

		TVector4 vPosition;
		rGoal.GetPosition( vPosition );
		vPosition.y -= LINE_LIFT;

		TUINT32 uiColour = GOAL_COLOUR;

#  ifdef BARNYARD_COMMUNITY_PATCH
		if ( m_pEditor && m_pEditor->GetNetwork() == a_pNetwork )
		{
			if ( m_pEditor->GetSelected().eType == AInteriorNetworkPlacementState::ELEMENT_GOAL && m_pEditor->GetSelected().iIndex == i )
				uiColour = SELECTED_COLOUR;
			else if ( m_pEditor->GetHover().eType == AInteriorNetworkPlacementState::ELEMENT_GOAL && m_pEditor->GetHover().iIndex == i )
				uiColour = HOVER_COLOUR;
		}
#  endif

		AddLine( vPosition, TVector4( vPosition.x, vPosition.y - 1.5f, vPosition.z, 1.0f ), uiColour );
		AddArrow( vPosition, TVector4( vPosition.x + rGoal.m_vForward.x, vPosition.y, vPosition.z + rGoal.m_vForward.z, 1.0f ), uiColour );
	}

#  ifdef BARNYARD_COMMUNITY_PATCH
	// The selected corner: a taller post
	if ( m_pEditor && m_pEditor->GetNetwork() == a_pNetwork && m_pEditor->GetSelected().eType == AInteriorNetworkPlacementState::ELEMENT_POINT )
	{
		TVector4 vCorner = m_pEditor->GetElementPosition( m_pEditor->GetSelected() );
		AddLine( vCorner, TVector4( vCorner.x, vCorner.y - 2.0f, vCorner.z, 1.0f ), SELECTED_COLOUR );
		AddCross( vCorner, 0.3f, SELECTED_COLOUR );
	}
#  endif
}

void AWorldDebugDraw::RenderSteering( ACamera* a_pCamera )
{
	if ( !AUnitManager::IsSingletonCreated() )
		return;

	const TVector4& rcCamera = a_pCamera->GetMatrix().GetTranslation();
	auto&           rUnits   = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = rUnits.Begin(); it != rUnits.End(); it++ )
	{
		AUnit*  pUnit  = it;
		ASteer* pSteer = pUnit->GetSteer();

		if ( pUnit->IsDying() || !pSteer )
			continue;

		TVector4 vPosition = *pSteer->GetPosition();

		if ( TVector4::DistanceSq( vPosition, rcCamera ) > STEERING_DISTANCE * STEERING_DISTANCE )
			continue;

		vPosition.w = 1.0f;
		vPosition.y -= LINE_LIFT;

		// The radius and the heading
		TFLOAT fRadius = pSteer->GetRadius();
		AddCircle( vPosition, fRadius, UNIT_COLOUR );

		const TVector4* pDirection = pSteer->GetDirection();
		TFLOAT          fLength    = TMath::Max( fRadius * 2.0f, 1.0f );
		TVector4        vHeading( vPosition.x, vPosition.y - 0.5f, vPosition.z, 1.0f );
		AddArrow( vHeading, TVector4( vHeading.x + pDirection->x * fLength, vHeading.y, vHeading.z + pDirection->z * fLength, 1.0f ), UNIT_COLOUR );

		// The path: the next point and the destination
		if ( !pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
			continue;

		const AInteriorNetworkPath::BuildPathState& rcPath = TSTATICCAST( AInteriorSteer, pSteer )->GetPath().m_oPath;

		if ( TVector4::DistanceSq( rcPath.m_vDestination, *pSteer->GetPosition() ) < 0.04f )
			continue;

		// The points are on the ground of the network (or at zero if they weren't resolved)
		auto fnPoint = [ & ]( const TVector4& a_rcPoint ) {
			TFLOAT fY = ( TMath::Abs( a_rcPoint.y - vPosition.y ) < 5.0f ) ? a_rcPoint.y - LINE_LIFT : vPosition.y;
			return TVector4( a_rcPoint.x, fY, a_rcPoint.z, 1.0f );
		};

		TVector4 vNext        = fnPoint( rcPath.m_vPosition );
		TVector4 vDestination = fnPoint( rcPath.m_vDestination );

		AddLine( vPosition, vNext, PATH_COLOUR );
		AddLine( vNext, vDestination, DESTINATION_COLOUR );

		if ( rcPath.m_vForward.x != 0.0f || rcPath.m_vForward.z != 0.0f )
			AddArrow( vDestination, TVector4( vDestination.x + rcPath.m_vForward.x, vDestination.y, vDestination.z + rcPath.m_vForward.z, 1.0f ), DESTINATION_COLOUR );
	}
}

void AWorldDebugDraw::RenderLocators( ACamera* a_pCamera )
{
	ATerrainInterface*   pTerrain     = ATerrainInterface::GetSingleton();
	ATerrainLocatorList* pLocatorList = pTerrain ? pTerrain->GetLocatorList() : TNULL;

	if ( !pLocatorList || !pLocatorList->GetHeader() )
		return;

	const TVector4& rcCamera = a_pCamera->GetMatrix().GetTranslation();

	TMatrix44 oWorld;
	pLocatorList->GetWorldTransform().GetLocalMatrixImp( oWorld );

	for ( TINT i = 0; i < pLocatorList->GetNumLocators(); i++ )
	{
		TVector3 vLocal = pLocatorList->GetLocator( i )->vecPosition;
		TVector3 vWorld;
		TMatrix44::TransformVector( vWorld, oWorld, vLocal );

		TVector4 vPosition( vWorld.x, vWorld.y, vWorld.z, 1.0f );
		TFLOAT   fDistanceSq = TVector4::DistanceSq( vPosition, rcCamera );

		if ( fDistanceSq > LOCATOR_DISTANCE * LOCATOR_DISTANCE )
			continue;

		AddCross( vPosition, 0.2f, LOCATOR_COLOUR );
		AddLine( vPosition, TVector4( vPosition.x, vPosition.y - 0.5f, vPosition.z, 1.0f ), LOCATOR_COLOUR );

		if ( fDistanceSq < LOCATOR_NAME_DISTANCE * LOCATOR_NAME_DISTANCE && ADebugOverlay::IsSingletonCreated() )
			ADebugOverlay::GetSingleton()->PrintWorldText( a_pCamera, TVector4( vPosition.x, vPosition.y - 0.6f, vPosition.z, 1.0f ), pLocatorList->GetLocatorName( i ), LOCATOR_COLOUR );
	}
}

AWorldDebugDraw::NetworkHeights* AWorldDebugDraw::GetNetworkHeights( AInteriorNetwork* a_pNetwork )
{
	AInteriorNetwork::INetwork* pINetwork = a_pNetwork->m_pINetwork;

	for ( TINT i = 0; i < m_iNumHeights; i++ )
	{
		if ( m_aHeights[ i ].pNetwork == a_pNetwork )
		{
			// A network loaded at the same address
			if ( m_aHeights[ i ].pINetwork == pINetwork && m_aHeights[ i ].iNumAreas == pINetwork->m_iNumAreas )
				return &m_aHeights[ i ];

			InvalidateNetworkHeights( a_pNetwork );
			break;
		}
	}

	if ( m_iNumHeights >= MAX_NETWORKS )
		InvalidateNetworkHeights( m_aHeights[ 0 ].pNetwork );

	NetworkHeights& rHeights = m_aHeights[ m_iNumHeights++ ];
	rHeights.pNetwork        = a_pNetwork;
	rHeights.pINetwork       = pINetwork;
	rHeights.iNumAreas       = pINetwork->m_iNumAreas;
	rHeights.piOffsets       = new TINT[ TMath::Max( pINetwork->m_iNumAreas, 1 ) ];

	TINT iNumCorners = 0;

	for ( TINT i = 0; i < pINetwork->m_iNumAreas; i++ )
	{
		rHeights.piOffsets[ i ] = iNumCorners;
		iNumCorners += pINetwork->m_pAreas[ i ].m_iNumArcBounds;
	}

	rHeights.pfHeights = new TFLOAT[ TMath::Max( iNumCorners, 1 ) ];

	for ( TINT i = 0; i < iNumCorners; i++ )
		rHeights.pfHeights[ i ] = TMath::MAXFLOAT;

	return &rHeights;
}

TBOOL AWorldDebugDraw::GetCornerHeight( AInteriorNetwork* a_pNetwork, NetworkHeights* a_pHeights, TINT a_iArea, TINT a_iCorner, TFLOAT& a_rfHeight )
{
#  ifdef BARNYARD_COMMUNITY_PATCH
	// The editor keeps the heights of its network (the corners move)
	if ( m_pEditor && m_pEditor->GetNetwork() == a_pNetwork )
	{
		a_rfHeight = m_pEditor->GetCornerHeight( a_iArea, a_iCorner );
		return TTRUE;
	}
#  endif

	TFLOAT& rfHeight = a_pHeights->pfHeights[ a_pHeights->piOffsets[ a_iArea ] + a_iCorner ];

	if ( rfHeight == TMath::MAXFLOAT )
	{
		// Resolved like the network does for the units (rays), a few a frame
		if ( m_iHeightBudget <= 0 )
			return TFALSE;

		m_iHeightBudget--;

		const AInteriorArcBounds& rcCorner = a_pNetwork->m_pINetwork->m_pAreas[ a_iArea ].m_pArcBounds[ a_iCorner ];
		TVector4                  vPosition( rcCorner.m_fX, 0.0f, rcCorner.m_fZ, 1.0f );
		a_pNetwork->ResolvePointHeight( a_iArea, vPosition, 0 );
		rfHeight = vPosition.y;
	}

	a_rfHeight = rfHeight;
	return TTRUE;
}

void AWorldDebugDraw::AddLine( const TVector4& a_rcStart, const TVector4& a_rcEnd, TUINT32 a_uiColour )
{
	ALineShader::GetSingleton()->AddLineDirect( a_rcStart.AsVector3(), a_rcEnd.AsVector3(), a_uiColour, a_uiColour );
	m_iNumLines++;
}

void AWorldDebugDraw::AddArrow( const TVector4& a_rcStart, const TVector4& a_rcEnd, TUINT32 a_uiColour )
{
	AddLine( a_rcStart, a_rcEnd, a_uiColour );

	// The head, in the horizontal plane
	TFLOAT fX = a_rcStart.x - a_rcEnd.x;
	TFLOAT fZ = a_rcStart.z - a_rcEnd.z;

	TFLOAT fLength = TMath::Sqrt( fX * fX + fZ * fZ );

	if ( fLength < 0.001f )
		return;

	TFLOAT fScale = TMath::Min( 0.3f, fLength * 0.3f ) / fLength;
	fX *= fScale;
	fZ *= fScale;

	// +-30 degrees
	const TFLOAT fCos = 0.866f;
	const TFLOAT fSin = 0.5f;

	AddLine( a_rcEnd, TVector4( a_rcEnd.x + fX * fCos - fZ * fSin, a_rcEnd.y, a_rcEnd.z + fX * fSin + fZ * fCos, 1.0f ), a_uiColour );
	AddLine( a_rcEnd, TVector4( a_rcEnd.x + fX * fCos + fZ * fSin, a_rcEnd.y, a_rcEnd.z - fX * fSin + fZ * fCos, 1.0f ), a_uiColour );
}

void AWorldDebugDraw::AddCircle( const TVector4& a_rcCentre, TFLOAT a_fRadius, TUINT32 a_uiColour )
{
	const TINT NUM_SEGMENTS = 16;

	TVector4 vPrevious( a_rcCentre.x + a_fRadius, a_rcCentre.y, a_rcCentre.z, 1.0f );

	for ( TINT i = 1; i <= NUM_SEGMENTS; i++ )
	{
		TFLOAT   fAngle = TMath::TWO_PI * TFLOAT( i ) / NUM_SEGMENTS;
		TVector4 vPoint( a_rcCentre.x + TMath::Cos( fAngle ) * a_fRadius, a_rcCentre.y, a_rcCentre.z + TMath::Sin( fAngle ) * a_fRadius, 1.0f );
		AddLine( vPrevious, vPoint, a_uiColour );
		vPrevious = vPoint;
	}
}

void AWorldDebugDraw::AddCross( const TVector4& a_rcCentre, TFLOAT a_fSize, TUINT32 a_uiColour )
{
	AddLine( TVector4( a_rcCentre.x - a_fSize, a_rcCentre.y, a_rcCentre.z, 1.0f ), TVector4( a_rcCentre.x + a_fSize, a_rcCentre.y, a_rcCentre.z, 1.0f ), a_uiColour );
	AddLine( TVector4( a_rcCentre.x, a_rcCentre.y, a_rcCentre.z - a_fSize, 1.0f ), TVector4( a_rcCentre.x, a_rcCentre.y, a_rcCentre.z + a_fSize, 1.0f ), a_uiColour );
}

#endif // !TOSHI_FINAL && TRENDERINTERFACE_DX8
