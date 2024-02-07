#include "pch.h"
#include "ATextureUpdater.h"

#include <AHooks.h>
#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedSystemManager.h>

#include <Toshi/TScheduler.h>

#include "Platform/DX8/TTextureResourceHAL_DX8.h"

TOSHI_NAMESPACE_USING

char g_szTexturePackName[64] = "Default";
char g_szTexturePackPath[MAX_PATH] = "Mods\\TexturePacks\\Default\\";
TMutex g_LoadMutex;
TBOOL g_bDumpTextures = TFALSE;
TBOOL g_bAutoReload = TTRUE;
THPTimer g_LastDumpTimer;

TBOOL MaterialLibrary_LoadTTLData(AMaterialLibrary* a_pMatLib, AMaterialLibrary::TTL* a_pTTLData)
{
	TMutexLock lock(g_LoadMutex);

	auto pTTL = TSTATICCAST(AMaterialLibrary::TTL*, a_pTTLData);

	auto pLibList = AMaterialLibraryManager::List::GetSingleton();
	TINT iNumTextures = 0;

	if (AMaterialLibrary::ms_bSkipLoadedTextures)
	{
		for (TINT i = 0; i < pTTL->m_iNumTextures; i++)
		{
			if (!pLibList->FindTexture(pTTL->m_pTextureInfos[i].m_szFileName, TNULL, TNULL))
			{
				iNumTextures++;
			}
		}
	}
	else
	{
		iNumTextures = pTTL->m_iNumTextures;
	}

	a_pMatLib->m_TexturesArray.Create(iNumTextures);
	a_pMatLib->m_pTextures = a_pMatLib->m_TexturesArray.GetArray();
	a_pMatLib->m_iNumTextures = iNumTextures;

	for (TINT i = 0; i < iNumTextures; i++)
	{
		auto pTexInfo = &pTTL->m_pTextureInfos[i];

		if (!AMaterialLibrary::ms_bSkipLoadedTextures || !pLibList->FindTexture(pTexInfo->m_szFileName, TNULL, TNULL))
		{
			TASSERT(pTexInfo->m_bIsT2Texture == TRUE, "No support of other texture types");
			a_pMatLib->m_pTextures[i].Name = pTexInfo->m_szFileName;

			if (pTexInfo->m_bIsT2Texture == TRUE)
			{
				auto pTexture = new T2Texture;

				if (pTexture)
				{
					TString8 dumpFilePath = g_szTexturePackPath;
					dumpFilePath.Concat(pTexInfo->m_szFileName, TStringManager::String8Length(pTexInfo->m_szFileName) - 3);
					dumpFilePath += "dds";

					auto pFile = TFile::Create(dumpFilePath, TFile::FileMode_Read);

					if (pFile)
					{
						// Load texture from Data\\TextureOverride
						auto uiFileSize = pFile->GetSize();
						void* pFileData = TMalloc(uiFileSize);

						if (pFile->Read(pFileData, uiFileSize) == uiFileSize)
						{
							pTexture->SetData(pFileData, uiFileSize);
							pTexture->Load();
							pTexture->SetData(TNULL, 0);
						}
						else
						{
							TASSERT(!"ERROR: Couldn't read texture file: {0}", dumpFilePath);
						}

						pFile->Destroy();
						TFree(pFileData);
					}
					else
					{
						pTexture->SetData(pTexInfo->m_pData, pTexInfo->m_uiDataSize);
						pTexture->Load();
						pTexture->SetData(TNULL, 0);

						if (g_bDumpTextures)
						{
							ATextureUpdater::DumpTexture(dumpFilePath, pTexInfo->m_pData, pTexInfo->m_uiDataSize);
						}
					}
				}

				a_pMatLib->m_pTextures[i].pTexture = pTexture;
			}
		}
	}

	return TTRUE;
}

ATextureUpdater::ATextureUpdater()
{
	AHooks::AddHook(Hook_MaterialLibrary_LoadTTLData, HookType_Before, MaterialLibrary_LoadTTLData);
}

void ATextureUpdater::DumpTexture(const Toshi::TString8& a_rFilePath, const void* a_pData, TUINT32 a_uiDataSize)
{
	TINT iSlashPos = 0;
	Toshi::TString8 filepath = a_rFilePath;

	// Create directory for the dump
	while (iSlashPos = filepath.Find('\\', iSlashPos), iSlashPos >= 0)
	{
		filepath[iSlashPos] = '\0';
		CreateDirectoryA(filepath, NULL);
		filepath[iSlashPos] = '\\';
		iSlashPos++;
	}

	auto pFile = TFile::Create(filepath, TFile::FileMode_Write | TFile::FileMode_CreateNew);

	if (pFile)
	{
		// Dump texture
		g_LastDumpTimer.Reset();
		pFile->Write(a_pData, a_uiDataSize);
		pFile->Destroy();
	}
}

void ATextureUpdater::ReloadTexture(Toshi::T2Texture* a_pT2Texture, void* a_pData, TUINT a_uiDataSize)
{
	auto pTexFactory = TSTATICCAST(
		TTextureFactory*,
		THookedRenderD3DInterface::GetSingleton()->GetSystemResource(TRenderInterface::SYSRESOURCE_TEXTUREFACTORY)
	);

	auto pOldD3DTexture = a_pT2Texture->GetD3DTexture();
	a_pT2Texture->Unload();

	a_pT2Texture->SetData(a_pData, a_uiDataSize);
	a_pT2Texture->Load();

	for (TINT k = 0; k < TTextureFactory::NUM_LISTS; k++)
	{
		auto pList = pTexFactory->GetList(k);

		for (auto pEntry = pList->Begin(); pEntry != pList->End(); pEntry++)
		{
			auto pTex = (Toshi::TTextureResourceHAL*)pEntry->GetTexture();

			if (pTex->GetD3DTexture() == pOldD3DTexture)
			{
				pTex->SetD3DTexture(a_pT2Texture->GetD3DTexture());
			}
		}
	}
}

void ATextureUpdater::RestoreTexture(const Toshi::TString8& a_rTextureName, TBOOL a_bForceDump)
{
	AMaterialLibrary* pLibrary;
	auto pAppTexture = AMaterialLibraryManager::List::GetSingleton()->FindTexture(a_rTextureName, &pLibrary, TNULL);

	TTRB trb;

	if (trb.Load(pLibrary->m_Path) == TTRB::ERROR_OK)
	{
		auto pTTL = trb.CastSymbol<AMaterialLibrary::TTL>("TTL");

		for (TINT i = 0; i < pTTL->m_iNumTextures; i++)
		{
			auto pTexInfo = &pTTL->m_pTextureInfos[i];

			if (pTexInfo->m_bIsT2Texture == TRUE &&
				a_rTextureName == pTexInfo->m_szFileName)
			{
				ReloadTexture(pAppTexture->pTexture, pTexInfo->m_pData, pTexInfo->m_uiDataSize);

				if (g_bDumpTextures || a_bForceDump)
				{
					TString8 dumpFilePath = g_szTexturePackPath;
					dumpFilePath.Concat(a_rTextureName, a_rTextureName.Length() - 3);
					dumpFilePath += "dds";

					DumpTexture(dumpFilePath, pTexInfo->m_pData, pTexInfo->m_uiDataSize);
				}

				pAppTexture->pTexture->SetData(TNULL, 0);
				break;
			}
		}
	}
}

void ATextureUpdater::ReloadTexture(const Toshi::TString8& a_rTextureName)
{
	TString8 texFilePath = g_szTexturePackPath;
	texFilePath.Concat(a_rTextureName, a_rTextureName.Length() - 3);
	texFilePath += "dds";

	if (Toshi::TFile* pFile = TFile::Create(texFilePath, TFile::FileMode_Read))
	{
		// Load texture data from raw file
		auto uiFileSize = pFile->GetSize();
		void* pFileData = TMalloc(uiFileSize);

		if (pFile->Read(pFileData, uiFileSize) == uiFileSize)
		{
			auto pLibraries = &AMaterialLibraryManager::List::GetSingleton()->m_Libraries;

			for (auto pLibrary = pLibraries->Begin(); pLibrary != pLibraries->End(); pLibrary++)
			{
				for (TINT i = 0; i < pLibrary->GetNumTextures(); i++)
				{
					auto pAppTexture = &pLibrary->m_TexturesArray[i];

					if (pAppTexture->Name == a_rTextureName)
					{
						// Load texture data from raw file
						ReloadTexture(pAppTexture->pTexture, pFileData, uiFileSize);
						pAppTexture->pTexture->SetData(TNULL, 0);
					}
				}
			}
		}
		else
		{
			TASSERT(!"ERROR: Couldn't read texture file: {0}", texFilePath);
		}

		TFree(pFileData);
		pFile->Destroy();
	}
	else
	{
		// Restore original texture from the material library
		RestoreTexture(a_rTextureName);
	}
}

void ATextureUpdater::ReloadAllTextures()
{
	if (!AMaterialLibraryManager::List::IsSingletonCreated())
	{
		return;
	}

	auto pLibraries = &AMaterialLibraryManager::List::GetSingleton()->m_Libraries;

	for (auto it = pLibraries->Begin(); it != pLibraries->End(); it++)
	{
		for (TINT i = 0; i < it->GetNumTextures(); i++)
		{
			auto pAppTexture = &it->m_TexturesArray[i];

			TString8 texFilePath = g_szTexturePackPath;
			texFilePath.Concat(pAppTexture->Name, TStringManager::String8Length(pAppTexture->Name) - 3);
			texFilePath += "dds";

			if (Toshi::TFile* pFile = TFile::Create(texFilePath, TFile::FileMode_Read))
			{
				auto uiFileSize = pFile->GetSize();
				void* pFileData = TMalloc(uiFileSize);

				if (pFile->Read(pFileData, uiFileSize) == uiFileSize)
				{
					ReloadTexture(pAppTexture->pTexture, pFileData, uiFileSize);
					pAppTexture->pTexture->SetData(TNULL, 0);
				}
				else
				{
					TASSERT(!"Couldn't read texture file: {0}", texFilePath);
				}

				TFree(pFileData);
				pFile->Destroy();
			}
			else
			{
				RestoreTexture(pAppTexture->Name);
			}
		}
	}
}
