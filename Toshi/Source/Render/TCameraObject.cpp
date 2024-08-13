#include "ToshiPCH.h"
#include "TCameraObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TCameraObject::TCameraObject()
{
	m_Unk1	   = 0;
	m_eMode	   = TRenderContext::CameraMode_Perspective;
	m_fNear	   = 1.0f;
	m_fFar	   = 100.0f;
	m_fFOV	   = TMath::DegToRad( 90.0f );
	m_fCentreX = 0.5f;
	m_fCentreY = 0.5f;
	m_bEnabled = TTRUE;
}

void TCameraObject::Render()
{
	if ( m_bEnabled )
	{
		auto pRender		= TRenderInterface::GetSingleton();
		auto pRenderContext = pRender->GetCurrentContext();

		TRenderContext::PROJECTIONPARAMS projParams = pRenderContext->GetProjectionParams();
		projParams.m_Centre							= { m_fCentreX * pRenderContext->GetWidth(), m_fCentreY * pRenderContext->GetHeight() };

		auto render = TRenderInterface::GetSingleton();

		if ( m_Unk1 == 0 )
		{
			projParams.m_Proj.x = ( pRenderContext->GetWidth() * 0.5f ) / tan( m_fFOV * 0.5f );
			projParams.m_Proj.y = projParams.m_Proj.x;

			if ( m_eMode == TRenderContext::CameraMode_Perspective )
			{
				projParams.m_Proj.y *= render->GetPixelAspectRatio();
			}
		}
		else
		{
			projParams.m_Proj.y = ( pRenderContext->GetHeight() * 0.5f ) / tan( m_fFOV * 0.5f );
			projParams.m_Proj.x = projParams.m_Proj.y;

			if ( m_eMode == TRenderContext::CameraMode_Perspective )
			{
				projParams.m_Proj.x /= render->GetPixelAspectRatio();
			}
		}

		projParams.m_fNearClip = m_fNear;
		projParams.m_fFarClip  = m_fFar;

		pRenderContext->SetProjectionParams( projParams );

		if ( m_eMode == TRenderContext::CameraMode_Perspective || m_eMode == TRenderContext::CameraMode_Orthographic )
			pRenderContext->SetCameraMode( m_eMode );

		TMatrix44 matrix;
		m_TransformObject.GetLocalMatrixImp( matrix );
		matrix.InvertOrthogonal();

		pRenderContext->SetWorldViewMatrix( matrix );
		pRenderContext->SetModelViewMatrix( matrix );
		pRenderContext->Update();
	}
}

TOSHI_NAMESPACE_END
