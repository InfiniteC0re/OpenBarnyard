#pragma once
#include <Toshi/Core/TTask.h>

class ARootTask : public Toshi::TTask
{
	TOSHI_CLASS_DEFINE(ARootTask)

public:
	ARootTask();
	~ARootTask();

	virtual bool OnUpdate(float deltaTime) override;

private:
	double m_Seconds = 0;
};