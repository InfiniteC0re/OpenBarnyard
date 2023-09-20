#include "ToshiPCH.h"
#include "TRenderContext_SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Toshi {

	void TRenderContextSDL::ComputePerspectiveProjection()
	{
		auto mat = glm::perspectiveFov(90.0f, m_oParams.fWidth, m_oParams.fHeight, m_ProjParams.m_fNearClip, m_ProjParams.m_fFarClip);
		
		std::memcpy(&m_Projection, &mat, 64);
		/*m_Projection.m_f11 = (m_ProjParams.m_Proj.x * 2.0f) / m_oParams.fWidth;
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
		m_Projection.m_f44 = 0.0f;*/
	}

	void TRenderContextSDL::ComputePerspectiveFrustum()
	{
		TIMPLEMENT();
	}

	void TRenderContextSDL::ComputeOrthographicProjection()
	{
		TIMPLEMENT();
	}

	void TRenderContextSDL::ComputeOrthographicFrustum()
	{
		TIMPLEMENT();
	}

	void TRenderContextSDL::Update()
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
