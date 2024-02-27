#pragma once
#include "ATexture.h"

#include <File/TTRB.h>

#include <Toshi/T2SimpleArray.h>
#include <Toshi/T2DList.h>

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/T2Texture_DX8.h"
#endif // TOSHI_SKU_WINDOWS

class AMaterialLibrary :
	public Toshi::T2DList<AMaterialLibrary>::Node
{
public:
	struct TTL
	{
		struct TexInfo
		{
			BOOL m_bIsT2Texture;
			const TCHAR* m_szFileName;
			TUINT m_uiDataSize;
			void* m_pData;
		};

		TINT m_iNumTextures;
		TexInfo* m_pTextureInfos;
		const TCHAR* m_szPackName;
	};

public:
	AMaterialLibrary();

	TBOOL LoadTTLData(void* a_pTTLData);
	TBOOL LoadTTLFile(const TCHAR* a_szFilePath);
	TBOOL LoadTRBFile(Toshi::TTRB* a_pTRB);

	void Destroy();

	TINT FindTextureIndex(const TCHAR* a_szTextureName);

	TINT GetNumTextures() const
	{
		return m_iNumTextures;
	}

	ATexture* GetTexture(TINT a_uiIndex)
	{
		TASSERT(a_uiIndex < m_iNumTextures);
		return m_pTextures + a_uiIndex;
	}

	void SetPath(const TCHAR* a_szFilePath)
	{
		m_Path = a_szFilePath;
	}

private:
	void DestroyTextures();

private:
	inline static TBOOL ms_bSkipLoadedTextures = TFALSE;

private:
	Toshi::TString8 m_Path;
	Toshi::T2SimpleArray<ATexture> m_TexturesArray;
	ATexture* m_pTextures;
	TINT m_iNumTextures;
	Toshi::TTRB m_TRB;
};