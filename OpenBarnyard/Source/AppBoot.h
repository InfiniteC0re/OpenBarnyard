#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TApplication.h>

#include "Tasks/ADummyTask.h"
#include "Tasks/ARootTask.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual TBOOL OnCreate(int argc, TCHAR** argv) override;
	virtual TBOOL OnUpdate(float deltaTime) override;
	
public:
	ARootTask* m_pRootTask;
	ADummyTask* m_pInputTask;
	ADummyTask* m_pUpdate1Task;
	ADummyTask* m_pUpdate2Task;
	ADummyTask* m_pUpdate3Task;
	ADummyTask* m_pRenderTask;
};

extern AApplication g_oTheApp;
