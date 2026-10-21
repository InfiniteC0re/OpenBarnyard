#include "ToshiPCH.h"
#include "TRenderInterface_DX11.h"
#include "TRenderContext_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TRenderContextD3D::ComputePerspectiveProjection()
{
	TRenderContext::ComputePerspectiveProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

void TRenderContextD3D::ComputeOrthographicProjection()
{
	TRenderContext::ComputeOrthographicProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

void TRenderContextD3D::ComputePerspectiveFrustum()
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

void TRenderContextD3D::ComputeOrthographicFrustum()
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

void TRenderContextD3D::EnableFogHAL()
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, m_pRenderer );

	pRenderer->SetFogEnable( TTRUE );
	pRenderer->SetFogColour( ( ( ( TUINT8( m_FogColor.x * 255.0f ) << 8 ) | TUINT8( m_FogColor.y * 255.0f ) ) << 8 ) | TUINT8( m_FogColor.z * 255.0f ) );
	pRenderer->SetFogStart( m_fFogDistanceStart );
	pRenderer->SetFogEnd( m_fFogDistanceEnd );
}

void TRenderContextD3D::DisableFogHAL()
{
	TSTATICCAST( TRenderD3DInterface, m_pRenderer )->SetFogEnable( TFALSE );
}

TRenderContextD3D::TRenderContextD3D( TRenderInterface* a_pRenderer )
    : TRenderContext( a_pRenderer )
{
}

TRenderContextD3D::~TRenderContextD3D()
{
}

void TRenderContextD3D::Update()
{
	if ( IsDirty() )
	{
		if ( m_eCameraMode == CameraMode_Perspective )
		{
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
