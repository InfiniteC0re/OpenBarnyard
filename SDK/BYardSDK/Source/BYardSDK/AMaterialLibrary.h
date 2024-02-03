#pragma once
#include <Toshi/File/TTRB.h>

#include <Toshi2/T2SimpleArray.h>
#include <Toshi2/T2DList.h>

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/T2Texture_DX8.h"
#endif // TOSHI_SKU_WINDOWS

class ATexture
{
public:
	Toshi::T2Texture* pTexture;
	Toshi::TString8 Name;
};

class AMaterialLibrary :
	public Toshi::T2DList<AMaterialLibrary>::Node
{
public:
	struct TTL
	{
		struct TexInfo
		{
			BOOL m_bIsT2Texture;
			const char* m_szFileName;
			TUINT m_uiDataSize;
			void* m_pData;
		};

		TINT m_iNumTextures;
		TexInfo* m_pTextureInfos;
		const char* m_szPackName;
	};

public:
	AMaterialLibrary();

	TBOOL LoadTTLData(void* a_pTTLData);
	TBOOL LoadTTLFile(const char* a_szFilePath);
	TBOOL LoadTRBFile(Toshi::TTRB* a_pTRB);

	void Destroy();

	TINT FindTextureIndex(const char* a_szTextureName);

	TINT GetNumTextures() const
	{
		return m_iNumTextures;
	}

	ATexture* GetTexture(TINT a_uiIndex)
	{
		TASSERT(a_uiIndex < m_iNumTextures);
		return m_pTextures + a_uiIndex;
	}

	void SetPath(const char* a_szFilePath)
	{
		m_Path = a_szFilePath;
	}

private:
	void DestroyTextures();

public:
	inline static TBOOL ms_bSkipLoadedTextures = TFALSE;

public:
	Toshi::TString8 m_Path;
	Toshi::T2SimpleArray<ATexture> m_TexturesArray;
	ATexture* m_pTextures;
	TINT m_iNumTextures;
	Toshi::TTRB m_TRB;
};