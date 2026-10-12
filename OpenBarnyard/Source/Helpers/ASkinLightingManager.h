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
		// Day and night lighting textures, e.g. D_bessy_a, D_bessy, N_bessy_a and N_bessy (see Data/MiscTextures/SkinLighting.trb).
		// ASkinShader uses LT_0/LT_1 (day) at shade coefficient 0 and LT_2/LT_3 (night) at 255.
		const TCHAR*     szTex1; // day, "_a"
		const TCHAR*     szTex2; // day
		const TCHAR*     szTex3; // night, "_a"
		const TCHAR*     szTex4; // night
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
