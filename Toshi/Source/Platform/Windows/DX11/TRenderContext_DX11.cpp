#include "ToshiPCH.h"
#include "TRenderContext_DX11.h"
#include <DirectXMath.h>

namespace Toshi
{
    void TRenderContextDX11::ComputePerspectiveProjection()
    {
        float v1; // xmm0_4
        float v2; // xmm0_4
        float v3; // xmm0_4
        float v4; // xmm0_4
        float v5; // xmm2_4

        v1 = (m_sProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
        m_Projection.e = 0.0f;
        m_Projection.a = v1;
        v2 = (m_sProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth;
        m_Projection.m = 0.0f;
        m_Projection.b = 0.0f;
        m_Projection.i = v2 - 1.0f;
        m_Projection.f = -((m_sProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight);
        v3 = (m_sProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight;
        m_Projection.n = 0.0f;
        m_Projection.c = 0.0f;
        m_Projection.g = 0.0f;
        m_Projection.j = -(v3 - 1.0f);
        m_Projection.k = m_sProjParams.m_fFarClip / (m_sProjParams.m_fFarClip - m_sProjParams.m_fNearClip);
        v4 = m_sProjParams.m_fNearClip;
        v5 = m_sProjParams.m_fFarClip;
        m_Projection.d = 0.0f;
        m_Projection.h = 0.0f;
        m_Projection.l = 1.0f;
        m_Projection.p = 0.0f;
        m_Projection.o = -((v4 * v5) / (v5 - v4));
    }

    void TRenderContextDX11::ComputePerspectiveFrustum()
    {
        TIMPLEMENT();
    }

    void TRenderContextDX11::ComputeOrthographicProjection()
    {
        float v2; // xmm0_4
        float v3; // xmm0_4
        float v4; // xmm0_4
        float v5; // xmm0_4
        float v6; // xmm1_4
        float v7; // xmm0_4

        v2 = (m_sProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
        m_Projection.e = 0.0f;
        m_Projection.i = 0.0f;
        m_Projection.a = v2;
        v3 = (m_sProjParams.m_Centre.x * 2.0f) / m_oParams.fWidth;
        m_Projection.b = 0.0f;
        m_Projection.m = v3 - 1.0f;
        v4 = (m_sProjParams.m_Proj.y * 2.0f) / m_oParams.fHeight;
        m_Projection.j = 0.0f;
        m_Projection.f = -v4;
        v5 = (m_sProjParams.m_Centre.y * 2.0f) / m_oParams.fHeight;
        m_Projection.c = 0.0f;
        m_Projection.g = 0.0f;
        m_Projection.n = -(v5 - 1.0f);
        m_Projection.k = 1.0f / (m_sProjParams.m_fFarClip - m_sProjParams.m_fNearClip);
        v6 = m_sProjParams.m_fNearClip;
        v7 = m_sProjParams.m_fFarClip - v6;
        m_Projection.d = 0.0f;
        m_Projection.h = 0.0f;
        m_Projection.l = 0.0f;
        m_Projection.p = 1.0f;
        m_Projection.o = -(v6 / v7);
    }

    void TRenderContextDX11::ComputeOrthographicFrustum()
    {
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

            TRender::GetSingletonWeak()->GetParamTable()->SetParameterM44(
                TRenderParamTable::M44PARAM_PROJ,
                m_Projection
            );
        }
    }
}