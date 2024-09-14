#include "ToshiPCH.h"
#include "TRenderInterface_DX8.h"
#include "TRenderContext_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006d6c80
void TRenderContextD3D::ComputePerspectiveProjection()
{
	TRenderContext::ComputePerspectiveProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

// $Barnyard: FUNCTION 006d6f80
void TRenderContextD3D::ComputeOrthographicProjection()
{
	TRenderContext::ComputeOrthographicProjection( m_Projection, m_oViewportParams, m_oProjParams );
}

// $Barnyard: FUNCTION 006d6d30
void TRenderContextD3D::ComputePerspectiveFrustum()
{
	TRenderContext::ComputePerspectiveFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	// Copy planes
	m_aFrustumPlanes2[ WORLDPLANE_LEFT ]   = m_aFrustumPlanes1[ WORLDPLANE_LEFT ];
	m_aFrustumPlanes2[ WORLDPLANE_RIGHT ]  = m_aFrustumPlanes1[ WORLDPLANE_RIGHT ];
	m_aFrustumPlanes2[ WORLDPLANE_BOTTOM ] = m_aFrustumPlanes1[ WORLDPLANE_BOTTOM ];
	m_aFrustumPlanes2[ WORLDPLANE_TOP ]    = m_aFrustumPlanes1[ WORLDPLANE_TOP ];
	m_aFrustumPlanes2[ WORLDPLANE_NEAR ]   = m_aFrustumPlanes1[ WORLDPLANE_NEAR ];
	m_aFrustumPlanes2[ WORLDPLANE_FAR ]    = m_aFrustumPlanes1[ WORLDPLANE_FAR ];
}

// $Barnyard: FUNCTION 006d7030
void TRenderContextD3D::ComputeOrthographicFrustum()
{
	TRenderContext::ComputeOrthographicFrustum( m_aFrustumPlanes1, m_oViewportParams, m_oProjParams );

	// Copy planes
	m_aFrustumPlanes2[ WORLDPLANE_LEFT ]   = m_aFrustumPlanes1[ WORLDPLANE_LEFT ];
	m_aFrustumPlanes2[ WORLDPLANE_RIGHT ]  = m_aFrustumPlanes1[ WORLDPLANE_RIGHT ];
	m_aFrustumPlanes2[ WORLDPLANE_BOTTOM ] = m_aFrustumPlanes1[ WORLDPLANE_BOTTOM ];
	m_aFrustumPlanes2[ WORLDPLANE_TOP ]    = m_aFrustumPlanes1[ WORLDPLANE_TOP ];
	m_aFrustumPlanes2[ WORLDPLANE_NEAR ]   = m_aFrustumPlanes1[ WORLDPLANE_NEAR ];
	m_aFrustumPlanes2[ WORLDPLANE_FAR ]    = m_aFrustumPlanes1[ WORLDPLANE_FAR ];
}

// $Barnyard: FUNCTION 006d7220
void TRenderContextD3D::EnableFogHAL()
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, m_pRenderer );
	auto pDevice   = pRenderer->GetDirect3DDevice();

	pDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
	pDevice->SetRenderState(
	    D3DRS_FOGCOLOR,
	    ( ( ( TUINT8( m_FogColor.x * 255.0f ) << 8 ) | TUINT8( m_FogColor.y * 255.0f ) ) << 8 ) | TUINT8( m_FogColor.z * 255.0f ) );

	pDevice->SetRenderState( D3DRS_FOGSTART, *TREINTERPRETCAST( DWORD*, &m_fFogDistanceStart ) );
	pDevice->SetRenderState( D3DRS_FOGEND, *TREINTERPRETCAST( DWORD*, &m_fFogDistanceEnd ) );
}

void TRenderContextD3D::DisableFogHAL()
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, m_pRenderer );
	auto pDevice   = pRenderer->GetDirect3DDevice();

	pDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
}

// $Barnyard: FUNCTION 006d6c60
TRenderContextD3D::TRenderContextD3D( TRenderInterface* a_pRenderer ) :
    TRenderContext( a_pRenderer )
{
}

// $Barnyard: FUNCTION 006d72e0
TRenderContextD3D::~TRenderContextD3D()
{
}

// $Barnyard: FUNCTION 006d7310
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

		auto pRenderer = TSTATICCAST( TRenderD3DInterface, m_pRenderer );
		pRenderer->GetDirect3DDevice()->SetTransform( D3DTS_VIEW, TREINTERPRETCAST( D3DMATRIX*, &TMatrix44::IDENTITY ) );
		pRenderer->GetDirect3DDevice()->SetTransform( D3DTS_PROJECTION, TREINTERPRETCAST( D3DMATRIX*, &m_Projection ) );
	}
}

TOSHI_NAMESPACE_END
