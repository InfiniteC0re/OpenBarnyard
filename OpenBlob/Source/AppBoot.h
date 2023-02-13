#pragma once
#include "Toshi/Core/TApplication.h"
#include "GameInterface/AGameStateController.h"
#include "Render/ARenderer.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() : m_Renderer(TNULL), m_bRenderWorld(TTRUE) { }
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate(float deltaTime) override;
	bool UpdateSound(float deltaTime);

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

public:
	static AApplication g_oTheApp;

public:
	ARenderer* m_Renderer;                        // 0x34
	AGameStateController* m_pGameStateController; // 0x38
	bool m_bRenderWorld;                          // 0x48
	uint32_t m_Width;                             // 0x58
	uint32_t m_Height;                            // 0x5C
	bool m_bUseDefaultHeightWidth;                                  // 0x60
	bool m_bIsFullscreen;                         // 0x61
	bool m_bUnk2;                                 // 0x60
	bool m_bLoadModelViewState;                   // 0x63
};