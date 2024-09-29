#pragma once

#include "Toshi/T2ResourceManager.h"



TOSHI_NAMESPACE_START

class TTexture
{

	enum GXTexFmt
	{
		I4     = 0x0,
		I8     = 0x1,
		IA4    = 0x2,
		IA8    = 0x3,
		RGB565 = 0x4,
		RGB5A3 = 0x5,
		RGBA8  = 0x6,
		C4     = 0x8,
		C8     = 0x9,
		C14X2  = 0xA,
		CMPR   = 0xE,
	};

	enum GXTlutFmt
	{
		GX_TL_IA8    = 0x0,
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

	TCHAR*      m_szFileName; //0x4
	TINT        m_iHash;      //0x8
	GXTexFmt    m_texFmt;     //0x38
	GXTlutFmt   m_tlutFmt;    //0x3C
	TUINT32     m_unk2;       //0x40
	uint8_t*    m_pImageData; //0x44
	TUINT32     m_iImageSize; //0x48
	TUINT32     m_wrapS;      //0x58
	TUINT32     m_wrapT;      //0x5C
	TUINT32     m_unk3;       //0x60
	GXTexFilter m_minFilter;  //0x64
	GXTexFilter m_magFilter;  //0x68
	TUINT32     m_unk4;       //0x70
	TUINT32     m_width;      //0x78
	TUINT32     m_height;     //0x7A
	TUINT32     m_mipMaps;    //0x80

	//static t_CreateDestroyCallbk ResourceCallback;

	TINT GetBitsPerTexel( GXTexFmt a_texFmt );
	void CreateResource();

	TINT ComputeHash( const TCHAR* str );
	void CheckValid();
	void InitTexObj();
	void InitRuntime( GXTexFmt a_texFmt, GXTlutFmt a_tlutFmt, TUINT a_unk, TUINT a_unk2, TUINT a_unk3, TUINT a_unk4, void* a_unk5, void* a_unk6, TCHAR const* a_szFileName );

	void GetPhysicalSize( TUINT32& width, TUINT32& height, TUINT32 bitsPerTexel );

	void Swizzle();
	void Swizzle32Bit();
	void Swizzle16Bit();
	void Swizzle8Bit();
	void Swizzle4Bit();

	//void CreateResource() { T2Resource::CreateResource(m_szFileName, this, ResourceCallback, this); }
};

TOSHI_NAMESPACE_END
