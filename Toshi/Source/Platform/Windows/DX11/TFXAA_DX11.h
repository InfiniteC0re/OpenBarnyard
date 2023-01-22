#pragma once
#include "Includes.h"

namespace Toshi
{
	class TFXAA
	{
	public:
		TFXAA();
		~TFXAA();

		void Initialize();

		void Render(ID3D11ShaderResourceView* pShaderResourceView);
		void FUN_006a6700(float posX, float posY, float width, float height, ID3D11ShaderResourceView* pShaderResourceView, ID3D11PixelShader* pPixelShader, const void* srcData);

	private:
		ID3D11Texture2D* m_pTexture;
		ID3D11ShaderResourceView* m_pTextureView;
		ID3D11RenderTargetView* m_pRenderTarget;
		ID3D11PixelShader* m_pShader;
		UINT m_Width;
		UINT m_Height;
	};
}