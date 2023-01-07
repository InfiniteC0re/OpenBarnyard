#include "ToshiPCH.h"
#include "TTexture_DX11.h"

void Toshi::TTextureDX11::Init()
{
	TASSERT(0 < m_DataSize, "DataSize is not empty");
	TString8 a_folderPath = TString8::TString8("Data\\TextureOverride\\");
	a_folderPath.Concat(m_imageName);
}
