#pragma once
#include <Toshi/Core/TSystem.h>
#include <Toshi2/T2DList.h>
#include <Toshi2/T2SimpleArray.h>

TOBJECT(ASoundManager, Toshi::TTask, TTRUE),
	public Toshi::TSingleton<ASoundManager>
{
public:
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

	using PauseListener = Toshi::TListener<Toshi::TSystemManager, TBOOL, ASoundManager>;

public:
	ASoundManager();
	~ASoundManager();

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnDestroy() override;

	void PauseAllSound(TBOOL a_bPaused);

private:
	TBOOL Initialise();

private:
	Toshi::TVector4 m_CameraPos;     // 0xC8
	Toshi::TVector4 m_SomeVelocity;  // 0xD8
	Toshi::TVector4 m_CameraForward; // 0xE8
	Toshi::TVector4 m_CameraUp;      // 0xF8
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
