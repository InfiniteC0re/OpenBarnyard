#pragma once
#include <Toshi/Core/TTask.h>

class ARootTask : public Toshi::TTask
{
	TOSHI_CLASS_DEFINE(ARootTask)

public:
	ARootTask();
	~ARootTask();

	virtual bool OnUpdate(float deltaTime) override
	{
		m_Seconds += deltaTime;

		if (m_Seconds >= 1)
		{
			m_Seconds -= 1;
			TOSHI_INFO("ARootTask: A second has passed. Deactivating...");
			Activate(false);
		}

		return true;
	}

private:
	double m_Seconds = 0;
};