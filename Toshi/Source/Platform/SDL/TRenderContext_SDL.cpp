#include "ToshiPCH.h"
#include "TRenderContext_SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Toshi {

	void TRenderContextSDL::ComputePerspectiveProjection()
	{
		TIMPLEMENT();
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
