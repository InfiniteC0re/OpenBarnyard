#pragma once
#include <Toshi/Core/TTask.h>

class ARootTask : public Toshi::TTask
{
	TOSHI_CLASS_DEFINE(ARootTask)

public:
	ARootTask();
	~ARootTask();
	
	inline void SetKernelInterface(Toshi::TKernelInterface* kernel) { m_Kernel = kernel; }
public:
	virtual bool OnUpdate(float deltaTime) override;

private:
	Toshi::TKernelInterface* m_Kernel = nullptr;
	double m_Seconds = 0;
};