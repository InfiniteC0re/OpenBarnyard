#pragma once
#include "Render/ASkinShader/ASkinMesh.h"

class ASkinMeshHAL : public ASkinMesh
{
public:
    TDECLARE_CLASS( ASkinMeshHAL, ASkinMesh );

public:
    //-----------------------------------------------------------------------------
    // Toshi::TMesh
    //-----------------------------------------------------------------------------
    virtual TBOOL Render() override;
};
