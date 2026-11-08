#pragma once
#include "Render/TRenderContext.h"

TOSHI_NAMESPACE_START

class MODAPI TRenderContextGL : public TRenderContext
{
public:
	TRenderContextGL( TRenderInterface* a_pRenderer );
	~TRenderContextGL();

	virtual void Update() override;

	void ComputePerspectiveProjection();
	void ComputeOrthographicProjection();

	void ComputePerspectiveFrustum();
	void ComputeOrthographicFrustum();

	void EnableFogHAL();
	void DisableFogHAL();

	const TMatrix44& GetProjectionMatrix() const { return m_Projection; }

	static TRenderContextGL* Upcast( TRenderContext* a_pRenderContext )
	{
		return TSTATICCAST( TRenderContextGL, a_pRenderContext );
	}

#ifndef TOSHI_SDK
private:
#endif

	TMatrix44 m_Projection;
};

TOSHI_NAMESPACE_END
