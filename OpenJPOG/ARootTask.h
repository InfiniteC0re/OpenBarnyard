#pragma once
#include <Toshi/Core/TTask.h>
#include <Toshi/Math/TRandom.h>

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), true>
{
public:
	ARootTask();
	~ARootTask() = default;

public:
	virtual bool OnUpdate(float deltaTime) override;

private:
	double m_Seconds = 0;

	Toshi::TRandom m_oRandom;
};