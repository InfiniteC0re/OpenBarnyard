#pragma once
#include <BYardSDK/AGUI2FontManager.h>
#include <BYardSDK/AGUI2TextBox.h>

class AGUITimer
{
public:
	AGUITimer();
	~AGUITimer();

	void Create();

	void Update();
	void Render();

	void Show() { m_bIsVisible = TTRUE; }
	void Hide() { m_bIsVisible = TFALSE; }
	void Reset() { m_fTotalTime = 0.0f; }

	void SetTime( FLOAT a_fTime ) { m_fTotalTime = a_fTime; }

	TBOOL& IsVisible()  { return m_bIsVisible; }
	TBOOL IsValid()     { return m_pTextBox; }

	TFLOAT GetTotalTime() const { return m_fTotalTime; }

	static void GetTime( TFLOAT a_fTime, TINT& a_iMilliseconds, TINT& a_iSeconds, TINT& a_iMinutes, TINT& a_iHours );

private:
	AGUI2TextBox* m_pTextBox;
	TFLOAT m_fTotalTime;
	TBOOL m_bIsVisible;
};
