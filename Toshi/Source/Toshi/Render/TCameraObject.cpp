#include "ToshiPCH.h"
#include "TCameraObject.h"
#include "TRender.h"

namespace Toshi {

	TCameraObject::TCameraObject()
	{
		m_fNear = 1.0f;
		m_fFar = 100.0f;
		m_fFOV = TMath::DegToRad(90.0f);
		m_Unk1 = 0;
		m_eMode = TRenderContext::CameraMode_Perspective;
		m_fCentreX = 0.5f;
		m_fCentreY = 0.5f;
		m_bEnabled = TTRUE;
		m_fUnk2 = 1.0f;
	}

	void TCameraObject::Render()
	{
		if (m_bEnabled)
		{
			auto pRender = TRender::GetSingletonWeak();
			auto pRenderContext = pRender->GetCurrentRenderContext();

			TRenderContext::PROJECTIONPARAMS projParams = pRenderContext->GetProjectionParams();
			projParams.m_Centre = { m_fCentreX * pRenderContext->GetWidth(), m_fCentreY * pRenderContext->GetHeight() };

			auto render = TRender::GetSingletonWeak();
			auto aspectRatio = render->GetAspectRatio();

			if (m_Unk1 == 0)
			{
				projParams.m_Proj.x = (pRenderContext->GetWidth() * 0.5f) / tan(m_fFOV * 0.5f);
				projParams.m_Proj.y = projParams.m_Proj.x;

				if (m_eMode == TRenderContext::CameraMode_Perspective)
				{
					projParams.m_Proj.y *= render->GetResolutionScalar();

					if (m_eMode == TRenderContext::CameraMode_Perspective && aspectRatio == TRender::ASPECT_RATIO_16_9)
					{
						projParams.m_Proj.x *= 0.8660254f;
						projParams.m_Proj.y *= 0.8660254f;
					}
				}
			}
			else
			{
				projParams.m_Proj.x = (pRenderContext->GetHeight() * 0.5f) / tan(m_fFOV * 0.5f);
				projParams.m_Proj.y = projParams.m_Proj.x;

				if (m_eMode == TRenderContext::CameraMode_Perspective)
				{
					projParams.m_Proj.x /= render->GetResolutionScalar();

					if (m_eMode == TRenderContext::CameraMode_Perspective && aspectRatio == TRender::ASPECT_RATIO_16_9)
					{
						projParams.m_Proj.x *= 0.8660254f;
						projParams.m_Proj.y *= 0.8660254f;
					}
				}
			}

			projParams.m_fNearClip = m_fNear;
			projParams.m_fFarClip = m_fFar;
			projParams.m_fUnk = m_fUnk2;

			pRenderContext->SetProjectionParams(projParams);

			if (m_eMode == TRenderContext::CameraMode_Perspective || m_eMode == TRenderContext::CameraMode_Orthographic)
				pRenderContext->SetCameraMode(m_eMode);

			if (m_TransformObject.HasChanged())
			{
				m_TransformObject.GetLocalMatrixImp(m_TransformMatrix);
				m_TransformObject.ResetChangedState();
			}

			TMatrix44 viewMatrix = m_TransformMatrix;
			viewMatrix.InvertOrthogonal();

			pRenderContext->SetWorldViewMatrix(viewMatrix);
			pRenderContext->SetModelViewMatrix(viewMatrix);
			pRenderContext->Update();
		}
	}

}
