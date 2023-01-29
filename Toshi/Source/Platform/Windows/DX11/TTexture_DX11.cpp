#include "ToshiPCH.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"

namespace Toshi
{
	void TTexture::Init()
	{
		TASSERT(0 < m_DataSize, "DataSize is not empty");
		
		TString8 a_Path("Data\\TextureOverride\\");
		a_Path += m_TexName;

		TFile* pFile = TFile::Create(a_Path, TFile::FileMode_Read);

		if (pFile != TNULL)
		{
			if (a_Path.Find(".tga") <= 0)
			{

			}
			else
			{
				pFile->Destroy();
				TASSERT(a_Path.IsIndexValid(0));
			}
		}
	}
}