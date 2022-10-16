#pragma once
#include <Toshi.h>
#include "Toshi/Sound/TSound.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate() override;
	virtual bool OnUpdate() override;

	static Toshi::TSound m_soundSystem;
};