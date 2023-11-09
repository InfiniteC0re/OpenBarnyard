#pragma once
#include "ARenderer.h"
#include "AOptions.h"

#include <Toshi/Core/TTask.h>
#include <Toshi/Strings/TString8.h>

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<ARootTask>
{
public:
	ARootTask();

	virtual TBOOL OnCreate() override;

	void LoadStartupData();

private:
	AOptions* m_pOptions;   // 0x20
	TTask* m_pGUISystem;    // 0x24
	TTask* m_pGUI2;         // 0x28
	ARenderer* m_pRenderer; // 0x30
	TBOOL m_bRenderScene;   // 0x57
};
