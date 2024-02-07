#pragma once

#include "Toshi/T2ResourceManager.h"



namespace Toshi
{
	class TTexture
	{

		enum GXTexFmt
		{
			I4 = 0x0,
			I8 = 0x1,
			IA4 = 0x2,
			IA8 = 0x3,
			RGB565 = 0x4,
			RGB5A3 = 0x5,
			RGBA8 = 0x6,
			C4 = 0x8,
			C8 = 0x9,
			C14X2 = 0xA,
			CMPR = 0xE,
		};

		enum GXTlutFmt
		{
			GX_TL_IA8 = 0x0,
			GX_TL_RGB565 = 0x1,
			GX_TL_RGB5A3 = 0x2,
		};

		enum GXTexWrapMode
		{
			GX_CLAMP,
			GX_REPEAT,
			GX_MIRROR
		};

		enum GXTexFilter
		{
			GX_NEAR,
			GX_LINEAR,
			GX_NEAR_MIP_NEAR,
			GX_LIN_MIP_NEAR,
			GX_NEAR_MIP_LIN,
			GX_LIN_MIP_LIN
		};

		char* m_szFileName;     //0x4
		int m_iHash;            //0x8
		GXTexFmt m_texFmt;      //0x38
		GXTlutFmt m_tlutFmt;    //0x3C
		uint32_t m_unk2;        //0x40
		uint8_t* m_pImageData;  //0x44
		uint32_t m_iImageSize;  //0x48
		uint32_t m_wrapS;       //0x58
		uint32_t m_wrapT;       //0x5C
		uint32_t m_unk3;        //0x60
		GXTexFilter m_minFilter;//0x64
		GXTexFilter m_magFilter;//0x68
		uint32_t m_unk4;        //0x70
		uint32_t m_width;       //0x78
		uint32_t m_height;      //0x7A
		uint32_t m_mipMaps;     //0x80

		//static t_CreateDestroyCallbk ResourceCallback;

		int GetBitsPerTexel(GXTexFmt a_texFmt);
		void CreateResource();

		int ComputeHash(const char* str);
		void CheckValid();
		void InitTexObj();
		void InitRuntime(GXTexFmt a_texFmt,
			GXTlutFmt a_tlutFmt,
			unsigned int a_unk,
			unsigned int a_unk2,
			unsigned int a_unk3,
			unsigned int a_unk4,
			void* a_unk5,
			void* a_unk6,
			char const* a_szFileName);

		void GetPhysicalSize(uint32_t& width, uint32_t& height, uint32_t bitsPerTexel);

		void Swizzle();
		void Swizzle32Bit();
		void Swizzle16Bit();
		void Swizzle8Bit();
		void Swizzle4Bit();

		//void CreateResource() { T2Resource::CreateResource(m_szFileName, this, ResourceCallback, this); }
	};
}
