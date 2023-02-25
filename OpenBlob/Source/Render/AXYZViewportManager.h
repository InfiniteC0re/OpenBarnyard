#pragma once

#include "Toshi/Render/TViewport.h"


class AXYZViewportManager
{
public:
	AXYZViewportManager();

	Toshi::TViewport* CreateViewport(float x, float y, float w, float h);
};

