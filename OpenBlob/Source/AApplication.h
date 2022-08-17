#pragma once
#include <Toshi.h>

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate() override;
	virtual bool OnUpdate() override;
};