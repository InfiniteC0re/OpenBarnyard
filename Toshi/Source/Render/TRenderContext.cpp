#include "ToshiPCH.h"
#include "TRenderContext.h"
#include "TRenderInterface.h"

namespace Toshi {

	TRenderContext::TRenderContext(TRenderInterface* pRender)
	{
		TIMPLEMENT();
		m_eFlags = FLAG_DIRTY;
		m_eCameraMode = CameraMode_Perspective;
		m_pCurrentSkeletonInstance = TNULL;
		m_pRenderer = pRender;

		// Setup viewport parameters
		if (pRender->GetCurrentDevice() != TNULL)
		{
			auto pDevice = pRender->GetCurrentDevice();
			auto pMode = pDevice->GetMode();
			m_oParams.fWidth = TFLOAT(pMode->GetWidth());
			m_oParams.fHeight = TFLOAT(pMode->GetHeight());
		}
		else
		{
			m_oParams.fWidth = 640.0f;
			m_oParams.fHeight = 480.0f;
		}

		m_oParams.fX = 0;
		m_oParams.fY = 0;
		m_oParams.fMaxZ = 1.0f;
		m_oParams.fMinZ = 1.0f;

		// Setup projection parameters
		TFLOAT fHalfWidth = m_oParams.fWidth * 0.5f;
		TFLOAT fHalfHeight = m_oParams.fHeight * 0.5f;
		TFLOAT fProj = fHalfHeight / TMath::Tan(TMath::DegToRad(45.0f));

		m_ProjParams.m_fNearClip = 1.0f;
		m_ProjParams.m_fFarClip = 1000.0f;
		m_ProjParams.m_Centre.x = fHalfWidth;
		m_ProjParams.m_Centre.y = fHalfHeight;
		m_ProjParams.m_Proj.x = fProj;
		m_ProjParams.m_Proj.y = fProj;

		m_oModelViewMatrix.Identity();
		m_oWorldViewMatrix.Identity();
		m_eFlags |= FLAG_DIRTY_VIEWMODELMATRIX | FLAG_DIRTY_WORLDMODELMATRIX;
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

	TBOOL TRenderContext::CullSphereToFrustumSimple(const TSphere& a_rSphere, const TPlane* a_pPlanes, int a_iNumPlanes)
	{
		for (size_t i = 0; i < 6; i++)
		{
			TFLOAT fDist = TVector4::DotProduct3(a_rSphere.AsVector4(), a_pPlanes[i].AsVector4());

			if (a_rSphere.GetRadius() < fDist - a_pPlanes[i].GetD())
				return TFALSE;
		}

		return TTRUE;
	}

	TINT TRenderContext::CullSphereToFrustum(const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask)
	{
		TINT iLeftPlanes = a_iClipFlags & a_iClipFlagsMask;
		TINT iPlaneFlag = 1;

		do {
			if (iLeftPlanes == 0)
			{
				return a_iClipFlags;
			}

			if (iLeftPlanes & iPlaneFlag)
			{
				TFLOAT fDist = TVector4::DotProduct3(a_rSphere.AsVector4(), a_pPlanes->AsVector4()) - a_pPlanes->GetD();

				if (a_rSphere.GetRadius() < fDist)
				{
					return -1;
				}

				if (fDist < -a_rSphere.GetRadius())
				{
					a_iClipFlags &= ~iPlaneFlag;
				}

				iLeftPlanes &= ~iPlaneFlag;
			}

			iPlaneFlag = iPlaneFlag << 1;
			a_pPlanes++;

		} while (TTRUE);
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