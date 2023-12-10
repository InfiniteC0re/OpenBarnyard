#pragma once
#include "Toshi/Render/TRenderInterface.h"
#include "Toshi/Render/TRenderContext.h"

namespace Toshi
{
	class TViewport
	{
	public:
		using ChangeEvent = uint32_t;
		enum ChangeEvent_ : ChangeEvent
		{
			ChangeEvent_BackgroundColor,
			ChangeEvent_MaxZ,
			ChangeEvent_MinZ,
			ChangeEvent_Height,
			ChangeEvent_Width,
			ChangeEvent_X,
			ChangeEvent_Y,
			ChangeEvent_AllocatorBlock,
			ChangeEvent_BackgroundClear,
			ChangeEvent_DepthClear,
		};

	public:
		TViewport(TBOOL a_bFlag);
		virtual ~TViewport();

		virtual void BeginSKU();
		virtual void EndSKU();
		virtual void ChangeSKU(ChangeEvent a_eEvent);

		float GetX() const
		{
			return m_pRenderCtx->GetX();
		}

		float GetY() const
		{
			return m_pRenderCtx->GetY();
		}

		float GetWidth() const
		{
			return m_pRenderCtx->GetWidth();
		}

		float GetHeight() const
		{
			return m_pRenderCtx->GetHeight();
		}

		void Begin()
		{
			TASSERT(m_bInViewport == TFALSE);
			m_bInViewport = TTRUE;
			BeginSKU();
		}

		void End()
		{
			EndSKU();
			TASSERT(m_bInViewport == TTRUE);
			m_bInViewport = TFALSE;
		}

		TBOOL AllowBackgroundClear(TBOOL allow)
		{
			TBOOL oldState = m_bAllowBackgroundClear;
			m_bAllowBackgroundClear = allow;
			ChangeSKU(ChangeEvent_BackgroundClear);
			return oldState;
		}

		TBOOL AllowDepthClear(TBOOL allow)
		{
			TBOOL oldState = m_bAllowDepthClear;
			m_bAllowDepthClear = allow;
			ChangeSKU(ChangeEvent_DepthClear);
			return oldState;
		}

		void Enable(TBOOL enable)
		{
			m_bEnable = enable;
		}

		TBOOL EnableDefaultBeginRender(TBOOL enable)
		{
			TBOOL oldState = m_bDefaultBeginRender;
			m_bDefaultBeginRender = enable;
			return oldState;
		}

		void SetMemoryAllocatorBlock(TMemory::MemBlock* a_pMemBlock)
		{
			m_MemAllocatorBlock = a_pMemBlock;
			ChangeSKU(ChangeEvent_AllocatorBlock);
		}

		void SetX(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fX != value)
			{
				params.fX = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_X);
			}
		}

		void SetY(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fY != value)
			{
				params.fY = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_Y);
			}
		}

		void SetWidth(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fWidth != value)
			{
				params.fWidth = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_Width);
			}
		}

		void SetHeight(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fHeight != value)
			{
				params.fHeight = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_Height);
			}
		}

		void SetMinZ(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fMinZ != value)
			{
				params.fMinZ = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_MinZ);
			}
		}

		void SetMaxZ(float value)
		{
			TRenderContext::Params params = m_pRenderCtx->GetParams();

			if (params.fMaxZ != value)
			{
				params.fMaxZ = value;
				m_pRenderCtx->SetViewportParameters(params);
				ChangeSKU(ChangeEvent_MaxZ);
			}
		}

		TRenderContext* GetRenderContext() const
		{
			return m_pRenderCtx;
		}

		void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		void GetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

	private:
		TMemory::MemBlock* m_MemAllocatorBlock; // 0x00
		TRenderContext* m_pRenderCtx;           // 0x04
		uint8_t m_ColorR;                       // 0x08
		uint8_t m_ColorG;                       // 0x09
		uint8_t m_ColorB;                       // 0x0A
		uint8_t m_ColorA;                       // 0x0B
		TBOOL m_bAllowBackgroundClear;          // 0x0C
		TBOOL m_bAllowDepthClear;               // 0x0D
		TBOOL m_bDefaultBeginRender;            // 0x0E
		TBOOL m_bEnable;                        // 0x0F
		TBOOL m_bInViewport;                    // 0x10
	};
}