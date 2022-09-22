#pragma once
#include <Toshi/Core/TTask.h>

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), true>
{
public:
	ARootTask() = default;
	~ARootTask() = default;
	
	inline void SetKernelInterface(Toshi::TKernelInterface* kernel) { m_Kernel = kernel; }
public:
	virtual bool OnUpdate(float deltaTime) override;

private:
	Toshi::TKernelInterface* m_Kernel = nullptr;
	double m_Seconds = 0;
};