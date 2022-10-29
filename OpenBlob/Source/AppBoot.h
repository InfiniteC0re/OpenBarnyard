#pragma once
#include <Toshi.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
<<<<<<< HEAD
#include "Locale/ALocaleManager.h"
//#include <Toshi/Core/TMSWindow.h>
#include <Toshi/Render/TRender_DX11.h>
#include "../ALevelInformation.h"
=======
#include <Platform/Windows/TMSWindow.h>
#include "Render/ARenderer.h"
>>>>>>> b19878c8db5405a44256abff728c3dce985f73ec

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