#include "ToshiPCH.h"
#include "TTexture_Rev.h"

namespace Toshi
{
	int TTexture::GetBitsPerTexel(GXTexFmt a_texFmt)
	{
		switch (a_texFmt)
		{
		case I4:
		case C4:
		case CMPR:
			return 4;
		case I8:
		case IA4:
		case C8:
		case 0x11:
			return 8;
		/*case IA8:
		case RGB565:
		case RGB5A3:*/
		case C14X2:
		case 0x13:
			return 16;
		case RGBA8:
		case 0x16:
			return 32;
		default:
			return 0;
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

	void TTexture::CheckValid()
	{
		if (m_texFmt - 8 < 3)
		{
			m_unk2 |= 0x20000000;
		}

		if (m_mipMaps != 0)
		{
			m_unk2 |= 0x10000000;
			m_unk3 = 2;
			m_magFilter = GX_LIN_MIP_LIN;
			m_unk4 = 0.5f * -(m_mipMaps - 176.0f);
		}

		if (m_texFmt - 8 < 2)
		{
			m_unk2 |= 0x20000000;
		}

		m_unk2 |= 0x80000000;
	}

	void TTexture::InitTexObj()
	{
		CheckValid();

		if (m_mipMaps & 0x20000000)
		{
			m_pImageData = new byte[m_iImageSize];
			if (m_width != (m_width & -m_width) || m_height != (m_height & -m_height))
			{
				m_wrapS = 0;
				m_wrapT = 0;
				m_mipMaps = 0;
			}
		}
	}

	void TTexture::InitRuntime(GXTexFmt a_texFmt, GXTlutFmt a_tlutFmt, unsigned int a_unk, unsigned int a_unk2, unsigned int a_unk3, unsigned int a_unk4, void* a_unk5, void* a_unk6, char const* a_szFileName)
	{
		m_pImageData = TNULL;
		m_texFmt = a_texFmt;
		m_tlutFmt = a_tlutFmt;
		m_wrapS = GX_REPEAT;
		m_wrapT = GX_REPEAT;
		m_szFileName = (char*)a_szFileName;
		
		m_iHash = ComputeHash(m_szFileName);

		if (m_texFmt - 8 < 3)
		{
			a_unk3 |= 0x20000000;
		}
		else
		{
			a_unk3 |= 0x10000000;
		}



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
		uint32_t* imgPtr3;
		uint32_t height = m_height;
		uint32_t width = m_width;
		for (size_t i = 0; i < m_mipMaps + 1; i++)
		{
			int iVar7 = 0;
			uint32_t* imagePtr = reinterpret_cast<uint32_t*>(m_pImageData);
			for (size_t j = 0; j < height; j += 4)
			{
				TUtil::MemCopy(buf, imagePtr + (iVar7 * 2), width * 16);
				int iVar1 = 0;
				for (size_t k = 0; k < width; k += 4)
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

	void TTexture::GetPhysicalSize(uint32_t& width, uint32_t& height, uint32_t bitsPerTexel)
	{
		if (bitsPerTexel < 16)
		{
			
		}
	}
}