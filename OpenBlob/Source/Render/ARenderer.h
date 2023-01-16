#pragma once
#include <Toshi/Utils/TSingleton.h>

class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	ARenderer();
	virtual ~ARenderer();

	void Update(float deltaTime);

	bool CreateInterface();
	bool CreateTRender();
};