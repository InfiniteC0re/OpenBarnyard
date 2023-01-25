#pragma once
#include "Toshi/Render/TRender.h"

namespace Toshi
{
	class TViewport
	{
	public:
		using ChangeEvent = uint32_t;
		enum ChangeEvent_ : ChangeEvent
		{
			ChangeEvent_Unk1,
			ChangeEvent_Unk2,
			ChangeEvent_Unk3,
			ChangeEvent_Unk4,
			ChangeEvent_Unk5,
		};

	public:
		TViewport();
		~TViewport();

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

		void ChangeSKU(ChangeEvent event)
		{

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

		bool AllowBackgroundClear(bool allow)
		{
			bool oldState = m_bAllowBackgroundClear;
			m_bAllowBackgroundClear = allow;
			ChangeSKU(ChangeEvent_Unk5);
			return oldState;
		}

		bool AllowDepthClear(bool allow)
		{
			bool oldState = m_bAllowDepthClear;
			m_bAllowDepthClear = allow;
			ChangeSKU(ChangeEvent_Unk5);
			return oldState;
		}

		bool EnableDefaultBeginRender(bool enable)
		{
			bool oldState = m_bDefaultBeginRender;
			m_bDefaultBeginRender = enable;
			return oldState;
		}

		void SetMemoryAllocatorBlock(TMemoryHeap* pHeap)
		{
			m_MemAllocatorBlock = pHeap;
			ChangeSKU(ChangeEvent_Unk5);
		}

		void BeginSKU();
		void EndSKU();
		void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		void GetBackgroundColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

	private:
		TMemoryHeap* m_MemAllocatorBlock; // 0x00
		TRenderContext* m_pRenderCtx;     // 0x04
		uint8_t m_ColorR;                 // 0x08
		uint8_t m_ColorG;                 // 0x09
		uint8_t m_ColorB;                 // 0x0A
		uint8_t m_ColorA;                 // 0x0B
		bool m_bAllowBackgroundClear;     // 0x0C
		bool m_bAllowDepthClear;          // 0x0D
		bool m_bDefaultBeginRender;       // 0x0E
		bool m_bInViewport;               // 0x10
	};
}