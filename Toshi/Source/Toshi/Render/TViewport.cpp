#include "ToshiPCH.h"
#include "Toshi/Render/TViewport.h"

namespace Toshi {

	TViewport::TViewport()
	{
		m_ColorR = 0;
		m_ColorG = 0;
		m_ColorB = 0;
		m_ColorA = 0;
		m_bInViewport = TFALSE;
		m_bAllowBackgroundClear = TTRUE;
		m_bAllowDepthClear = TTRUE;
		m_bDefaultBeginRender = TTRUE;
		m_pRenderCtx = TRenderInterface::GetSingleton()->CreateRenderContext();
		m_MemAllocatorBlock = TNULL;

		SetWidth(-1.0f);
		SetHeight(-1.0f);
	}

	TViewport::~TViewport()
	{
		TIMPLEMENT();
	}

	void TViewport::SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		if (m_ColorR != r || m_ColorG != g || m_ColorB != b || m_ColorA != a)
		{
			m_ColorR = r;
			m_ColorG = g;
			m_ColorB = b;
			m_ColorA = a;

			ChangeSKU(ChangeEvent_BackgroundColor);
		}
	}

	void TViewport::GetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
	{
		r = m_ColorR;
		g = m_ColorG;
		b = m_ColorB;
		a = m_ColorA;
	}

}
