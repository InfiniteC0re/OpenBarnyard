#pragma once

class AApplication : public Toshi::TApplication
{
public:
	AApplication() = default;
	~AApplication() { }

	virtual bool OnCreate() override;
	virtual bool OnUpdate() override;
};