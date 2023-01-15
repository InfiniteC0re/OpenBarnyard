#pragma once
#include "Locale/ALocaleManager.h"
#include "Render/ARenderer.h"
#include "ALevelInformation.h"

#include <Toshi/Render/TRender.h>

class AApplication : public Toshi::TApplication
{
public:
	AApplication() : m_Renderer(TNULL) { }
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate(float deltaTime) override;

private:
	ARenderer* m_Renderer; // 0x34
};