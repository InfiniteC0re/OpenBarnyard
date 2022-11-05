#pragma once
#include <Toshi/Xui/TXUIResource.h>
namespace Toshi
{
	enum Flags : uint8_t
	{
		Flags_BlendModeMask = BITFIELD(3),
		Flags_AnchorMask = BITFIELD(6)
	};

	class TXUIElement :
		public TGenericClassDerived<TXUIElement, TObject, "TXUIElement", TMAKEVERSION(1, 0), false>
	{
		
		uint32_t m_uiNumNamedFrames; // 0x10
		uint32_t m_uiNumChildren; // 0x16
		void* m_child;
	};

	class XURXUIObjectData
	{
	public:
		XURXUIObjectData()
		{

		}

		uint16_t unk; //0x12 de blob 0x1C NT08

		void Load(uint8_t*& a_pData);
		void LoadChildren(uint8_t* a_pData);
	};

	class XURXUIElementData : public XURXUIObjectData
	{
	public:
		XURXUIElementData() : XURXUIObjectData()
		{

		}

		uint16_t m_unk2; //0x1C de blob
		uint16_t m_unk3; //0x1E de blob
		uint16_t m_unk4; //0x20 de blob
		uint16_t m_unk5; //0x22 de blob
		uint16_t m_unk6; //0x24 de blob
		uint16_t m_unk7; //0x26 de blob
		uint16_t m_unk; //0x28 de blob
		uint32_t m_unkFlags; //0x2A de blob
		uint32_t m_fOpacity; //0x2C de blob

		void Load(uint8_t*& a_pData);
	};
}

