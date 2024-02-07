#pragma once
#include "Render/TRenderContext.h"

namespace Toshi {

	class TRenderContextD3D : public TRenderContext
	{
	public:
		TRenderContextD3D(TRenderInterface* a_pRenderer) : TRenderContext(a_pRenderer) { }

		virtual void Update() override;

		void ComputePerspectiveProjection();
		void ComputePerspectiveFrustum();
		void ComputeOrthographicProjection();
		void ComputeOrthographicFrustum();

		const TMatrix44& GetProjectionMatrix() const { return m_Projection; }

	private:
		TMatrix44 m_Projection;
	};

}
