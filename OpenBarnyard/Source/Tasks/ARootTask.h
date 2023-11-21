#pragma once
#include "AOptions.h"
#include "Render/ARenderer.h"

#include <Toshi/Core/TTask.h>
#include <Toshi/Strings/TString8.h>

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<ARootTask>
{
public:
	ARootTask();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	void CreateStartupGameStates();
	void LoadStartupData();

	TBOOL IsPaused() const;
	TBOOL IsGameSystemCreated() const { return m_bGameSystemCreated; }

private:
	AOptions* m_pOptions;          // 0x20
	TTask* m_pGUISystem;           // 0x24
	TTask* m_pGUI2;                // 0x28
	TTask* m_pInputHandler;        // 0x2C
	ARenderer* m_pRenderer;        // 0x30
	TTask* m_pGameStateController; // 0x34
	TTask* m_pMoviePlayer;         // 0x4C
	TTask* m_pSoundManager;        // 0x44
	TBOOL m_bLoadedTerrain;        // 0x54
	TBOOL m_bPaused;               // 0x55
	TBOOL m_bStopRenderingWorld;   // 0x56
	TBOOL m_bGameSystemCreated;    // 0x57
	TBOOL m_bStartedGame;
};
