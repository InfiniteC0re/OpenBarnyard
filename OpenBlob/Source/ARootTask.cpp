#include "pch.h"
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

	if (m_Seconds >= 5)
	{
		TOSHI_INFO("TVector:");
		Toshi::TVector<float> v;
		v.push_back(0.1f);
		v.push_back(0.5f);
		v.push_back(0.9f);

		TOSHI_INFO("TVector's size: {0}", v.size());
		TOSHI_INFO("TVector's elements:");

		for (auto& f : v)
		{
			TOSHI_INFO(f);
		}

		TOSHI_INFO("ARootTask: Five seconds has passed. Deactivating ARootTask");
		TOSHI_INFO("Average FPS is {0}", m_Kernel->GetAverageFPS());
		Activate(false);
		m_Seconds = 0;
	}

	return true;
}
