#include "ToshiPCH.h"
#include "TTexture.h"

namespace Toshi
{

	int TTexture::GetBitsPerTexel(uint8_t a_texFmt)
	{
		switch (a_texFmt)
		{
		case 0x0:
		case 0x8:
		case 0xE:
			return 0x4;
		case 0x1:
		case 0x2:
		case 0x9:
		case 0x11:
			return 0x8;
		case 0x3:
		case 0x4:
		case 0x5:
		case 0xA:
		case 0x13:
			return 0x10;
		case 0x6:
		case 0x16:
			return 0x20;
		default:
			return 0x0;
		}
	}

	int TTexture::ComputeHash(const char* str)
	{
		int hash = 0;
		while (true)
		{
			unsigned char val = *str++;
			if (val == '\0') break;
			hash = hash * 0x11 + val;
		}
		return hash;
	}

	void TTexture::Swizzle()
	{
		uint8_t bitsPerTexel = GetBitsPerTexel(m_texFmt);
		switch (bitsPerTexel)
		{
		case 0x4:
			Swizzle4Bit();
			return;
		case 0x8:
			Swizzle8Bit();
			return;
		case 0x10:
			Swizzle16Bit();
			return;
		case 0x20:
			Swizzle32Bit();
			return;
		default:
			return;
		}
	}

	void TTexture::Swizzle32Bit()
	{
		uint8_t* buf;
		uint8_t* imgPtr3;
		uint32_t height = m_height;
		uint32_t width = m_width;
		for (size_t i = 0; i < m_unk+1; i++)
		{
			int iVar7 = 0;
			uint8_t* imagePtr = m_imagePtr;
			for (size_t j = 0; j < height; j+=4)
			{
				TUtil::MemCopy(buf, imagePtr + (iVar7 * 2), width * 16);
				int iVar1 = 0;
				for (size_t k = 0; k < width; k+=4)
				{
					uint8_t* buf2 = buf + iVar1;
					for (size_t s = 0; s < 2; s++)
					{
						imgPtr3 = imagePtr;
						imgPtr3[0] = buf2[0] >> 0x10;
						imgPtr3[1] = buf2[1] >> 0x10;
						imgPtr3[2] = buf2[2] >> 0x10;
						imgPtr3[3] = buf2[3] >> 0x10;

						imgPtr3[16] = buf2[0];
						imgPtr3[17] = buf2[1];
						imgPtr3[18] = buf2[2];
						imgPtr3[19] = buf2[3];

						uint8_t* buf3 = buf2 + width;
						imgPtr3[4] = buf3[0] >> 0x10;
						imgPtr3[5] = buf3[1] >> 0x10;
						imgPtr3[6] = buf3[2] >> 0x10;
						imgPtr3[7] = buf3[3] >> 0x10;

						imgPtr3[20] = buf3[0];
						imgPtr3[21] = buf3[1];
						imgPtr3[22] = buf3[2];
						imgPtr3[23] = buf3[3];

						buf2 = buf3 + width;
						imagePtr += 8;
					}
					imagePtr = imgPtr3 + 24;
					iVar1 += 16;
				}
				iVar7 += width * 4;
			}
			height /= 2;
			width /= 2;
			imagePtr += 32;
		}
	}
}


