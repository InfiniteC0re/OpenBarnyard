#pragma once
#include "AExampleClass.h"

#include <Toshi.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include "Locale/ALocaleManager.h"
#include <Toshi/Core/TMSWindow.h>
#include <Toshi/Render/TRender_DX11.h>

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate() override;

};