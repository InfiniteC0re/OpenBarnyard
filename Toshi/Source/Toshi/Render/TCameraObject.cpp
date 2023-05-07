#include "ToshiPCH.h"
#include "TCameraObject.h"
#include "TRender.h"

namespace Toshi {

TCameraObject::TCameraObject()
{
	m_fNear = 1.0f;
	m_fFar = 100.0f;
	m_fFOV = TMath::HALF_PI;
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

		if (m_Unk1 == 0)
		{
			float fHalfWidth = pRenderContext->GetWidth() * 0.5f;
			double dHalfFOV = m_fFOV * 0.5;
			TTODO("__libm_sse2_tan_precise?");
		}
		else
		{
			float fHalfheight = pRenderContext->GetHeight() * 0.5f;
			double dHalfFOV = m_fFOV * 0.5;
			TTODO("__libm_sse2_tan_precise?");
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
