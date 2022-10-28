#include "ToshiPCH.h"
#include "TRenderContext_DX11.h"

namespace Toshi
{
	TRenderContext* TRenderInterface::CreateRenderContext()
	{
		return new TRenderContextDX11(*this);
	}
}