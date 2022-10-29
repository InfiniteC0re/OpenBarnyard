#pragma once
#include "Toshi/Render/TRender.h"

namespace Toshi
{
	class TRenderContextDX11 : public TRenderContext
	{
	public:
		TRenderContextDX11() : TRenderContext() {}
		TRenderContextDX11(TRender& renderer) : TRenderContext(renderer) {}

	};
}