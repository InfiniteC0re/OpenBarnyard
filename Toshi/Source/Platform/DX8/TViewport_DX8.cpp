#include "ToshiPCH.h"
#include "Toshi/Render/TViewport.h"
#include "TRenderInterface_DX8.h"

namespace Toshi {

	void TViewport::BeginSKU()
	{
		D3DVIEWPORT8 viewport;
		viewport.X = DWORD(m_pRenderCtx->GetParams().fX);
		viewport.Y = DWORD(m_pRenderCtx->GetParams().fY);
		viewport.Width = DWORD(m_pRenderCtx->GetParams().fWidth);
		viewport.Height = DWORD(m_pRenderCtx->GetParams().fHeight);
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;

		TUINT8 r, g, b, a;
		GetBackgroundColor(r, g, b, a);

		auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
		auto pD3DDevice = pRender->GetDirect3DDevice();

		pD3DDevice->SetViewport(&viewport);
		
		TUINT eClearFlags = ((m_bAllowDepthClear != TTRUE) - 1U & 2 | m_bAllowBackgroundClear == TTRUE);
		
		if (eClearFlags != 0)
		{
			pRender->Clear(
				DWORD(m_pRenderCtx->GetParams().fX),
				DWORD(m_pRenderCtx->GetParams().fY),
				DWORD(m_pRenderCtx->GetParams().fWidth),
				DWORD(m_pRenderCtx->GetParams().fHeight),
				eClearFlags,
				r,
				g,
				b,
				1.0f,
				0
			);
		}
	}

	void TViewport::EndSKU()
	{
		TRenderInterface::GetSingleton()->FlushOrderTables();
	}

	void TViewport::ChangeSKU(ChangeEvent a_eEvent)
	{

	}

}