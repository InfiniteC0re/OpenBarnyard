#pragma once
#include <Thread/TThread.h>
#include <Thread/TMutexLock.h>

extern char			   g_szTexturePackName[ 64 ];
extern char			   g_szTexturePackPath[ MAX_PATH ];
extern Toshi::TMutex   g_LoadMutex;
extern TBOOL		   g_bDumpTextures;
extern TBOOL		   g_bAutoReload;
extern Toshi::THPTimer g_LastDumpTimer;

class ATextureUpdateWatcher :
	public Toshi::TThread
{
public:
	virtual void Main() override;

	void Init( const char* a_szPath );
	void Destroy();

private:
	const char* m_szPath;
	HANDLE		m_hFile;
	OVERLAPPED	m_Overlapped;
	TUINT8		m_ChangeBuf[ 1024 ];
	TBOOL		m_bDestroyed = TFALSE;
};

class ATextureUpdater :
	public Toshi::TSingleton<ATextureUpdater>
{
public:
	ATextureUpdater();

	static void DumpTexture( const Toshi::TString8& a_rFilePath, const void* a_pData, TUINT32 a_uiDataSize );
	static void ReloadTexture( Toshi::T2Texture* a_pT2Texture, void* a_pData, TUINT a_uiDataSize );
	static void RestoreTexture( const Toshi::TString8& a_rTextureName, TBOOL a_bForceDump = TFALSE );
	static void ReloadTexture( const Toshi::TString8& a_rTextureName );
	static void ReloadAllTextures();
};
