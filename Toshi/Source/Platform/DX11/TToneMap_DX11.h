#pragma once
#include "Includes.h"

namespace Toshi
{
	class TToneMap
	{
	public:
		TToneMap();
		~TToneMap();

	private:
		ID3D11ShaderResourceView* m_pTextureView;
		ID3D11RenderTargetView* m_pRenderTarget;
		uint32_t m_Unk1;
		uint32_t m_Unk2;
		uint32_t m_Unk3;
		uint32_t m_Unk4;
		uint32_t m_Unk5;
	};
}