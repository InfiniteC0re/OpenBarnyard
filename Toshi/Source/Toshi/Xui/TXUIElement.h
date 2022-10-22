#pragma once
#include <Toshi/Xui/TXUIResource.h>
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

	class XURXUIObjectData
	{

		uint16_t unk; //0x12 de blob

		void Load(uint8_t* a_pData);
		void LoadChildren(uint8_t* a_pData);
	};
}

