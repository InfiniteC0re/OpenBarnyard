#pragma once
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TViewport.h>

class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	ARenderer();
	virtual ~ARenderer();

	void Update(float deltaTime);
	bool CreateInterface();
	bool CreateTRender();

	void SetBackgroundColour(uint32_t r, uint32_t g, uint32_t b);

private:
	Toshi::TViewport* m_pViewport; // 0x0C
	uint32_t m_BackgroundColor;    // 0x48
};