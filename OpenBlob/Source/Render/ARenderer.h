#pragma once
#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/File/TTRB.h>

class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	using t_MainScene = void(*)(float deltaTime, void* pCameraObject);

public:
	ARenderer();
	virtual ~ARenderer();

	void Update(float deltaTime);
	bool CreateInterface();
	bool CreateTRender();
	void Create();

	void SetBackgroundColour(uint32_t r, uint32_t g, uint32_t b);

private:
	void RenderGUI(bool allowBackgroundClear);
	void RenderMainScene(float deltaTime, Toshi::TViewport* pViewport, void* unk2, void* pCameraObject, t_MainScene mainSceneCb, bool allowBackgroundClear);
	void CreateMainViewport();

private:
	static Toshi::TTRB s_BootAssetsTRB;

private:
	Toshi::TViewport* m_pViewport; // 0x0C
	uint32_t m_BackgroundColor;    // 0x48
};