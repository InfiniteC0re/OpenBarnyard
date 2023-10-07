#include "ToshiPCH.h"
#include "TRenderContext.h"
#include "TRenderInterface.h"

namespace Toshi {

	TRenderContext::TRenderContext(TRenderInterface* pRender)
	{
		TIMPLEMENT();
		m_eFlags = 0;
		m_pRenderer = pRender;

		m_oParams.fX = 0;
		m_oParams.fY = 0;
		m_oParams.fMaxZ = 1.0f;
		m_oParams.fMinZ = 1.0f;

		m_eCameraMode = CameraMode_Perspective;
		m_ProjParams.m_fNearClip = 1.0f;
		m_ProjParams.m_fFarClip = 1000.0f;
		m_oModelViewMatrix.Identity();
		m_oWorldViewMatrix.Identity();

		auto pDevice = pRender->GetCurrentDevice();

		if (pDevice == TNULL)
		{
			m_oParams.fWidth = 640.0f;
			m_oParams.fHeight = 480.0f;
		}
		else
		{
			TASSERT(TFALSE, "Not used in De blob");
		}
	}

	void TRenderContext::SetModelViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= (FLAG_DIRTY_WORLDMODELMATRIX | FLAG_DIRTY_VIEWMODELMATRIX);
		m_oModelViewMatrix = a_rMatrix;
		m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_UNK3);
	}

	void TRenderContext::SetWorldViewMatrix(const TMatrix44& a_rMatrix)
	{
		m_eFlags |= FLAG_DIRTY_VIEWMODELMATRIX;
		m_oWorldViewMatrix = a_rMatrix;
		m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_HAS_VIEWWORLDMATRIX | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6);
	}

	void TRenderContext::SetProjectionParams(const PROJECTIONPARAMS& params)
	{
		TASSERT(params.m_Proj.x != 0.0f);
		TASSERT(params.m_Proj.y != 0.0f);
		TASSERT(TMath::IsFinite(params.m_Proj.x) && (!TMath::IsNaN(params.m_Proj.x)));
		TASSERT(TMath::IsFinite(params.m_Proj.y) && (!TMath::IsNaN(params.m_Proj.y)));
		TASSERT(TMath::IsFinite(params.m_Centre.x) && (!TMath::IsNaN(params.m_Centre.x)));
		TASSERT(TMath::IsFinite(params.m_Centre.y) && (!TMath::IsNaN(params.m_Centre.y)));

		m_ProjParams = params;
		m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6))) | FLAG_DIRTY;
	}

	const TPlane* TRenderContext::GetWorldPlanes()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_WORLDPLANES))
		{
			auto& viewWorld = GetViewWorldMatrix();

			for (size_t i = 0; i < 6; i++)
			{
				TMatrix44::TransformPlaneOrthogonal(m_aWorldPlanes[i], viewWorld, m_aFrustumPlanes1[i]);
			}

			m_eFlags |= FLAG_HAS_WORLDPLANES;
		}

		return m_aWorldPlanes;
	}

	TBOOL TRenderContext::CullSphereToFrustumSimple(const TSphere& a_rSphere, const TPlane* a_pPlanes, int a_iUnused)
	{
		for (size_t i = 0; i < 6; i++)
		{
			TFLOAT fDist = TVector4::DotProduct3(a_rSphere.AsVector4(), a_pPlanes[i].AsVector4());

			if (a_rSphere.GetRadius() < fDist - a_pPlanes[i].GetD())
				return TFALSE;
		}

		return TTRUE;
	}

	const TMatrix44& TRenderContext::GetViewWorldMatrix()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_VIEWWORLDMATRIX))
		{
			m_oViewWorldMatrix.InvertOrthogonal(m_oWorldViewMatrix);
			m_eFlags |= FLAG_HAS_VIEWWORLDMATRIX;
		}

		return m_oViewWorldMatrix;
	}

	const TMatrix44& TRenderContext::GetModelWorldMatrix()
	{
		if (!HASFLAG(m_eFlags & FLAG_HAS_MODELWORLDMATRIX))
		{
			m_oModelWorldMatrix.Multiply(GetViewWorldMatrix(), m_oModelViewMatrix);
			m_eFlags |= FLAG_HAS_MODELWORLDMATRIX;
		}

		return m_oModelWorldMatrix;
	}

	const TMatrix44& TRenderContext::GetViewModelMatrix()
	{
		if (HASFLAG(m_eFlags & FLAG_DIRTY_VIEWMODELMATRIX))
		{
			m_oViewModelMatrix.Invert(m_oModelViewMatrix);
			m_eFlags &= ~FLAG_DIRTY_VIEWMODELMATRIX;
		}

		return m_oViewModelMatrix;
	}

	const TMatrix44& TRenderContext::GetWorldModelMatrix()
	{
		if (HASFLAG(m_eFlags & FLAG_DIRTY_WORLDMODELMATRIX))
		{
			m_oViewModelMatrix.Invert(m_oModelViewMatrix);
			m_eFlags &= ~FLAG_DIRTY_WORLDMODELMATRIX;
		}

		return m_oWorldModelMatrix;
	}

}
