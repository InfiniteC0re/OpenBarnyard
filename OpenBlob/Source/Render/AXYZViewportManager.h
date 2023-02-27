#pragma once

#include "Toshi/Render/TViewport.h"


class AXYZViewportManager
{
public:

	enum Viewport
	{
		VIEWPORT_NONE = -1,

	};

	AXYZViewportManager();
	~AXYZViewportManager() = default;

	Toshi::TViewport* CreateViewport(float x, float y, float w, float h);

	void GetUseableArea(float& x, float& y, float& w, float& h);

	void SetMainFullScreen(bool fullscreen);

	void SetMargins(int left, int right, int top, int bottom)
	{
		m_iMarginLeft = left;
		m_iMarginRight = right;
		m_iMarginTop = top;
		m_iMarginBottom = bottom;
	}

private:
	int m_iMarginLeft;
	int m_iMarginRight;
	int m_iMarginTop;
	int m_iMarginBottom;
};

