#include "pch.h"
#include <AImGUI.h>
#include <ModLoader.h>
#include <AHooks.h>

#include <BYardSDK/AGUI2.h>
#include <BYardSDK/THookedRenderD3DInterface.h>

#include <Toshi/Core/THPTimer.h>
#include <Toshi/File/TFile.h>

#include TOSHI_MULTIRENDER(TTextureResourceHAL)

TOSHI_NAMESPACE_USING

#define TEXTURE_PACK_PATH "Data\\TextureOverride\\"

TMutex g_DumpMutex;
TINT g_iNumDumpEvents = 0;
TBOOL g_bDumpTextures = TTRUE;

TBOOL MaterialLibrary_LoadTTLData(AMaterialLibrary* a_pMatLib, AMaterialLibrary::TTL* a_pTTLData)
{
	TMutexLock lock(g_DumpMutex);

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
					TString8 dumpFilePath = TEXTURE_PACK_PATH;
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
							TFree(pFileData);
						}
						else
						{
							TASSERT(!"Couldn't read texture file: {0}", dumpFilePath);
						}

						pFile->Destroy();
					}
					else
					{
						void* pData = TMalloc(pTexInfo->m_uiDataSize);

						TUtil::MemCopy(pData, pTexInfo->m_pData, pTexInfo->m_uiDataSize);
						pTexture->SetData(pData, pTexInfo->m_uiDataSize);
						pTexture->Load();

						if (g_bDumpTextures)
						{
							// Create directory for the dump
							TINT iSlashPos = 0;
							while (iSlashPos = dumpFilePath.Find('\\', iSlashPos), iSlashPos >= 0)
							{
								dumpFilePath[iSlashPos] = '\0';
								CreateDirectoryA(dumpFilePath, NULL);
								dumpFilePath[iSlashPos] = '\\';
								iSlashPos++;
							}

							pFile = TFile::Create(dumpFilePath, TFile::FileMode_Write | TFile::FileMode_CreateNew);

							if (pFile)
							{
								// Dump texture
								pFile->Write(pData, pTexInfo->m_uiDataSize);
								pFile->Destroy();

								g_iNumDumpEvents += 2;
							}

							pTexture->SetData(TNULL, 0);
							TFree(pData);
						}
					}
				}

				a_pMatLib->m_pTextures[i].pTexture = pTexture;
			}
		}
	}

	return TTRUE;
}

void ReloadTexture(Toshi::T2Texture* a_pT2Texture, void* a_pData, TUINT a_uiDataSize)
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

void ReloadTexture(const Toshi::TString8& a_rTextureName)
{
	auto pAppTexture = AMaterialLibraryManager::List::GetSingleton()->FindTexture(a_rTextureName, TNULL, TNULL);

	if (pAppTexture)
	{
		TString8 texFilePath = TEXTURE_PACK_PATH;
		texFilePath.Concat(pAppTexture->Name, a_rTextureName.Length() - 3);
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
	}
}

void ReloadAllTextures()
{
	auto pLibraries = &AMaterialLibraryManager::List::GetSingleton()->m_Libraries;

	for (auto it = pLibraries->Begin(); it != pLibraries->End(); it++)
	{
		for (TINT i = 0; i < it->GetNumTextures(); i++)
		{
			auto pAppTexture = &it->m_TexturesArray[i];

			TString8 texFilePath = TEXTURE_PACK_PATH;
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
		}
	}
}

class ATexturePackWatcher : public Toshi::TThread
{
public:
	virtual void Main() override
	{
		m_Overlapped.hEvent = CreateEvent(NULL, FALSE, 0, NULL);

		ReadDirectoryChangesW(
			m_hFile, m_ChangeBuf, 1024, TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			NULL, &m_Overlapped, NULL
		);

		while (TTRUE)
		{
			DWORD result = WaitForSingleObject(m_Overlapped.hEvent, 0);

			if (result == WAIT_OBJECT_0)
			{
				TMutexLock lock(g_DumpMutex);
				DWORD bytes_transferred;
				GetOverlappedResult(m_hFile, &m_Overlapped, &bytes_transferred, FALSE);

				FILE_NOTIFY_INFORMATION* event = (FILE_NOTIFY_INFORMATION*)m_ChangeBuf;

				while (TTRUE)
				{
					DWORD name_len = event->FileNameLength / sizeof(wchar_t);

					if (g_iNumDumpEvents == 0)
					{
						switch (event->Action)
						{
						case FILE_ACTION_ADDED:
						case FILE_ACTION_MODIFIED:
						case FILE_ACTION_RENAMED_NEW_NAME:
						{
							static char sFileName[MAX_PATH];
							TStringManager::StringUnicodeToChar(sFileName, event->FileName, name_len);
							
							TString8 texName = sFileName;

							if (texName.Find(".dds", name_len - 4) != -1)
							{
								texName[name_len - 3] = 't';
								texName[name_len - 2] = 'g';
								texName[name_len - 1] = 'a';

								Sleep(50);
								ReloadTexture(texName);
							}
						}
						}
					}
					else
					{
						g_iNumDumpEvents -= 1;
					}

					// Are there more events to handle?
					if (event->NextEntryOffset)
					{
						*((uint8_t**)&event) += event->NextEntryOffset;
					}
					else
					{
						break;
					}
				}

				// Queue the next event
				ReadDirectoryChangesW(
					m_hFile, m_ChangeBuf, 1024, TRUE,
					FILE_NOTIFY_CHANGE_FILE_NAME |
					FILE_NOTIFY_CHANGE_DIR_NAME |
					FILE_NOTIFY_CHANGE_LAST_WRITE,
					NULL, &m_Overlapped, NULL
				);
			}
		}
	}

	void Init(const char* a_szPath)
	{
		m_szPath = a_szPath;

		m_hFile = CreateFileA(
			m_szPath,
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL
		);
	}
	
private:
	const char* m_szPath;
	HANDLE m_hFile;
	OVERLAPPED m_Overlapped;
	TUINT8 m_ChangeBuf[1024];
};

class ABYTexturePacks : public AModInstance
{
public:
	TBOOL OnLoad() override
	{
		m_pWatcher = new ATexturePackWatcher();
		m_pWatcher->Init("Data\\TextureOverride");
		m_pWatcher->Create(32, TThread::THREAD_PRIORITY_IDLE, 0);

		return AHooks::AddHook(Hook_MaterialLibrary_LoadTTLData, HookType_Before, MaterialLibrary_LoadTTLData);
	}

	TBOOL OnUpdate(TFLOAT a_fDeltaTime) override
	{
		return TTRUE;
	}

	void OnUnload() override
	{
		
	}

	void OnRenderInterfaceReady(Toshi::TRenderD3DInterface* a_pRenderInterface) override
	{
		TRenderInterface::SetSingletonExplicit(
			THookedRenderD3DInterface::GetSingleton()
		);
	}

	void OnImGuiRender() override
	{
		ImGui::Checkbox("Dump Textures", &g_bDumpTextures);

		if (ImGui::Button("Reload Textures"))
		{
			ReloadAllTextures();
		}
	}
	
	TBOOL HasSettingsUI() override
	{
		return TTRUE;
	}

	const char* GetName() override
	{
		return "ABYTexturePacks";
	}

private:
	ATexturePackWatcher* m_pWatcher;
};

extern "C"
{
	MODLOADER_EXPORT AModInstance* CreateModInstance()
	{
		TMemory::Initialise(64 * 1024 * 1024, 0);
		TFileManager::Create();
		TLog::Create("ABYTexturePacks");

		g_DumpMutex.Create();

		return new ABYTexturePacks();
	}
}
