#pragma once
#include <BYardSDK/AGUI2FontManager.h>
#include <BYardSDK/AGUI2TextBox.h>

class AGUITimer
{
public:
	AGUITimer();
	~AGUITimer();

	void Create();

	void Update(TFLOAT a_fDeltaTime);
	void Render();

	void Start() { m_bIsStopped = TFALSE; }
	void Stop() { m_bIsStopped = TTRUE; }
	void Show() { m_bIsVisible = TTRUE; }
	void Hide() { m_bIsVisible = TFALSE; }
	void Reset() { m_fTotalTime = 0.0f; }

	TFLOAT GetTotalTime() const { return m_fTotalTime; }
	TBOOL IsVisible() const { return m_bIsVisible; }
	TBOOL IsStopped() const { return m_bIsStopped; }

private:
	AGUI2TextBox* m_pTextBox;
	TFLOAT m_fTotalTime;
	TBOOL m_bIsVisible;
	TBOOL m_bIsStopped;
};
