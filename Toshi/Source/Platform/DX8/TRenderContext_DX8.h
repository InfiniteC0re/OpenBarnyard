#pragma once
#include "Render/TRenderContext.h"

namespace Toshi
{

class TRenderContextD3D : public TRenderContext
{
public:
    TRenderContextD3D( TRenderInterface* a_pRenderer );
    ~TRenderContextD3D();

    virtual void Update() override;

    void ComputePerspectiveProjection();
    void ComputeOrthographicProjection();

    void ComputePerspectiveFrustum();
    void ComputeOrthographicFrustum();

    void EnableFogHAL();
    void DisableFogHAL();

    const TMatrix44& GetProjectionMatrix() const { return m_Projection; }

    static TRenderContextD3D* Upcast( TRenderContext* a_pRenderContext )
    {
        return TSTATICCAST( TRenderContextD3D, a_pRenderContext );
    }

private:
    TMatrix44 m_Projection;
};

} // namespace Toshi
