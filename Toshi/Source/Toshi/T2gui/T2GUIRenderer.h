#pragma once
#include "Toshi/Render/TTexture.h"

namespace Toshi
{
	class T2GUIMaterial;

	class T2GUIRenderer
	{
	public:
		virtual ~T2GUIRenderer() = default;

		virtual TTexture* GetTexture(char* texName) const = 0;
		virtual void BeginScene(char* texName) = 0;
		virtual void EndScene(char* texName) = 0;
		virtual void SetMaterial(T2GUIMaterial* pMat) = 0;
		virtual void PushTransform(...) = 0;
		virtual void PopTransform() = 0;

	private:
		int m_WhiteGlyphsCount;
	};

}
