#pragma once

#include "Toshi2/T2ResourceManager.h"

namespace Toshi
{
	class TTexture
	{
		char* m_imageName; //0x4
		uint8_t m_texFmt; //0x38
		uint8_t* m_imagePtr; //0x44
		uint32_t m_wrapS; //0x58
		uint32_t m_wrapT; //0x5C
		uint32_t m_minFilter; //0x64
		uint32_t m_maxFilter; //0x68
		uint32_t m_width; //0x78
		uint32_t m_height; //0x7A
		uint32_t m_unk; //0x80

		static t_CreateDestroyCallbk ResourceCallback;

		int GetBitsPerTexel(uint8_t a_texFmt);
		inline void CreateResource() { T2Resource::CreateResource(m_imageName, this, ResourceCallback, this); }

		int ComputeHash(const char* str);

		void Swizzle();
		void Swizzle32Bit();
		void Swizzle16Bit();
		void Swizzle8Bit();
		void Swizzle4Bit();
	};
}