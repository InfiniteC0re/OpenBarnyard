#pragma once
#include "Render/AGrassShader/AGrassMaterial.h"

class MODAPI AGrassMaterialHAL
    : public AGrassMaterial
{
public:
	TDECLARE_CLASS( AGrassMaterialHAL, AGrassMaterial );

public:
	AGrassMaterialHAL();
	~AGrassMaterialHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() OVERRIDE;
	virtual void PostRender() OVERRIDE;
};
