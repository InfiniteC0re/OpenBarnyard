#include "ARootTask.h"

TOSHI_CLASS_DERIVED_INITIALIZE(ARootTask, Toshi::TTask, MKVERSION(1, 1))

ARootTask::ARootTask()
{
	TOSHI_INFO("Called ARootTask");
}

ARootTask::~ARootTask()
{
	TOSHI_INFO("Called ~ARootTask");
}

bool ARootTask::OnUpdate(float deltaTime)
{
	m_Seconds += deltaTime;

	if (m_Seconds >= 1)
	{
		m_Seconds -= 1;
		TOSHI_INFO("ARootTask: A second has passed. Deactivating task...");
		Activate(false);
	}

	return true;
}
