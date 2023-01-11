#pragma once
#include <Toshi/Core/TTask.h>

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), true>
{
public:
	ARootTask() = default;
	~ARootTask() = default;
	
public:
	virtual bool OnUpdate(float deltaTime) override;

private:
	double m_Seconds = 0;
};