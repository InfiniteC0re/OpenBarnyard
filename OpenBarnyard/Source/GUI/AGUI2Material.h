#pragma once
#include <GUI/T2GUIMaterial.h>

class AGUI2Material
    : public Toshi::T2GUIMaterial
{
public:
	// NOTE: retail registers TMaterial as the parent class (T2GUIMaterial has no TClass)
	TDECLARE_CLASS( AGUI2Material, Toshi::TMaterial );

public:
#ifdef TRENDERINTERFACE_GL
	friend class AGUI2RendererGL;
#else
	friend class AGUI2RendererDX8;
#endif
};
