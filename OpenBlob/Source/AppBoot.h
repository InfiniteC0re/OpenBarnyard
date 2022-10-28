#pragma once
#include "AExampleClass.h"
#include "Locale/ALocaleManager.h"

#include <Toshi.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Platform/Windows/TMSWindow.h>

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate() override;

};