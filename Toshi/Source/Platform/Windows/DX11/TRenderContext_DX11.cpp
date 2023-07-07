#include "ToshiPCH.h"
#include "TRenderContext_DX11.h"
#include <DirectXMath.h>

namespace Toshi
{
    void TRenderContextDX11::ComputePerspectiveProjection()
    {
        //m_Projection = DirectX::XMMatrixPerspectiveFovLH(90, 16.0f / 9.0f, 0.1f, 100.0f);
        float Zf;
        float Zn;

        Zn = m_ProjParams.m_Proj.x;
        Zf = m_oParams.fWidth;
        m_Projection.e = 0.0f;
        m_Projection.a = (Zn * 2.0f) / Zf;
        Zn = m_ProjParams.m_Centre.x;
        Zf = m_oParams.fWidth;
        m_Projection.m = 0.0f;
        m_Projection.b = 0.0f;
        m_Projection.i = (Zn * 2.0f) / Zf - 1.0f;
        m_Projection.f = -(m_ProjParams.m_Proj.y * 2.0f / m_oParams.fHeight);
        Zn = m_ProjParams.m_Centre.y;
        Zf = m_oParams.fHeight;
        m_Projection.n = 0.0f;
        m_Projection.c = 0.0f;
        m_Projection.g = 0.0f;
        m_Projection.j = -((Zn * 2.0f) / Zf - 1.0f);
        Zn = m_ProjParams.m_fFarClip;
        m_Projection.k = Zn / (Zn - m_ProjParams.m_fNearClip);
        Zn = m_ProjParams.m_fNearClip;
        Zf = m_ProjParams.m_fFarClip;
        m_Projection.d = 0.0f;
        m_Projection.h = 0.0f;
        m_Projection.l = 1.0f;
        m_Projection.p = 0.0f;
        m_Projection.o = -((Zn * Zf) / (Zf - Zn));
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

        float v2; // xmm0_4
        float v3; // xmm0_4
        float v4; // xmm0_4
        float v5; // xmm0_4
        float v6; // xmm1_4
        float v7; // xmm0_4

        v2 = (m_ProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
        m_Projection.e = 0.0f;
        m_Projection.i = 0.0f;
        m_Projection.a = v2;
        v3 = (m_ProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth;
        m_Projection.b = 0.0f;
        m_Projection.m = v3 - 1.0f;
        v4 = (m_ProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight;
        m_Projection.j = 0.0f;
        m_Projection.f = -v4;
        v5 = (m_ProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight;
        m_Projection.c = 0.0f;
        m_Projection.g = 0.0f;
        m_Projection.n = -(v5 - 1.0f);
        m_Projection.k = 1.0f / (m_ProjParams.m_fFarClip - m_ProjParams.m_fNearClip);
        v6 = m_ProjParams.m_fNearClip;
        v7 = m_ProjParams.m_fFarClip - v6;
        m_Projection.d = 0.0f;
        m_Projection.h = 0.0f;
        m_Projection.l = 0.0f;
        m_Projection.p = 1.0f;
        m_Projection.o = -(v6 / v7);
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