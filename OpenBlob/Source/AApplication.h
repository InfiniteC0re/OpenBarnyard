#pragma once
#include <Toshi.h>
#include "Toshi/Sound/TSound.h"
//#include <Toshi/Core/Core.h>
//#include <Toshi/Math/TMathInline.h>
#include <Toshi/Render/TRender.h>
#include "AExampleClass.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate() override;
	virtual bool OnUpdate() override;

};