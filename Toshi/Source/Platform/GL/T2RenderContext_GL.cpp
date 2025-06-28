#include "ToshiPCH.h"
#include "T2RenderContext_GL.h"
#include "Math/TMatrix44.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Camera g_oDefaultCamera;

T2RenderContext::T2RenderContext()
    : m_uiCurrentShaderProgram( -1 )
    , m_iCurrentTextureUnit( 0 )
    , m_bForceRefreshFeatures( TFALSE )
    , m_pCamera( &g_oDefaultCamera )
    , m_pSkeletonInstance( TNULL )
{
	m_bDepthTest = TFALSE;
	glDisable( GL_DEPTH_TEST );

	m_bBlend = TFALSE;
	glDisable( GL_BLEND );
}

T2RenderContext::~T2RenderContext()
{
}

void T2RenderContext::ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams )
{
	a_rOutProjection.m_f11 = ( a_rProjParams.vecProj.x * 2.0f ) / a_rViewportParams.GetWidth();
	a_rOutProjection.m_f12 = 0.0f;
	a_rOutProjection.m_f13 = 0.0f;
	a_rOutProjection.m_f14 = 0.0f;
	a_rOutProjection.m_f21 = 0.0f;
	a_rOutProjection.m_f22 = -( ( a_rProjParams.vecProj.y * 2.0f ) / a_rViewportParams.GetHeight() );
	a_rOutProjection.m_f23 = 0.0f;
	a_rOutProjection.m_f24 = 0.0f;
	a_rOutProjection.m_f31 = ( a_rProjParams.vecCenter.x * 2.0f ) / a_rViewportParams.GetWidth() - 1.0f;
	a_rOutProjection.m_f32 = -( ( a_rProjParams.vecCenter.y * 2.0f ) / a_rViewportParams.GetHeight() - 1.0f );
	a_rOutProjection.m_f33 = a_rProjParams.fFarClip / ( a_rProjParams.fFarClip - a_rProjParams.fNearClip );
	a_rOutProjection.m_f34 = 1.0f;
	a_rOutProjection.m_f41 = 0.0f;
	a_rOutProjection.m_f42 = 0.0f;
	a_rOutProjection.m_f43 = -( ( a_rProjParams.fNearClip * a_rProjParams.fFarClip ) / ( a_rProjParams.fFarClip - a_rProjParams.fNearClip ) );
	a_rOutProjection.m_f44 = 0.0f;
}

void T2RenderContext::ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams )
{
	TASSERT( a_rProjParams.vecProj.x != 0.0f );
	TASSERT( a_rProjParams.vecProj.y != 0.0f );
	TASSERT( a_rViewportParams.GetWidth() != 0.0f );
	TASSERT( a_rViewportParams.GetHeight() != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.x ) && ( !TMath::IsNaN( a_rProjParams.vecProj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.y ) && ( !TMath::IsNaN( a_rProjParams.vecProj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.x ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.y ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.y ) ) );

	a_rOutProjection.m_f11 = ( a_rProjParams.vecProj.x * 2.0f ) / a_rViewportParams.GetWidth();
	a_rOutProjection.m_f12 = 0.0f;
	a_rOutProjection.m_f13 = 0.0f;
	a_rOutProjection.m_f14 = 0.0f;
	a_rOutProjection.m_f21 = 0.0f;
	a_rOutProjection.m_f22 = -( ( a_rProjParams.vecProj.y * 2.0f ) / a_rViewportParams.GetHeight() );
	a_rOutProjection.m_f23 = 0.0f;
	a_rOutProjection.m_f24 = 0.0f;
	a_rOutProjection.m_f31 = 0.0f;
	a_rOutProjection.m_f32 = 0.0f;
	a_rOutProjection.m_f33 = 1.0f / ( a_rProjParams.fFarClip - a_rProjParams.fNearClip );
	a_rOutProjection.m_f34 = 0.0f;
	a_rOutProjection.m_f41 = ( a_rProjParams.vecCenter.x * 2.0f ) / a_rViewportParams.GetWidth() - 1.0f;
	a_rOutProjection.m_f42 = -( ( a_rProjParams.vecCenter.y * 2.0f ) / a_rViewportParams.GetHeight() - 1.0f );
	a_rOutProjection.m_f43 = -( a_rProjParams.fNearClip / ( a_rProjParams.fFarClip - a_rProjParams.fNearClip ) );
	a_rOutProjection.m_f44 = 1.0f;
}

void T2RenderContext::ComputePerspectiveFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams )
{
	TASSERT( a_rProjParams.vecProj.x != 0.0f );
	TASSERT( a_rProjParams.vecProj.y != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.x ) && ( !TMath::IsNaN( a_rProjParams.vecProj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.y ) && ( !TMath::IsNaN( a_rProjParams.vecProj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.x ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.y ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.y ) ) );

	TFLOAT   fVal1 = 1.0f / a_rProjParams.vecProj.x;
	TFLOAT   fVal2 = 1.0f / a_rProjParams.vecProj.y;
	TVector4 vec1  = TVector4( -a_rProjParams.vecCenter.x * fVal1, ( a_rViewportParams.GetHeight() - a_rProjParams.vecCenter.y ) * fVal2, 1.0f, 0.0f );
	TVector4 vec2  = TVector4( ( a_rViewportParams.GetWidth() - a_rProjParams.vecCenter.x ) * fVal1, vec1.y, 1.0f, 0.0f );
	TVector4 vec3  = TVector4( vec2.x, -a_rProjParams.vecCenter.y * fVal2, 1.0f, 0.0f );
	TVector4 vec4  = TVector4( vec1.x, vec3.y, 1.0f, 0.0f );

	// Setup planes
	a_rcFrustum[ FRUSTUMPLANE_LEFT ].AsVector4().CrossProduct( vec4, vec1 );
	a_rcFrustum[ FRUSTUMPLANE_RIGHT ].AsVector4().CrossProduct( vec2, vec3 );
	a_rcFrustum[ FRUSTUMPLANE_BOTTOM ].AsVector4().CrossProduct( vec3, vec4 );
	a_rcFrustum[ FRUSTUMPLANE_TOP ].AsVector4().CrossProduct( vec1, vec2 );

	// Normalize each plane
	a_rcFrustum[ FRUSTUMPLANE_LEFT ].AsNormal().Normalize();
	a_rcFrustum[ FRUSTUMPLANE_RIGHT ].AsNormal().Normalize();
	a_rcFrustum[ FRUSTUMPLANE_BOTTOM ].AsNormal().Normalize();
	a_rcFrustum[ FRUSTUMPLANE_TOP ].AsNormal().Normalize();

	// Setup near and far planes
	a_rcFrustum[ FRUSTUMPLANE_NEAR ].Set( 0.0f, 0.0f, -1.0f, -a_rProjParams.fNearClip );
	a_rcFrustum[ FRUSTUMPLANE_FAR ].Set( 0.0f, 0.0f, 1.0f, a_rProjParams.fFarClip );
}

void T2RenderContext::ComputeOrthographicFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams )
{
	TASSERT( a_rProjParams.vecProj.x != 0.0f );
	TASSERT( a_rProjParams.vecProj.y != 0.0f );
	TASSERT( a_rViewportParams.GetWidth() != 0.0f );
	TASSERT( a_rViewportParams.GetHeight() != 0.0f );

	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.x ) && ( !TMath::IsNaN( a_rProjParams.vecProj.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecProj.y ) && ( !TMath::IsNaN( a_rProjParams.vecProj.y ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.x ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.x ) ) );
	TASSERT( TMath::IsFinite( a_rProjParams.vecCenter.y ) && ( !TMath::IsNaN( a_rProjParams.vecCenter.y ) ) );

	TFLOAT fWidth        = a_rViewportParams.GetWidth();
	TFLOAT fHeight       = a_rViewportParams.GetHeight();
	TFLOAT fCentreX      = a_rProjParams.vecCenter.x;
	TFLOAT fCentreY      = a_rProjParams.vecCenter.y;
	TFLOAT fProjXOverOne = 1.0f / a_rProjParams.vecProj.x;
	TFLOAT fProjYOverOne = 1.0f / a_rProjParams.vecProj.y;

	// Setup planes
	a_rcFrustum[ FRUSTUMPLANE_LEFT ].Set( -1.0f, 0.0f, 0.0f, fCentreX * fProjXOverOne );
	a_rcFrustum[ FRUSTUMPLANE_RIGHT ].Set( 1.0f, 0.0f, 0.0f, ( fWidth - fCentreX ) * fProjXOverOne );
	a_rcFrustum[ FRUSTUMPLANE_BOTTOM ].Set( 0.0f, -1.0f, 0.0f, fCentreY * fProjYOverOne );
	a_rcFrustum[ FRUSTUMPLANE_TOP ].Set( 0.0f, 1.0f, 0.0f, ( fHeight - fCentreY ) * fProjYOverOne );
	a_rcFrustum[ FRUSTUMPLANE_NEAR ].Set( 0.0f, 0.0f, -1.0f, -a_rProjParams.fNearClip );
	a_rcFrustum[ FRUSTUMPLANE_FAR ].Set( 0.0f, 0.0f, 1.0f, a_rProjParams.fFarClip );
}

TBOOL T2RenderContext::CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes )
{
	for ( TSIZE i = 0; i < FRUSTUMPLANE_NUMOF; i++ )
	{
		TFLOAT fDist = TVector4::DotProduct3( a_rSphere.AsVector4(), a_pPlanes[ i ].AsVector4() );

		if ( a_rSphere.GetRadius() < fDist - a_pPlanes[ i ].GetD() )
			return TFALSE;
	}

	return TTRUE;
}

TINT T2RenderContext::CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask )
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

TBOOL T2RenderContext::SetShaderProgram( const T2Shader& a_rcShaderProgram )
{
	if ( m_bForceRefreshFeatures || m_uiCurrentShaderProgram != a_rcShaderProgram.GetProgram() )
	{
		a_rcShaderProgram.Use();
		m_uiCurrentShaderProgram = a_rcShaderProgram.GetProgram();

		return TTRUE;
	}

	return TFALSE;
}

GLuint T2RenderContext::GetTexture2D( TINT a_iTextureIndex )
{
	return m_aCurrentTextures[ a_iTextureIndex ];
}

void T2RenderContext::SetTexture2D( TINT a_iTextureIndex, GLuint a_uiTexture )
{
	if ( m_bForceRefreshFeatures || m_aCurrentTextures[ a_iTextureIndex ] != a_uiTexture )
	{
		if ( m_bForceRefreshFeatures || m_iCurrentTextureUnit != a_iTextureIndex )
		{
			glActiveTexture( GL_TEXTURE0 + a_iTextureIndex );
			m_iCurrentTextureUnit = a_iTextureIndex;
		}

		glBindTexture( GL_TEXTURE_2D, a_uiTexture );
		m_aCurrentTextures[ a_iTextureIndex ] = a_uiTexture;
	}
}

void T2RenderContext::SetTexture2D( TINT a_iTextureIndex, const T2GLTexture& a_rcTexture )
{
	SetTexture2D( a_iTextureIndex, a_rcTexture.GetHandle() );
}

void T2RenderContext::ResetTexture2D( TINT a_iTextureIndex )
{
	if ( m_bForceRefreshFeatures || m_iCurrentTextureUnit != a_iTextureIndex )
	{
		glActiveTexture( GL_TEXTURE0 + a_iTextureIndex );
		glBindTexture( GL_TEXTURE_2D, NULL );

		m_iCurrentTextureUnit                 = a_iTextureIndex;
		m_aCurrentTextures[ a_iTextureIndex ] = -1;
	}
}

void T2RenderContext::EnableDepthTest( TBOOL a_bEnable )
{
	if ( m_bForceRefreshFeatures || m_bDepthTest != a_bEnable )
	{
		if ( a_bEnable )
			glEnable( GL_DEPTH_TEST );
		else
			glDisable( GL_DEPTH_TEST );

		m_bDepthTest = a_bEnable;
	}
}

void T2RenderContext::EnableBlend( TBOOL a_bEnable )
{
	if ( m_bForceRefreshFeatures || m_bBlend != a_bEnable )
	{
		if ( a_bEnable )
			glEnable( GL_BLEND );
		else
			glDisable( GL_BLEND );

		m_bBlend = a_bEnable;
	}
}

void T2RenderContext::SetViewMatrix( const TMatrix44& a_rMatrix )
{
	m_matViewMatrix = a_rMatrix;
}

void T2RenderContext::ForceRefreshFeatures()
{
	m_bForceRefreshFeatures = TTRUE;

	SetShaderProgram( m_uiCurrentShaderProgram );
	EnableBlend( m_bBlend );
	EnableDepthTest( m_bDepthTest );

	ResetTexture2D( 0 );
	ResetTexture2D( 1 );
	ResetTexture2D( 2 );
	ResetTexture2D( 3 );
	ResetTexture2D( 4 );
	ResetTexture2D( 5 );
	ResetTexture2D( 6 );
	ResetTexture2D( 7 );

	m_bForceRefreshFeatures = TFALSE;
}

void T2RenderContext::UpdateCamera()
{
	TVALIDPTR( m_pCamera );

	m_oProjectionParams.vecCenter.x = m_oViewport.GetWidth() * 0.5f;
	m_oProjectionParams.vecCenter.y = m_oViewport.GetHeight() * 0.5f;
	m_oProjectionParams.vecProj.x   = m_oProjectionParams.vecCenter.x / TMath::Tan( m_pCamera->GetFOV() * 0.5f );
	m_oProjectionParams.vecProj.y   = m_oProjectionParams.vecProj.x;
	m_oProjectionParams.fNearClip   = m_pCamera->GetNearPlane();
	m_oProjectionParams.fFarClip    = m_pCamera->GetFarPlane();

	switch ( m_pCamera->GetCameraMode() )
	{
		case CM_PERSPECTIVE:
			ComputePerspectiveProjection( m_matProjection, m_oViewport, m_oProjectionParams );
			break;

		case CM_ORTHOGRAPHIC:
			ComputeOrthographicProjection( m_matProjection, m_oViewport, m_oProjectionParams );
			break;
	}

	m_matViewMatrix = m_pCamera->GetViewMatrix();
}

void T2RenderContext::SetModelMatrix( const TMatrix44& a_rMatrix )
{
	m_matModelMatrix = a_rMatrix;
}

void T2RenderContext::Projection::SetFromFOV( TFLOAT a_fViewportWidth, TFLOAT a_fViewportHeight, TFLOAT a_fFOV, TFLOAT a_fNearPlane, TFLOAT a_fFarPlane )
{
	TFLOAT fHalfWidth  = a_fViewportWidth * 0.5f;
	TFLOAT fHalfHeight = a_fViewportHeight * 0.5f;
	TFLOAT fProj       = fHalfHeight / TMath::Tan( a_fFOV );

	fNearClip   = a_fNearPlane;
	fFarClip    = a_fFarPlane;
	vecCenter.x = fHalfWidth;
	vecCenter.y = fHalfHeight;
	vecProj.x   = fProj;
	vecProj.y   = fProj;
}

TOSHI_NAMESPACE_END
