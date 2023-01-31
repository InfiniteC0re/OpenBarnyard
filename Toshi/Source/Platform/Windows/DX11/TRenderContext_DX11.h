#pragma once
#include "Toshi/Render/TRender.h"

namespace Toshi
{
	class TRenderContextDX11 : public TRenderContext
	{
	public:
		TRenderContextDX11(TRender* pRender) : TRenderContext(pRender)
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

		virtual void Update();

	private:
		TMatrix44 m_Projection;
	};
}