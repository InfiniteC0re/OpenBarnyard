#pragma once
#include "Assets/AMaterialLibrary.h"

#include <Toshi/TSingleton.h>
#include <File/TTRB.h>

class ASkinLightingManager
    : public Toshi::TSingleton<ASkinLightingManager>
{
public:
	struct SkinLight
	{
		// TODO: Figure out which textures are for day and for night
		const TCHAR*     szTex1;
		const TCHAR*     szTex2;
		const TCHAR*     szTex3;
		const TCHAR*     szTex4;
		Toshi::TTexture* pTex1;
		Toshi::TTexture* pTex2;
		Toshi::TTexture* pTex3;
		Toshi::TTexture* pTex4;
		TINT             iNumModels;
		const TCHAR**    pszModels;
	};

	struct SkinLightTRBHeader
	{
		TINT       iNumLights;
		SkinLight* pSkinLights;
	};

public:
	ASkinLightingManager();
	~ASkinLightingManager();

	void       ApplySkinLight( Toshi::TManagedModel* a_pModel, const TCHAR* a_szMaterialName, SkinLight* a_pLight );
	SkinLight* FindSkinLight( const TCHAR* a_szMaterialName );

private:
	TINT                m_iUnk;
	AMaterialLibrary*   m_pSkinLightMatLib;
	Toshi::TTRB         m_oTRB;
	SkinLightTRBHeader* m_pSkinLightHeader;
};
