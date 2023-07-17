#include "ToshiPCH.h"
#include "TRenderContext_DX11.h"
#include <DirectXMath.h>

namespace Toshi
{
	void TRenderContextDX11::ComputePerspectiveProjection()
	{
		//m_Projection = DirectX::XMMatrixPerspectiveFovLH(90, 16.0f / 9.0f, 0.1f, 100.0f);
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

	void TRenderContextDX11::ComputePerspectiveFrustum()
	{
		TIMPLEMENT();
	}

	void TRenderContextDX11::ComputeOrthographicProjection()
	{
		TASSERT(m_ProjParams.m_Proj.x != 0.0f);
		TASSERT(m_ProjParams.m_Proj.y != 0.0f);
		TASSERT(m_oParams.fWidth != 0.0f);
		TASSERT(m_oParams.fHeight != 0.0f);

		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.x) && (!TMath::IsNaN(m_ProjParams.m_Proj.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.y) && (!TMath::IsNaN(m_ProjParams.m_Proj.y)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.x) && (!TMath::IsNaN(m_ProjParams.m_Centre.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.y) && (!TMath::IsNaN(m_ProjParams.m_Centre.y)));
	
		m_Projection.m_f21 = 0.0f;
		m_Projection.m_f31 = 0.0f;
		m_Projection.m_f11 = (m_ProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
		m_Projection.m_f12 = 0.0f;
		m_Projection.m_f41 = (m_ProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth - 1.0f;
		m_Projection.m_f32 = 0.0f;
		m_Projection.m_f22 = -((m_ProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight);
		m_Projection.m_f13 = 0.0f;
		m_Projection.m_f23 = 0.0f;
		m_Projection.m_f42 = -((m_ProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight - 1.0f);
		m_Projection.m_f33 = 1.0f / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip);
		m_Projection.m_f14 = 0.0f;
		m_Projection.m_f24 = 0.0f;
		m_Projection.m_f34 = 0.0f;
		m_Projection.m_f44 = 1.0f;
		m_Projection.m_f43 = -(m_ProjParams.m_fNearClip / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip));
	}

	void TRenderContextDX11::ComputeOrthographicFrustum()
	{
		TASSERT(m_ProjParams.m_Proj.x != 0.0f);
		TASSERT(m_ProjParams.m_Proj.y != 0.0f);
		TASSERT(m_oParams.fWidth != 0.0f);
		TASSERT(m_oParams.fHeight != 0.0f);

		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.x) && (!TMath::IsNaN(m_ProjParams.m_Proj.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Proj.y) && (!TMath::IsNaN(m_ProjParams.m_Proj.y)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.x) && (!TMath::IsNaN(m_ProjParams.m_Centre.x)));
		TASSERT(TMath::IsFinite(m_ProjParams.m_Centre.y) && (!TMath::IsNaN(m_ProjParams.m_Centre.y)));

		TIMPLEMENT();
	}

	void TRenderContextDX11::Update()
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

			TRender::GetSingleton()->GetParamTable()->SetParameterM44(
				TRenderParamTable::M44PARAM_PROJ,
				m_Projection
			);
		}
	}
}