#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TApplication.h>

#include "Tasks/ADummyTask.h"
#include "Tasks/ARootTask.h"

class AApplication : public Toshi::TApplication
{
public:
	// $Barnyard: FUNCTION 00403ff0
	AApplication() {}
	~AApplication();

	virtual TBOOL OnCreate( int argc, TCHAR** argv ) OVERRIDE;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original doesn't override OnUpdate. The community patch uses it to limit the frame rate (see AppBoot.cpp)
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
#endif

public:
	ARootTask*  m_pRootTask;
	ADummyTask* m_pInputTask;
	ADummyTask* m_pUpdate1Task;
	ADummyTask* m_pUpdate2Task;
	ADummyTask* m_pUpdate3Task;
	ADummyTask* m_pRenderTask;
};

extern MODAPI AApplication g_oTheApp;
