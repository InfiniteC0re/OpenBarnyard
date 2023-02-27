#pragma once
#include "Toshi/Core/TApplication.h"
#include "Memory/AMemory.h"




class AApplication : public Toshi::TApplication
{
public:

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate(float deltaTime) override;
	bool UpdateSound(float deltaTime);

	void ParseCommandLineParams(int argc, char** argv);
	void InitialiseFreeList();

	bool ShouldLoadModelViewState() const
	{
		return m_bLoadModelViewState;
	}

	void SetRenderWorld(bool render)
	{
		m_bRenderWorld = render;
	}

	bool ShouldRenderWorld() const
	{
		return m_bRenderWorld;
	}

	static Toshi::T2Allocator* AppStringAllocator()
	{
		return AMemory::ms_aMemoryAllocators;
	}

public:
	static AApplication g_oTheApp;

private:
	bool m_bRenderWorld;                          // 0x48
	bool m_bLoadModelViewState;                   // 0x63
};