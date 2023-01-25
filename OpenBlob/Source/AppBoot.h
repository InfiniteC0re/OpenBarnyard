#pragma once
#include "Toshi/Core/TApplication.h"
#include "GameInterface/AGameStateController.h"
#include "Render/ARenderer.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() : m_Renderer(TNULL) { }
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate(float deltaTime) override;

	bool ShouldLoadModelViewState() const
	{
		return m_bLoadModelViewState;
	}

public:
	static AApplication g_oTheApp;

private:
	ARenderer* m_Renderer;                        // 0x34
	AGameStateController* m_pGameStateController; // 0x38
	bool m_bLoadModelViewState;                   // 0x63
};