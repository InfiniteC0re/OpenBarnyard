#pragma once
#include "AXYZViewportManager.h"

#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/THPTimer.h>
#include <Toshi2/T2GUI/T2GUIRectangle.h>

class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	using t_MainScene = void(*)(float deltaTime, void* pCameraObject);

public:
	ARenderer();
	virtual ~ARenderer();

	void Update(float deltaTime);
	TBOOL CreateInterface();
	TBOOL CreateTRender();
	void Create();

	void SetBackgroundColour(uint32_t r, uint32_t g, uint32_t b);

private:
	void RenderLoadingScreen(float fDeltaTime);
	void RenderGUI(TBOOL allowBackgroundClear);
	void RenderImGui(TBOOL allowBackgroundClear);
	void RenderMainScene(float deltaTime, Toshi::TViewport* pViewport, Toshi::TCameraObject* pCameraObject, ACamera* pCamera, t_MainScene mainSceneCb, TBOOL bAllowBackgroundClear);
	void CreateMainViewport();
	void CreateTRenderResources();

	void GetAppCamera();
	void InitialiseViewPort();

private:
	static Toshi::TTRB s_BootAssetsTRB;
	static Toshi::THPTimer s_FrameTimer;

private:
	Toshi::TViewport* m_pViewport;           // 0x0C
	AXYZViewportManager* m_pViewportManager; // 0x10
	TBOOL m_bRenderGUI;                      // 0x44
	TBOOL m_bIsLoadingScreen;                // 0x46
	uint32_t m_BackgroundColor;              // 0x48
	Toshi::T2GUIRectangle* m_pRectangle;     // 0x60
	float m_fFrameTime;                      // 0x64
};