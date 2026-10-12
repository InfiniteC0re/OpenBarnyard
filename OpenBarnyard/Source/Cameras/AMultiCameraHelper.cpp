#include "pch.h"
#include "AMultiCameraHelper.h"
#include "ACameraManager.h"
#include "ACamera.h"
#include "Render/ARenderer.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AMultiCameraHelper );

// $Barnyard: FUNCTION 00464f60
AMultiCameraHelper::AMultiCameraHelper()
    : m_iNumTargets( 0 )
    , m_fSpeed( 1.8f )
    , m_fMinDistance( 0.0f )
    , m_fMaxDistance( 1000.0f )
{
}

// $Barnyard: FUNCTION 00464cb0
// $Barnyard: FUNCTION 00464fa0
AMultiCameraHelper::~AMultiCameraHelper()
{
}

// $Barnyard: FUNCTION 00465770
TBOOL AMultiCameraHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	return UpdateCamera( a_fDeltaTime, TFALSE );
}

// $Barnyard: FUNCTION 00464b60
void AMultiCameraHelper::OnCreate()
{
	ACameraManager::GetSingleton()->GetCurrentCamera()->GetDirection( m_vViewDirection );
	m_fDistance = m_fMinDistance;
}

// $Barnyard: FUNCTION 00464c00
void AMultiCameraHelper::OnDetach()
{
	ClearTargets();
}

// $Barnyard: FUNCTION 00464b00
void AMultiCameraHelper::ClearTargets()
{
	for ( TINT i = 0; i < m_iNumTargets; i++ )
		m_aTargets[ i ] = Target();

	m_iNumTargets = 0;
}

// $Barnyard: FUNCTION 00464ba0
TBOOL AMultiCameraHelper::AddTarget( const TVector4& a_rcPosition, TBOOL a_bOptional )
{
	Target& rTarget   = m_aTargets[ m_iNumTargets ];
	rTarget.vPosition = a_rcPosition;
	rTarget.bIsUnit   = TFALSE;
	rTarget.bOptional = a_bOptional;
	m_iNumTargets++;

	return TTRUE;
}

// $Barnyard: FUNCTION 00464cc0
TBOOL AMultiCameraHelper::AddTarget( TRef<AUnitProxy> a_pUnit, TBOOL a_bOptional )
{
	Target& rTarget   = m_aTargets[ m_iNumTargets ];
	rTarget.pUnit     = a_pUnit ? a_pUnit->GetUnit() : TNULL;
	rTarget.bIsUnit   = TTRUE;
	rTarget.bOptional = a_bOptional;
	m_iNumTargets++;

	return TTRUE;
}

// $Barnyard: FUNCTION 00464de0
void AMultiCameraHelper::RemoveTarget( const TVector4& a_rcPosition )
{
	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		if ( m_aTargets[ i ].vPosition.IsEqual( a_rcPosition ) )
		{
			RemoveTargetAt( i );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00464d40
void AMultiCameraHelper::RemoveTarget( TRef<AUnitProxy> a_pUnit )
{
	AUnit* pUnit = a_pUnit ? a_pUnit->GetUnit() : TNULL;

	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		// NOTE: bIsUnit isn't checked
		if ( m_aTargets[ i ].pUnit == pUnit )
		{
			RemoveTargetAt( i );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00464c10
void AMultiCameraHelper::RemoveTargetAt( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < m_iNumTargets - 1; i++ )
		m_aTargets[ i ] = m_aTargets[ i + 1 ];

	m_aTargets[ m_iNumTargets - 1 ] = Target();
	m_iNumTargets--;
}

const TVector4& AMultiCameraHelper::GetTargetPosition( const Target& a_rcTarget ) const
{
	return a_rcTarget.bIsUnit ? *a_rcTarget.pUnit->GetSteer()->GetPosition() : a_rcTarget.vPosition;
}

// $Barnyard: FUNCTION 00464e60
TBOOL AMultiCameraHelper::AreTargetsInView( TBOOL a_bIgnoreOptional )
{
	// NOTE: ARenderer::GetMainViewportSize inlined (only called from here), it outputs the size of the render context of the main viewport
	TRenderContext* pRenderContext = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext();
	TFLOAT          fScreenWidth   = pRenderContext->GetWidth();
	TFLOAT          fScreenHeight  = pRenderContext->GetHeight();

	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		const Target& rcTarget = m_aTargets[ i ];
		if ( a_bIgnoreOptional && rcTarget.bOptional ) continue;

		const TVector4& rcPosition = GetTargetPosition( rcTarget );

		TVector2 vScreenPos;
		ARenderer::GetSingleton()->ProjectToScreen( ACameraManager::GetSingleton()->GetCurrentCamera(), rcPosition, vScreenPos );

		if ( fScreenWidth * 0.125f > vScreenPos.x || fScreenWidth * 0.875f < vScreenPos.x ||
		     fScreenHeight * 0.125f > vScreenPos.y || fScreenHeight * 0.875f < vScreenPos.y )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00464fc0
TBOOL AMultiCameraHelper::UpdateCamera( TFLOAT a_fDeltaTime, TBOOL a_bIgnoreOptional )
{
	if ( m_iNumTargets <= 0 )
		return TFALSE;

	// Bounding rectangle of the targets on the screen
	TFLOAT fMinX = TMath::MAXFLOAT;
	TFLOAT fMaxX = -TMath::MAXFLOAT;
	TFLOAT fMinY = TMath::MAXFLOAT;
	TFLOAT fMaxY = -TMath::MAXFLOAT;

	for ( TINT i = 0; i < m_iNumTargets; i++ )
	{
		const Target& rcTarget = m_aTargets[ i ];
		if ( a_bIgnoreOptional && rcTarget.bOptional ) continue;

		const TVector4& rcPosition = GetTargetPosition( rcTarget );

		TVector2 vScreenPos;
		ARenderer::GetSingleton()->ProjectToScreen( ACameraManager::GetSingleton()->GetCurrentCamera(), rcPosition, vScreenPos );

		fMinY = TMath::Min( vScreenPos.y, fMinY );
		fMaxY = TMath::Max( fMaxY, vScreenPos.y );
		fMinX = TMath::Min( vScreenPos.x, fMinX );
		fMaxX = TMath::Max( fMaxX, vScreenPos.x );
	}

	// Find the ground under the centre of the rectangle
	TVector2 vScreenCentre( ( fMaxX + fMinX ) * 0.5f, ( fMaxY + fMinY ) * 0.5f );
	TVector4 vFocus;
	TVector4 vRayDirection;
	ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vScreenCentre, vFocus, vRayDirection );

	constexpr TCollisionCommon::TOSHICGROUPFLAG uiCollisionGroups = 0xC02B;

	TFLOAT   fRayLength = 100.0f;
	TVector4 vOffset;

	if ( ATerrainInterface::GetSingleton()->CollideRay( vFocus, vRayDirection, fRayLength, uiCollisionGroups, TFALSE ) )
	{
		vOffset.Multiply( vRayDirection, fRayLength );
		vFocus.Add( vOffset );
	}
	else
	{
		// Try behind the camera
		TVector4 vBackDirection;
		vBackDirection.Multiply( vRayDirection, -1.0f );

		if ( ATerrainInterface::GetSingleton()->CollideRay( vFocus, vBackDirection, fRayLength, uiCollisionGroups, TFALSE ) )
		{
			vOffset.Multiply( vRayDirection, fRayLength );
		}
		else
		{
			// Intersect the ray with the y = 0 plane
			TVector4 vScaled;
			vScaled.Multiply( vRayDirection, vFocus.y );
			vOffset.Divide( vScaled, vRayDirection.y );
		}

		vFocus.Substract( vOffset );
	}

	ACamera*  pCamera       = ACameraManager::GetSingleton()->GetCurrentCamera();
	TVector4& rCameraPos    = pCamera->GetMatrix().GetTranslation();
	TVector4  vOldCameraPos = rCameraPos;

	// The point the camera currently focuses on
	vOffset.Multiply( m_vViewDirection, m_fDistance );
	TVector4 vCurrentFocus;
	vCurrentFocus.Add( vOldCameraPos, vOffset );

	if ( !a_bIgnoreOptional )
	{
		// If the targets don't fit from the farthest distance, retry without the optional ones
		vOffset.Multiply( m_vViewDirection, m_fMaxDistance );
		rCameraPos.Substract( vFocus, vOffset );

		if ( !AreTargetsInView( TFALSE ) )
		{
			rCameraPos = vOldCameraPos;
			return UpdateCamera( a_fDeltaTime, TTRUE );
		}
	}

	// Move the focus point towards the new one
	TVector4 vToFocus;
	vToFocus.Substract( vFocus, vCurrentFocus );

	if ( vToFocus.Magnitude() <= 0.01f || a_fDeltaTime * m_fSpeed >= 1.0f )
	{
		vCurrentFocus = vFocus;
	}
	else
	{
		TVector4 vVelocity;
		vVelocity.Multiply( vToFocus, m_fSpeed );

		TVector4 vStep;
		vStep.Multiply( vVelocity, a_fDeltaTime );
		vCurrentFocus.Add( vStep );
	}

	// Binary search for the closest distance at which every target is in view
	TFLOAT fNear = m_fMinDistance;
	TFLOAT fFar  = m_fMaxDistance;

	if ( a_bIgnoreOptional )
		fNear = ( fNear + fFar ) * 0.5f;

	while ( fFar - fNear > 0.1f )
	{
		TFLOAT fDistance = ( fNear + fFar ) * 0.5f;

		vOffset.Multiply( m_vViewDirection, fDistance );
		rCameraPos.Substract( vCurrentFocus, vOffset );
		pCamera->LookAtDirection( m_vViewDirection );

		if ( AreTargetsInView( a_bIgnoreOptional ) )
			fFar = fDistance;
		else
			fNear = fDistance;
	}

	// Smoothly move to that distance
	TFLOAT fDiff = m_fDistance - fFar;

	if ( TMath::Abs( fDiff ) <= 0.001f )
		m_fDistance = fFar;
	else
		m_fDistance = m_fDistance - fDiff * m_fSpeed * a_fDeltaTime;

	vOffset.Multiply( m_vViewDirection, m_fDistance );
	rCameraPos.Substract( vCurrentFocus, vOffset );
	pCamera->LookAtDirection( m_vViewDirection );

	return TTRUE;
}
