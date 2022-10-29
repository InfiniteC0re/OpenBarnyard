#pragma once
#include <Toshi/Utils/TSingleton.h>

class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	ARenderer();
	virtual ~ARenderer();

	bool CreateInterface();
	bool CreateTRender();
};