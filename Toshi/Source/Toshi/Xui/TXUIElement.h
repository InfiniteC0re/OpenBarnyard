#pragma once
namespace Toshi
{
	class TXUIElement
	{

		enum Flags : uint8_t
		{
			Flags_BlendModeMask = BITFIELD(3),
			Flags_AnchorMask = BITFIELD(6)
		};

		uint32_t m_uiNumChildren; // 0x16
		void* m_child;
	};
}

