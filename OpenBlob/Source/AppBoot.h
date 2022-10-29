#pragma once
#include <Toshi.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include "Locale/ALocaleManager.h"
#include "./ALevelInformation.h"
#include "Render/ARenderer.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate() override;

private:
	ARenderer* m_Renderer; // 0x34
};