#pragma once

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
			ChangeEvent_BackgroundColor,
		};

	public:
		TViewport()
		{

		}

		void ChangeSKU(ChangeEvent event)
		{

		}

		void BeginSKU()
		{

		}

		void SetBackgroundColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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

	private:
		uint8_t m_ColorR;             // 0x08
		uint8_t m_ColorG;             // 0x09
		uint8_t m_ColorB;             // 0x0A
		uint8_t m_ColorA;             // 0x0B
		bool m_bAllowBackgroundClear; // 0x0C
		bool m_bInViewport;           // 0x10
	};
}