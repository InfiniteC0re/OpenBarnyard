#pragma once
#include "Includes.h"

namespace Toshi
{
	class TFXAA
	{
	private:
		enum PSBufferOffset
		{
			PSBufferOffset_V2ScreenSize = 26
		};

	public:
		TFXAA();
		~TFXAA();

		void Initialize();

		void Render(ID3D11ShaderResourceView* pShaderResourceView);

	private:
		ID3D11Texture2D* m_pTexture;
		ID3D11ShaderResourceView* m_pTextureView;
		ID3D11RenderTargetView* m_pRenderTarget;
		ID3D11PixelShader* m_pShader;
		UINT m_Width;
		UINT m_Height;
	};
}