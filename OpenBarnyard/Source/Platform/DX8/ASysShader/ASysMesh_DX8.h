#pragma once
#include "Render/ASysShader/ASysMesh.h"

class ASysMeshHAL : public ASysMesh
{
public:
    TDECLARE_CLASS( ASysMeshHAL, ASysMesh );

public:
    ASysMeshHAL();

    virtual TBOOL Render() override;
    virtual void  SetZBias( TINT a_iZBias ) override;

    TINT GetZBias() const { return m_iZBias; }

public:
    inline static BOOL ms_bStopRendering;

private:
    TINT m_iZBias;
};
