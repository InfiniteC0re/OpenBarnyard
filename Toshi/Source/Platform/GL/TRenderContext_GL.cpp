#include "ToshiPCH.h"
#include "TRenderInterface_GL.h"
#include "TRenderContext_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TRenderContextGL::ComputePerspectiveProjection()
{
	TRenderContext::ComputePerspectiveProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

void TRenderContextGL::ComputeOrthographicProjection()
{
	TRenderContext::ComputeOrthographicProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

void TRenderContextGL::ComputePerspectiveFrustum()
{
	TRenderContext::ComputePerspectiveFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	// Copy planes
	m_aGuardPlanes[ WORLDPLANE_LEFT ]   = m_aFrustumPlanes1[ WORLDPLANE_LEFT ];
	m_aGuardPlanes[ WORLDPLANE_RIGHT ]  = m_aFrustumPlanes1[ WORLDPLANE_RIGHT ];
	m_aGuardPlanes[ WORLDPLANE_BOTTOM ] = m_aFrustumPlanes1[ WORLDPLANE_BOTTOM ];
	m_aGuardPlanes[ WORLDPLANE_TOP ]    = m_aFrustumPlanes1[ WORLDPLANE_TOP ];
	m_aGuardPlanes[ WORLDPLANE_NEAR ]   = m_aFrustumPlanes1[ WORLDPLANE_NEAR ];
	m_aGuardPlanes[ WORLDPLANE_FAR ]    = m_aFrustumPlanes1[ WORLDPLANE_FAR ];
}

void TRenderContextGL::ComputeOrthographicFrustum()
{
	TRenderContext::ComputeOrthographicFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	// Copy planes
	m_aGuardPlanes[ WORLDPLANE_LEFT ]   = m_aFrustumPlanes1[ WORLDPLANE_LEFT ];
	m_aGuardPlanes[ WORLDPLANE_RIGHT ]  = m_aFrustumPlanes1[ WORLDPLANE_RIGHT ];
	m_aGuardPlanes[ WORLDPLANE_BOTTOM ] = m_aFrustumPlanes1[ WORLDPLANE_BOTTOM ];
	m_aGuardPlanes[ WORLDPLANE_TOP ]    = m_aFrustumPlanes1[ WORLDPLANE_TOP ];
	m_aGuardPlanes[ WORLDPLANE_NEAR ]   = m_aFrustumPlanes1[ WORLDPLANE_NEAR ];
	m_aGuardPlanes[ WORLDPLANE_FAR ]    = m_aFrustumPlanes1[ WORLDPLANE_FAR ];
}

void TRenderContextGL::EnableFogHAL()
{
	auto pRenderer = TSTATICCAST( TRenderGLInterface, m_pRenderer );

	pRenderer->SetFogEnable( TTRUE );
	pRenderer->SetFogColour( ( ( ( TUINT8( m_FogColor.x * 255.0f ) << 8 ) | TUINT8( m_FogColor.y * 255.0f ) ) << 8 ) | TUINT8( m_FogColor.z * 255.0f ) );
	pRenderer->SetFogStart( m_fFogDistanceStart );
	pRenderer->SetFogEnd( m_fFogDistanceEnd );
}

void TRenderContextGL::DisableFogHAL()
{
	TSTATICCAST( TRenderGLInterface, m_pRenderer )->SetFogEnable( TFALSE );
}

TRenderContextGL::TRenderContextGL( TRenderInterface* a_pRenderer )
    : TRenderContext( a_pRenderer )
{
}

TRenderContextGL::~TRenderContextGL()
{
}

void TRenderContextGL::Update()
{
	if ( IsDirty() )
	{
		if ( m_eCameraMode == CameraMode_Perspective )
		{
			if ( ms_fPerspectiveFarClip > 0.0f )
				m_oProjParams.m_fFarClip = ms_fPerspectiveFarClip;

			ComputePerspectiveProjection();
			ComputePerspectiveFrustum();
		}
		else
		{
			ComputeOrthographicProjection();
			ComputeOrthographicFrustum();
		}
	}
}

TOSHI_NAMESPACE_END
