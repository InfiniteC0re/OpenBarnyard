#pragma once
#include "Toshi/Render/TRender.h"

namespace Toshi {

	class TRenderContextSDL : public TRenderContext
	{
	public:
		TRenderContextSDL(TRender* a_pRender) : TRenderContext(a_pRender)
		{

		}

		TMatrix44& GetProjectionMatrix()
		{
			return m_Projection;
		}

		void ComputePerspectiveProjection();
		void ComputePerspectiveFrustum();
		void ComputeOrthographicProjection();
		void ComputeOrthographicFrustum();

		virtual void Update() override;

	private:
		TMatrix44 m_Projection;
	};

}
