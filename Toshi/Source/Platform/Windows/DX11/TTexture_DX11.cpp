#include "ToshiPCH.h"
#include "TTexture_DX11.h"

void Toshi::TTexture::Init()
{
	TASSERT(0 < m_DataSize, "DataSize is not empty");
	TString8 a_Path = TString8::TString8("Data\\TextureOverride\\");
	a_Path.Concat(m_imageName);
	TFile* file = TFile::Create(a_Path, TFile::FileMode_Read);

	if (file != TNULL)
	{
		if (a_Path.Find(".tga") <= 0)
		{

		}
		else
		{
			file->Destroy();
			TASSERT(a_Path.IsIndexValid(0));
		}
	}
}
