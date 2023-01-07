#pragma once

#include "Toshi/Strings/TStringManager.h"
#include "Toshi/Render/TTexture.h"

namespace Toshi
{
	class TTextureDX11 : TTexture
	{
		char* m_unk; //0x0
		char* m_imageName; //0x4
		char* m_unk2; //0x8

		uint32_t m_DataSize; // 0xC
		uint8_t* m_TexData; // 0x10

	public:
		void Init();
	};
}
