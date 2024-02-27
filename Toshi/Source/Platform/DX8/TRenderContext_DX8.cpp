#include "ToshiPCH.h"
#include "TRenderInterface_DX8.h"
#include "TRenderContext_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	void TRenderContextD3D::ComputePerspectiveProjection()
	{
		m_Projection.m_f11 = (m_ProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
		m_Projection.m_f12 = 0.0f;
		m_Projection.m_f13 = 0.0f;
		m_Projection.m_f14 = 0.0f;
		m_Projection.m_f21 = 0.0f;
		m_Projection.m_f22 = -((m_ProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight);
		m_Projection.m_f23 = 0.0f;
		m_Projection.m_f24 = 0.0f;
		m_Projection.m_f31 = (m_ProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth - 1.0f;
		m_Projection.m_f32 = -((m_ProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight - 1.0f);
		m_Projection.m_f33 = m_ProjParams.m_fFarClip / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip);
		m_Projection.m_f34 = 1.0f;
		m_Projection.m_f41 = 0.0f;
		m_Projection.m_f42 = 0.0f;
		m_Projection.m_f43 = -((m_ProjParams.m_fNearClip * m_ProjParams.m_fFarClip) / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip));
		m_Projection.m_f44 = 0.0f;
	}

	void TRenderContextD3D::ComputePerspectiveFrustum()
	{
		TASSERT(m_ProjParams.m_Proj.x != 0.0f);
		TASSERT(m_ProjParams.m_Proj.y != 0.0f);

		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.x) && (!TMath::IsNaN(m_ProjParams.m_Proj.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.y) && (!TMath::IsNaN(m_ProjParams.m_Proj.y)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.x) && (!TMath::IsNaN(m_ProjParams.m_Centre.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.y) && (!TMath::IsNaN(m_ProjParams.m_Centre.y)));

		float fVal1 = 1.0f / m_ProjParams.m_Proj.x;
		float fVal2 = 1.0f / m_ProjParams.m_Proj.y;
		TVector4 vec1 = TVector4(-m_ProjParams.m_Centre.x * fVal1, (m_oParams.fHeight - m_ProjParams.m_Centre.y) * fVal2, 1.0f, 0.0f);
		TVector4 vec2 = TVector4((m_oParams.fWidth - m_ProjParams.m_Centre.x) * fVal1, vec1.y, 1.0f, 0.0f);
		TVector4 vec3 = TVector4(vec2.x, -m_ProjParams.m_Centre.y * fVal2, 1.0f, 0.0f);
		TVector4 vec4 = TVector4(vec1.x, vec3.y, 1.0f, 0.0f);

		// Setup planes
		m_aFrustumPlanes1[WORLDPLANE_LEFT].AsVector4().CrossProduct(vec4, vec1);
		m_aFrustumPlanes1[WORLDPLANE_RIGHT].AsVector4().CrossProduct(vec2, vec3);
		m_aFrustumPlanes1[WORLDPLANE_BOTTOM].AsVector4().CrossProduct(vec3, vec4);
		m_aFrustumPlanes1[WORLDPLANE_TOP].AsVector4().CrossProduct(vec1, vec2);

		// Normalize each plane
		m_aFrustumPlanes1[WORLDPLANE_LEFT].AsNormal().Normalize();
		m_aFrustumPlanes1[WORLDPLANE_RIGHT].AsNormal().Normalize();
		m_aFrustumPlanes1[WORLDPLANE_BOTTOM].AsNormal().Normalize();
		m_aFrustumPlanes1[WORLDPLANE_TOP].AsNormal().Normalize();

		// Setup near and far planes
		m_aFrustumPlanes1[WORLDPLANE_NEAR].Set(0.0f, 0.0f, -1.0f, -m_ProjParams.m_fNearClip);
		m_aFrustumPlanes1[WORLDPLANE_FAR].Set(0.0f, 0.0f, 1.0f, m_ProjParams.m_fFarClip);

		// Copy planes
		m_aFrustumPlanes2[WORLDPLANE_LEFT] = m_aFrustumPlanes1[WORLDPLANE_LEFT];
		m_aFrustumPlanes2[WORLDPLANE_RIGHT] = m_aFrustumPlanes1[WORLDPLANE_RIGHT];
		m_aFrustumPlanes2[WORLDPLANE_BOTTOM] = m_aFrustumPlanes1[WORLDPLANE_BOTTOM];
		m_aFrustumPlanes2[WORLDPLANE_TOP] = m_aFrustumPlanes1[WORLDPLANE_TOP];
		m_aFrustumPlanes2[WORLDPLANE_NEAR] = m_aFrustumPlanes1[WORLDPLANE_NEAR];
		m_aFrustumPlanes2[WORLDPLANE_FAR] = m_aFrustumPlanes1[WORLDPLANE_FAR];
	}

	void TRenderContextD3D::ComputeOrthographicProjection()
	{
		TASSERT(m_ProjParams.m_Proj.x != 0.0f);
		TASSERT(m_ProjParams.m_Proj.y != 0.0f);
		TASSERT(m_oParams.fWidth != 0.0f);
		TASSERT(m_oParams.fHeight != 0.0f);

		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.x) && (!TMath::IsNaN(m_ProjParams.m_Proj.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.y) && (!TMath::IsNaN(m_ProjParams.m_Proj.y)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.x) && (!TMath::IsNaN(m_ProjParams.m_Centre.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.y) && (!TMath::IsNaN(m_ProjParams.m_Centre.y)));

		m_Projection.m_f11 = (m_ProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
		m_Projection.m_f12 = 0.0f;
		m_Projection.m_f13 = 0.0f;
		m_Projection.m_f14 = 0.0f;
		m_Projection.m_f21 = 0.0f;
		m_Projection.m_f22 = -((m_ProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight);
		m_Projection.m_f23 = 0.0f;
		m_Projection.m_f24 = 0.0f;
		m_Projection.m_f31 = 0.0f;
		m_Projection.m_f32 = 0.0f;
		m_Projection.m_f33 = 1.0f / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip);
		m_Projection.m_f34 = 0.0f;
		m_Projection.m_f41 = (m_ProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth - 1.0f;
		m_Projection.m_f42 = -((m_ProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight - 1.0f);
		m_Projection.m_f43 = -(m_ProjParams.m_fNearClip / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip));
		m_Projection.m_f44 = 1.0f;
	}

	void TRenderContextD3D::ComputeOrthographicFrustum()
	{
		TASSERT(m_ProjParams.m_Proj.x != 0.0f);
		TASSERT(m_ProjParams.m_Proj.y != 0.0f);
		TASSERT(m_oParams.fWidth != 0.0f);
		TASSERT(m_oParams.fHeight != 0.0f);

		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.x) && (!TMath::IsNaN(m_ProjParams.m_Proj.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.y) && (!TMath::IsNaN(m_ProjParams.m_Proj.y)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.x) && (!TMath::IsNaN(m_ProjParams.m_Centre.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.y) && (!TMath::IsNaN(m_ProjParams.m_Centre.y)));

		float fWidth = m_oParams.fWidth;
		float fHeight = m_oParams.fHeight;
		float fCentreX = m_ProjParams.m_Centre.x;
		float fCentreY = m_ProjParams.m_Centre.y;
		float fVal1 = 1.0f / m_ProjParams.m_Proj.x;
		float fVal2 = 1.0f / m_ProjParams.m_Proj.y;

		// Setup planes
		m_aFrustumPlanes1[WORLDPLANE_LEFT].Set(-1.0f, 0.0f, 0.0f, fCentreX * fVal1);
		m_aFrustumPlanes1[WORLDPLANE_RIGHT].Set(1.0f, 0.0f, 0.0f, (fWidth - fCentreX) * fVal1);
		m_aFrustumPlanes1[WORLDPLANE_BOTTOM].Set(0.0f, -1.0f, 0.0f, fCentreY * fVal2);
		m_aFrustumPlanes1[WORLDPLANE_TOP].Set(0.0f, 1.0f, 0.0f, (fHeight - fCentreY) * fVal2);
		m_aFrustumPlanes1[WORLDPLANE_NEAR].Set(0.0f, 0.0f, -1.0f, -m_ProjParams.m_fNearClip);
		m_aFrustumPlanes1[WORLDPLANE_FAR].Set(0.0f, 0.0f, 1.0f, m_ProjParams.m_fFarClip);

		// Copy planes
		m_aFrustumPlanes2[WORLDPLANE_LEFT] = m_aFrustumPlanes1[WORLDPLANE_LEFT];
		m_aFrustumPlanes2[WORLDPLANE_RIGHT] = m_aFrustumPlanes1[WORLDPLANE_RIGHT];
		m_aFrustumPlanes2[WORLDPLANE_BOTTOM] = m_aFrustumPlanes1[WORLDPLANE_BOTTOM];
		m_aFrustumPlanes2[WORLDPLANE_TOP] = m_aFrustumPlanes1[WORLDPLANE_TOP];
		m_aFrustumPlanes2[WORLDPLANE_NEAR] = m_aFrustumPlanes1[WORLDPLANE_NEAR];
		m_aFrustumPlanes2[WORLDPLANE_FAR] = m_aFrustumPlanes1[WORLDPLANE_FAR];
	}

	void TRenderContextD3D::EnableFogHAL()
	{
		auto pRenderer = TSTATICCAST(TRenderD3DInterface*, m_pRenderer);
		auto pDevice = pRenderer->GetDirect3DDevice();

		pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
		pDevice->SetRenderState(
			D3DRS_FOGCOLOR,
			(((TUINT8(m_FogColor.x * 255.0f) << 8) | TUINT8(m_FogColor.y * 255.0f)) << 8) | TUINT8(m_FogColor.z * 255.0f)
		);

		pDevice->SetRenderState(D3DRS_FOGSTART, *TREINTERPRETCAST(DWORD*, &m_fFogDistanceStart));
		pDevice->SetRenderState(D3DRS_FOGEND, *TREINTERPRETCAST(DWORD*, &m_fFogDistanceEnd));
	}

	void TRenderContextD3D::DisableFogHAL()
	{
		auto pRenderer = TSTATICCAST(TRenderD3DInterface*, m_pRenderer);
		auto pDevice = pRenderer->GetDirect3DDevice();

		pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	}

	void TRenderContextD3D::Update()
	{
		if (IsDirty())
		{
			if (m_eCameraMode == CameraMode_Perspective)
			{
				ComputePerspectiveProjection();
				ComputePerspectiveFrustum();
			}
			else
			{
				ComputeOrthographicProjection();
				ComputeOrthographicFrustum();
			}

			auto pRenderer = TSTATICCAST(TRenderD3DInterface*, m_pRenderer);
			pRenderer->GetDirect3DDevice()->SetTransform(D3DTS_VIEW, TREINTERPRETCAST(D3DMATRIX*, &TMatrix44::IDENTITY));
			pRenderer->GetDirect3DDevice()->SetTransform(D3DTS_PROJECTION, TREINTERPRETCAST(D3DMATRIX*, &m_Projection));
		}
	}

}
