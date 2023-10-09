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

private:
	AOptions* m_pOptions;   // 0x20
	ARenderer* m_pRenderer; // 0x30
	TBOOL m_bRenderScene;   // 0x57
};
