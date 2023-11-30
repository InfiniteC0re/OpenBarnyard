#include "pch.h"
#include "ASoundManager.h"
#include "Assets/AAssetLoader.h"

#include <fmod/fmod.h>

ASoundManager::ASoundManager()
{
	TIMPLEMENT();
	m_bUseMinHardwareChannels = TTRUE;
	m_iMinHWChannels = 32;
	m_iNumChannels = 32;

	ms_pFileSystem = Toshi::TFileManager::GetSingleton()->FindFileSystem("local");
}

ASoundManager::~ASoundManager()
{
	TIMPLEMENT();

	m_FreeListS2.Clear();

	FSOUND_Close();
	m_PauseListener.Disconnect();
	Reset();
}

TBOOL ASoundManager::Reset()
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL ASoundManager::OnCreate()
{
	Initialise();
	m_PauseListener.Connect(
		Toshi::TSystemManager::GetSingleton()->GetPauseEmitter(),
		this,
		[](ASoundManager* a_pSndMngr, Toshi::TSystemManager* a_pSysMngr, TBOOL* a_pPaused) {
			a_pSndMngr->PauseAllSound(*a_pPaused);
			return TTRUE;
		}
	, 0);

	m_pS4.Create(32);

	for (TINT i = 0; i < 32; i++)
	{
		m_FreeListS4.PushBack(&m_pS4[i]);
	}

	for (TINT i = 0; i < sizeof(m_aS2) / sizeof(*m_aS2); i++)
	{
		m_FreeListS2.PushBack(&m_aS2[i]);
	}

	return TTRUE;
}

TBOOL ASoundManager::OnUpdate(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();
	m_fTotalTime += a_fDeltaTime;

	FSOUND_Update();
	return TTRUE;
}

void ASoundManager::OnDestroy()
{
	m_UnkList1.Clear();
	m_FreeListS4.Clear();
	m_pS4.Destroy();
	m_PauseListener.Disconnect();
}

TBOOL ASoundManager::Initialise()
{
	FSOUND_Init(44100, m_iMinHWChannels + m_iNumChannels, 0);

	TINT num2DC, num3DC, numDC;
	FSOUND_GetNumHWChannels(&num2DC, &num3DC, &numDC);
	FSOUND_GetMaxChannels();
	FSOUND_Close();

	TBOOL bRes = FSOUND_SetMinHardwareChannels(m_bUseMinHardwareChannels ? m_iMinHWChannels : 0);
	FSOUND_Init(44100, m_iNumChannels, 0);
	FSOUND_GetNumHWChannels(&num2DC, &num3DC, &numDC);
	FSOUND_GetMaxChannels();
	FSOUND_Stream_SetBufferSize(2000);
	FSOUND_SetBufferSize(100);
	FSOUND_3D_SetDistanceFactor(1.0f);
	FSOUND_3D_SetRolloffFactor(1.0f);
	FSOUND_3D_SetDopplerFactor(1.0f);

	return bRes;
}

void ASoundManager::PauseAllSound(TBOOL a_bPaused)
{
	TIMPLEMENT();
}

TBOOL ASoundManager::LoadWaveBanks(const char* a_szFileName)
{
	TBOOL bOpened = AAssetLoader::Load(
		"Data/Assets/lib_wavebank.trb",
		AAssetType_WaveBank,
		TTRUE
	);

	if (!bOpened) return TFALSE;

	TIMPLEMENT();
	//AAssetLoader::CastSymbol()

	return TTRUE;
}
