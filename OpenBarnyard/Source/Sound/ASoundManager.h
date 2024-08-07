#pragma once
#include "AWaveBank.h"
#include "ASoundBank.h"

#include <Toshi/TPString8.h>
#include <Toshi/TTask.h>
#include <Toshi/TSystem.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2DList.h>

#include <File/TFile.h>

#include <Math/TVector4.h>

class ASoundManager :
	public Toshi::TTask,
	public Toshi::TSingleton<ASoundManager>
{
public:
	TDECLARE_CLASS(ASoundManager, Toshi::TTask);

	struct S1 : public Toshi::T2DList<S1>::Node
	{

	};

	struct S2 : public Toshi::T2DList<S2>::Node
	{

	};

	struct S3 : public Toshi::T2DList<S3>::Node
	{

	};

	struct S4 : public Toshi::T2DList<S4>::Node
	{

	};

	struct CameraData
	{
		Toshi::TVector4 Position = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Velocity = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Forward = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Up = Toshi::TVector4::VEC_ZERO;
	};

	using PauseListener = Toshi::TListener<Toshi::TSystemManager, TBOOL, ASoundManager>;

	static constexpr const TCHAR* SOUNDS_BASE_DIRECTORY = "Data/Sound/";

	friend AWaveBank;

public:
	ASoundManager();
	~ASoundManager();

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnDestroy() override;

	void PauseAllSound(TBOOL a_bPaused);

	//-----------------------------------------------------------------------------
	// Wavebanks
	// ----------------------------------------------------------------------------
	// They are storing information about different packs of files (usually .fsb
	// if FMOD is being used) and their samples. Samples store technical info about
	// flags, frequency and length making sure the sounds from soundbanks can get
	// this information when neccessary.
	//-----------------------------------------------------------------------------

	// Returns pointer to a loaded wavebank or TNULL if not found
	AWaveBank* FindWaveBank( const Toshi::TPString8& a_rcName );

	// Loads info about all wavebanks from a specified PProperties file
	TBOOL LoadWaveBanksInfo( const TCHAR* a_szFileName );

	// Loads all samples (f.e. from fsb file) of a specified bank
	TBOOL LoadWaveBankSamples( const Toshi::TPString8& a_rcName, AWaveBank::LOADFLAGS a_eLoadFlags, TINT a_iBufferSize );

	//-----------------------------------------------------------------------------
	// Soundbanks
	// ----------------------------------------------------------------------------
	// Unlike wavebanks, soundbanks store info about how each sound should be
	// played by the engine. Such info includes volume, pitch, various
	// randomizations and much much more...
	//-----------------------------------------------------------------------------
	// The sounds can be stored either with ASound or ASoundEx class.
	// ASound class doesn't store any advanced settings for the sound.
	// ASoundEx class allows sounds to have different volume, pitch and more.
	//-----------------------------------------------------------------------------

	// Returns pointer to a loaded soundbank or TNULL if not found
	ASoundBank* FindSoundBank( const Toshi::TPString8& a_rcName );

	// Loads soundbank if it's not loaded yet.
	// If a_bSimpleSound is TFALSE, sounds are created from ASoundEx allowing many settings to be set from the properties file.
	// Returns TTRUE if was (now) loaded.
	TBOOL LoadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately );

	// Initialises sounds of the sound bank
	void LoadSoundBankSamples( const Toshi::TPString8& a_rcName );

public:
	// Whether ALoadScreen should be updated while loading audio stream or not
	inline static TBOOL ms_bShouldUpdateLoadingScreen = TFALSE;

private:
	TBOOL Initialise();

	AWaveBank* LoadWaveBankFromAsset( const Toshi::TString8& a_strName, TUINT32 a_uiForcedFlags );
	AWaveBank* AllocateWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strLibrary, const Toshi::TPString8& a_strType, const Toshi::TPString8& a_strPath );

	TBOOL LoadSoundBankImpl( const TCHAR* a_szName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately );

private:
	inline static Toshi::TFileSystem* ms_pFileSystem;
	inline static Toshi::T2Map<Toshi::TPString8, AWaveBank*, Toshi::TPString8::Comparator> ms_WaveBanks;

private:
	Toshi::T2Map<Toshi::TPString8, TSIZE, Toshi::TPString8::Comparator> m_CategoryIndices; // 0x80
	Toshi::T2Map<TINT, ASoundEx*> m_SoundIdToSoundEx;                                      // 0x80
	Toshi::T2Map<TINT, ASound*> m_SoundIdToSound;                                          // 0x80
	CameraData m_CameraData;                                                               // 0xC8
	S1 m_aS1[128];                                                                         // 0x108
	S2 m_aS2[8];                                                                           // 0x4D08
	S3 m_aS3[16];                                                                          // 0x4E10
	TFLOAT m_fTotalTime;                                                                   // 0x4F5C
	TBOOL m_bUseMinHardwareChannels;                                                       // 0x4FA9
	TINT m_iMinHWChannels;                                                                 // 0x4FAC
	TINT m_iNumChannels;                                                                   // 0x4FB0
	TUINT m_uiGlobalFrequency;                                                             // 0x4FB4
	PauseListener m_PauseListener;                                                         // 0x4FB8
	S4* m_pS4;                                                                             // 0x4FCC
	Toshi::T2DList<S4> m_FreeListS4;                                                       // 0x4FD0
	Toshi::T2DList<S4> m_UnkList1;                                                         // 0x4FD8
	Toshi::T2DList<ASoundBank> m_SoundBanks;                                               // 0x4FE0
	Toshi::T2DList<S2> m_FreeListS2;                                                       // 0x4FE8
};
