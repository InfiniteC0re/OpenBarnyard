#pragma once
#include "Toshi/Strings/TStringManager.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TTexture
	{
	public:
		struct Info
		{
			UINT Width;
			UINT Height;
			UINT MipMapCount;
			UINT16 Width2;
			UINT16 Height2;
			DXGI_FORMAT Format;
			ID3D11ShaderResourceView* pSRView;
		};

	public:
		void Init();

		// static TTexture* CreateTexture(DXGI_FORMAT format, UINT width, UINT height, const void* srcData);

	private:
		void* m_Unk;              // 0x00
		char* m_TexName;          // 0x04
		Info* m_TexInfo;          // 0x08
		uint32_t m_DataSize;      // 0x0C
		uint8_t* m_TexData;       // 0x10
		TTexture* m_pNextTexture; // 0x14
	};
}
