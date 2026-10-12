#pragma once
#include "Render/AStaticInstanceShader/AStaticInstanceMaterial.h"

class AStaticInstanceMaterialHAL
    : public AStaticInstanceMaterial
{
public:
	TDECLARE_CLASS( AStaticInstanceMaterialHAL, AStaticInstanceMaterial );

public:
	AStaticInstanceMaterialHAL();
	~AStaticInstanceMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() OVERRIDE;
	virtual void PostRender() OVERRIDE;
};

TSTATICASSERT( sizeof( AStaticInstanceMaterialHAL ) == 0x60 );
