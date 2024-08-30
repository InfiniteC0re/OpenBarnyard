#include "ToshiPCH.h"
#include "TRenderContext.h"
#include "TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TRenderContext::SetDirty( TBOOL a_bDirty )
{
	if ( a_bDirty ) m_eFlags |= FLAG_DIRTY;
	else
		m_eFlags &= ~FLAG_DIRTY;
}

void TRenderContext::SetFlag( FLAG a_eFlag, TBOOL a_bEnable )
{
	if ( a_bEnable ) m_eFlags |= a_eFlag;
	else
		m_eFlags &= ~a_eFlag;
}

void TRenderContext::EnableFog( TBOOL a_bEnable )
{
	if ( a_bEnable ) m_eFlags |= FLAG_FOG;
	else
		m_eFlags &= ~FLAG_FOG;
}

void TRenderContext::SetFogDistance( TFLOAT a_fStart, TFLOAT a_fEnd )
{
	m_fFogDistanceStart = a_fStart;
	m_fFogDistanceEnd   = a_fEnd;
	m_eFlags |= FLAG_DIRTY;
	m_eFlags &= ~( FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6 );
}

TRenderContext::TRenderContext( TRenderInterface* a_pRenderer ) :
    m_pRenderer( a_pRenderer ),
    m_eFlags( FLAG_DIRTY ),
    m_eClipFlags( 0x3F ),
    m_eCameraMode( CameraMode_Perspective ),
    m_pCurrentSkeletonInstance( TNULL ),
    m_pCurrentCameraObject( TNULL ),
    m_AmbientColor( 1.0f, 1.0f, 1.0f, 0.0f ),
    m_FogColor( 1.0f, 1.0f, 1.0f, 0.0f ),
    m_fFogDistanceStart( 10.0f ),
    m_fFogDistanceEnd( 1000.0f ),
    m_fAlphaBlend( 1.0f ),
    m_fShadeCoeff( 0.0f )
{
	TIMPLEMENT();

	// Setup viewport parameters
	if ( m_pRenderer->GetCurrentDevice() != TNULL )
	{
		auto pDevice              = m_pRenderer->GetCurrentDevice();
		auto pMode                = pDevice->GetMode();
		m_oViewportParams.fWidth  = TFLOAT( pMode->GetWidth() );
		m_oViewportParams.fHeight = TFLOAT( pMode->GetHeight() );
	}
	else
	{
		m_oViewportParams.fWidth  = 640.0f;
		m_oViewportParams.fHeight = 480.0f;
	}

	m_oViewportParams.fX    = 0;
	m_oViewportParams.fY    = 0;
	m_oViewportParams.fMaxZ = 1.0f;
	m_oViewportParams.fMinZ = 1.0f;

	// Setup projection parameters
	m_oProjParams.SetFromFOV( m_oViewportParams.fWidth, m_oViewportParams.fHeight, TMath::DegToRad( 45.0f ), 1.0f, 1000.0f );

	m_oModelViewMatrix.Identity();
	m_oWorldViewMatrix.Identity();
	m_eFlags |= FLAG_DIRTY_VIEWMODELMATRIX | FLAG_DIRTY_WORLDMODELMATRIX;

	m_AmbientColor.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	ClearLightIDs();
}

TRenderContext::~TRenderContext()
{
}

void TRenderContext::SetModelViewMatrix( const TMatrix44& a_rMatrix )
{
	m_eFlags |= ( FLAG_DIRTY_WORLDMODELMATRIX | FLAG_DIRTY_VIEWMODELMATRIX );
	m_oModelViewMatrix = a_rMatrix;
	m_eFlags &= ~( FLAG_HAS_MODELWORLDMATRIX | FLAG_UNK3 );
}

void TRenderContext::SetWorldViewMatrix( const TMatrix44& a_rMatrix )
{
	m_eFlags |= FLAG_DIRTY_VIEWMODELMATRIX;
	m_oWorldViewMatrix = a_rMatrix;
	m_eFlags &= ~( FLAG_HAS_MODELWORLDMATRIX | FLAG_HAS_VIEWWORLDMATRIX | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6 );
}

void TRenderContext::SetProjectionParams( const PROJECTIONPARAMS& params )
{
	TASSERT( params.m_Proj.x != 0.0f );
	TASSERT( params.m_Proj.y != 0.0f );
	TASSERT( TMath::IsFinite( params.m_Proj.x ) && ( !TMath::IsNaN( params.m_Proj.x ) ) );
	TASSERT( TMath::IsFinite( params.m_Proj.y ) && ( !TMath::IsNaN( params.m_Proj.y ) ) );
	TASSERT( TMath::IsFinite( params.m_Centre.x ) && ( !TMath::IsNaN( params.m_Centre.x ) ) );
	TASSERT( TMath::IsFinite( params.m_Centre.y ) && ( !TMath::IsNaN( params.m_Centre.y ) ) );

	m_oProjParams = params;
	m_eFlags      = ( m_eFlags & ( ~( FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6 ) ) ) | FLAG_DIRTY;
}

void TRenderContext::SetViewportParameters( const VIEWPORTPARAMS& params )
{
	m_oViewportParams = params;
	m_eFlags          = ( m_eFlags & ( ~( FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6 ) ) ) | FLAG_DIRTY;
}

void TRenderContext::SetCameraMode( CameraMode cameraMode )
{
	m_eCameraMode = cameraMode;
	m_eFlags      = ( m_eFlags & ( ~( FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6 ) ) ) | FLAG_DIRTY;
}

void TRenderContext::ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams )
{
	a_rOutProjection.m_f11 = ( a_rProjParams.m_Proj.x * 2.0f ) / a_rViewportParams.fWidth;
	a_rOutProjection.m_f12 = 0.0f;
	a_rOutProjection.m_f13 = 0.0f;
	a_rOutProjection.m_f14 = 0.0f;
	a_rOutProjection.m_f21 = 0.0f;
	a_rOutProjection.m_f22 = -( ( a_rProjParams.m_Proj.y * 2.0f ) / a_rViewportParams.fHeight );
	a_rOutProjection.m_f23 = 0.0f;
	a_rOutProjection.m_f24 = 0.0f;
	a_rOutProjection.m_f31 = ( a_rProjParams.m_Centre.x * 2.0f ) / a_rViewportParams.fWidth - 1.0f;
	a_rOutProjection.m_f32 = -( ( a_rProjParams.m_Centre.y * 2.0f ) / a_rViewportParams.fHeight - 1.0f );
	a_rOutProjection.m_f33 = a_rProjParams.m_fFarClip / ( a_rProjParams.m_fFarClip - a_rProjParams.m_fNearClip );
	a_rOutProjection.m_f34 = 1.0f;
	a_rOutProjection.m_f41 = 0.0f;
	a_rOutProjection.m_f42 = 0.0f;
	a_rOutProjection.m_f43 = -( ( a_rProjParams.m_fNearClip * a_rProjParams.m_fFarClip ) / ( a_rProjParams.m_fFarClip - a_rProjParams.m_fNearClip ) );
	a_rOutProjection.m_f44 = 0.0f;
}

void TRenderContext::ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams )
{
	TASSERT( a_rProjParams.m_Proj.x != 0.0f );
	TASSERT( a_rProjParams.m_Proj.y != 0.0f );
	TASSERT( a_rViewportParams.fWidth != 0.0f );
	TASSERT( a_rViewportParams.fHeight != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.x ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.y ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.x ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.y ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.y ) ) );

	a_rOutProjection.m_f11 = ( a_rProjParams.m_Proj.x * 2.0f ) / a_rViewportParams.fWidth;
	a_rOutProjection.m_f12 = 0.0f;
	a_rOutProjection.m_f13 = 0.0f;
	a_rOutProjection.m_f14 = 0.0f;
	a_rOutProjection.m_f21 = 0.0f;
	a_rOutProjection.m_f22 = -( ( a_rProjParams.m_Proj.y * 2.0f ) / a_rViewportParams.fHeight );
	a_rOutProjection.m_f23 = 0.0f;
	a_rOutProjection.m_f24 = 0.0f;
	a_rOutProjection.m_f31 = 0.0f;
	a_rOutProjection.m_f32 = 0.0f;
	a_rOutProjection.m_f33 = 1.0f / ( a_rProjParams.m_fFarClip - a_rProjParams.m_fNearClip );
	a_rOutProjection.m_f34 = 0.0f;
	a_rOutProjection.m_f41 = ( a_rProjParams.m_Centre.x * 2.0f ) / a_rViewportParams.fWidth - 1.0f;
	a_rOutProjection.m_f42 = -( ( a_rProjParams.m_Centre.y * 2.0f ) / a_rViewportParams.fHeight - 1.0f );
	a_rOutProjection.m_f43 = -( a_rProjParams.m_fNearClip / ( a_rProjParams.m_fFarClip - a_rProjParams.m_fNearClip ) );
	a_rOutProjection.m_f44 = 1.0f;
}

TUINT TRenderContext::GetClipFlags() const
{
	return m_eClipFlags;
}

TUINT TRenderContext::SetClipFlags( TUINT a_uiClipFlags )
{
	return std::exchange( m_eClipFlags, a_uiClipFlags );
}

const TPlane* TRenderContext::GetWorldPlanes()
{
	if ( !HASANYFLAG( m_eFlags, FLAG_HAS_WORLDPLANES ) )
	{
		auto& viewWorld = GetViewWorldMatrix();

		for ( TSIZE i = 0; i < WORLDPLANE_NUMOF; i++ )
		{
			TMatrix44::TransformPlaneOrthogonal( m_aWorldPlanes[ i ], viewWorld, m_aFrustumPlanes1[ i ] );
		}

		m_eFlags |= FLAG_HAS_WORLDPLANES;
	}

	return m_aWorldPlanes;
}

TBOOL TRenderContext::CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes )
{
	for ( TSIZE i = 0; i < WORLDPLANE_NUMOF; i++ )
	{
		TFLOAT fDist = TVector4::DotProduct3( a_rSphere.AsVector4(), a_pPlanes[ i ].AsVector4() );

		if ( a_rSphere.GetRadius() < fDist - a_pPlanes[ i ].GetD() )
			return TFALSE;
	}

	return TTRUE;
}

TINT TRenderContext::CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask )
{
	TINT iLeftPlanes = a_iClipFlags & a_iClipFlagsMask;
	TINT iPlaneFlag  = 1;

	do {
		if ( iLeftPlanes == 0 )
		{
			return a_iClipFlags;
		}

		if ( iLeftPlanes & iPlaneFlag )
		{
			TFLOAT fDist = TVector4::DotProduct3( a_rSphere.AsVector4(), a_pPlanes->AsVector4() ) - a_pPlanes->GetD();

			if ( a_rSphere.GetRadius() < fDist )
			{
				return -1;
			}

			if ( fDist < -a_rSphere.GetRadius() )
			{
				a_iClipFlags &= ~iPlaneFlag;
			}

			iLeftPlanes &= ~iPlaneFlag;
		}

		iPlaneFlag = iPlaneFlag << 1;
		a_pPlanes++;

	} while ( TTRUE );
}

const TMatrix44& TRenderContext::GetViewWorldMatrix()
{
	if ( !HASANYFLAG( m_eFlags, FLAG_HAS_VIEWWORLDMATRIX ) )
	{
		m_oViewWorldMatrix.InvertOrthogonal( m_oWorldViewMatrix );
		m_eFlags |= FLAG_HAS_VIEWWORLDMATRIX;
	}

	return m_oViewWorldMatrix;
}

const TMatrix44& TRenderContext::GetModelWorldMatrix()
{
	if ( !HASANYFLAG( m_eFlags, FLAG_HAS_MODELWORLDMATRIX ) )
	{
		m_oModelWorldMatrix.Multiply( GetViewWorldMatrix(), m_oModelViewMatrix );
		m_eFlags |= FLAG_HAS_MODELWORLDMATRIX;
	}

	return m_oModelWorldMatrix;
}

const TMatrix44& TRenderContext::GetViewModelMatrix()
{
	if ( HASANYFLAG( m_eFlags, FLAG_DIRTY_VIEWMODELMATRIX ) )
	{
		m_oViewModelMatrix.Invert( m_oModelViewMatrix );
		m_eFlags &= ~FLAG_DIRTY_VIEWMODELMATRIX;
	}

	return m_oViewModelMatrix;
}

const TMatrix44& TRenderContext::GetWorldModelMatrix()
{
	if ( HASANYFLAG( m_eFlags, FLAG_DIRTY_WORLDMODELMATRIX ) )
	{
		m_oViewModelMatrix.Invert( m_oModelViewMatrix );
		m_eFlags &= ~FLAG_DIRTY_WORLDMODELMATRIX;
	}

	return m_oWorldModelMatrix;
}

void TRenderContext::ClearLightIDs()
{
	m_oLightIds.Reset();
}

void TRenderContext::AddLight( TLightID a_iLightId )
{
	m_oLightIds.Add( a_iLightId );
}

void TRenderContext::ComputePerspectiveFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams )
{
	TASSERT( a_rProjParams.m_Proj.x != 0.0f );
	TASSERT( a_rProjParams.m_Proj.y != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.x ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.y ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.x ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.y ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.y ) ) );

	TFLOAT   fVal1 = 1.0f / a_rProjParams.m_Proj.x;
	TFLOAT   fVal2 = 1.0f / a_rProjParams.m_Proj.y;
	TVector4 vec1  = TVector4( -a_rProjParams.m_Centre.x * fVal1, ( a_rViewportParams.fHeight - a_rProjParams.m_Centre.y ) * fVal2, 1.0f, 0.0f );
	TVector4 vec2  = TVector4( ( a_rViewportParams.fWidth - a_rProjParams.m_Centre.x ) * fVal1, vec1.y, 1.0f, 0.0f );
	TVector4 vec3  = TVector4( vec2.x, -a_rProjParams.m_Centre.y * fVal2, 1.0f, 0.0f );
	TVector4 vec4  = TVector4( vec1.x, vec3.y, 1.0f, 0.0f );

	// Setup planes
	a_rcFrustum[ WORLDPLANE_LEFT ].AsVector4().CrossProduct( vec4, vec1 );
	a_rcFrustum[ WORLDPLANE_RIGHT ].AsVector4().CrossProduct( vec2, vec3 );
	a_rcFrustum[ WORLDPLANE_BOTTOM ].AsVector4().CrossProduct( vec3, vec4 );
	a_rcFrustum[ WORLDPLANE_TOP ].AsVector4().CrossProduct( vec1, vec2 );

	// Normalize each plane
	a_rcFrustum[ WORLDPLANE_LEFT ].AsNormal().Normalize();
	a_rcFrustum[ WORLDPLANE_RIGHT ].AsNormal().Normalize();
	a_rcFrustum[ WORLDPLANE_BOTTOM ].AsNormal().Normalize();
	a_rcFrustum[ WORLDPLANE_TOP ].AsNormal().Normalize();

	// Setup near and far planes
	a_rcFrustum[ WORLDPLANE_NEAR ].Set( 0.0f, 0.0f, -1.0f, -a_rProjParams.m_fNearClip );
	a_rcFrustum[ WORLDPLANE_FAR ].Set( 0.0f, 0.0f, 1.0f, a_rProjParams.m_fFarClip );
}

void TRenderContext::ComputeOrthographicFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams )
{
	TASSERT( a_rProjParams.m_Proj.x != 0.0f );
	TASSERT( a_rProjParams.m_Proj.y != 0.0f );
	TASSERT( a_rViewportParams.fWidth != 0.0f );
	TASSERT( a_rViewportParams.fHeight != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.x ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Proj.y ) && ( !TMath::IsNaN( a_rProjParams.m_Proj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.x ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.m_Centre.y ) && ( !TMath::IsNaN( a_rProjParams.m_Centre.y ) ) );

	TFLOAT fWidth        = a_rViewportParams.fWidth;
	TFLOAT fHeight       = a_rViewportParams.fHeight;
	TFLOAT fCentreX      = a_rProjParams.m_Centre.x;
	TFLOAT fCentreY      = a_rProjParams.m_Centre.y;
	TFLOAT fProjXOverOne = 1.0f / a_rProjParams.m_Proj.x;
	TFLOAT fProjYOverOne = 1.0f / a_rProjParams.m_Proj.y;

	// Setup planes
	a_rcFrustum[ WORLDPLANE_LEFT ].Set( -1.0f, 0.0f, 0.0f, fCentreX * fProjXOverOne );
	a_rcFrustum[ WORLDPLANE_RIGHT ].Set( 1.0f, 0.0f, 0.0f, ( fWidth - fCentreX ) * fProjXOverOne );
	a_rcFrustum[ WORLDPLANE_BOTTOM ].Set( 0.0f, -1.0f, 0.0f, fCentreY * fProjYOverOne );
	a_rcFrustum[ WORLDPLANE_TOP ].Set( 0.0f, 1.0f, 0.0f, ( fHeight - fCentreY ) * fProjYOverOne );
	a_rcFrustum[ WORLDPLANE_NEAR ].Set( 0.0f, 0.0f, -1.0f, -a_rProjParams.m_fNearClip );
	a_rcFrustum[ WORLDPLANE_FAR ].Set( 0.0f, 0.0f, 1.0f, a_rProjParams.m_fFarClip );
}

void TRenderContext::PROJECTIONPARAMS::SetFromFOV( TFLOAT a_fViewportWidth, TFLOAT a_fViewportHeight, TFLOAT a_fFOV, TFLOAT a_fNearPlane, TFLOAT a_fFarPlane )
{
	TFLOAT fHalfWidth  = a_fViewportWidth * 0.5f;
	TFLOAT fHalfHeight = a_fViewportHeight * 0.5f;
	TFLOAT fProj       = fHalfHeight / TMath::Tan( a_fFOV );

	m_fNearClip = 1.0f;
	m_fFarClip  = 1000.0f;
	m_Centre.x  = fHalfWidth;
	m_Centre.y  = fHalfHeight;
	m_Proj.x    = fProj;
	m_Proj.y    = fProj;
}

void TLightIDList::Add( TLightID a_iLightId )
{
	for ( TINT i = 0; i < 4; i++ )
	{
		if ( aIDs[ i ] == -1 )
		{
			aIDs[ i ] = a_iLightId;
			return;
		}
	}
}

void TLightIDList::Reset()
{
	aIDs[ 3 ] = -1;
	aIDs[ 2 ] = -1;
	aIDs[ 1 ] = -1;
	aIDs[ 0 ] = -1;
}

TLightID& TLightIDList::operator[]( TINT a_iIndex )
{
	TASSERT( a_iIndex < MAX_NUM_LIGHTS );
	return aIDs[ a_iIndex ];
}

TOSHI_NAMESPACE_END
