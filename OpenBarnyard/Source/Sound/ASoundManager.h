#pragma once
#include "AWaveBank.h"

#include <Toshi/Strings/TPString8.h>
#include <Toshi/Core/TTask.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi/Math/TVector4.h>
#include <Toshi2/T2Map.h>
#include <Toshi2/T2DList.h>
#include <Toshi2/T2SimpleArray.h>

class ASoundManager :
	public Toshi::TTask,
	public Toshi::TSingleton<ASoundManager>
{
public:
	TDECLARE_CLASS(Toshi::TTask);

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

public:
	ASoundManager();
	~ASoundManager();

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnDestroy() override;

	TBOOL LoadWaveBanks(const char* a_szFileName);
	void PauseAllSound(TBOOL a_bPaused);

private:
	TBOOL Initialise();

private:
	inline static Toshi::TFileSystem* ms_pFileSystem;
	inline static Toshi::T2Map<Toshi::TPString8, AWaveBank, Toshi::TPString8::Comparator> ms_WaveBanks;

private:
	CameraData m_CameraData;         // 0xC8
	S1 m_aS1[128];                   // 0x108
	S2 m_aS2[8];                     // 0x4D08
	S3 m_aS3[16];                    // 0x4E10
	TFLOAT m_fTotalTime;             // 0x4F5C
	TBOOL m_bUseMinHardwareChannels; // 0x4FA9
	TINT m_iMinHWChannels;           // 0x4FAC
	TINT m_iNumChannels;             // 0x4FB0
	PauseListener m_PauseListener;   // 0x4FB8
	Toshi::T2SimpleArray<S4> m_pS4;  // 0x4FCC
	Toshi::T2DList<S4> m_FreeListS4; // 0x4FD0
	Toshi::T2DList<S4> m_UnkList1;   // 0x4FD8
	Toshi::T2DList<S2> m_FreeListS2; // 0x4FE8
};
