#pragma once
#include "AUIVerticalStack.h"
#include "AGUISpeedometer.h"
#include "ARunTimer.h"

class AUIManager
    : public Toshi::TSingleton<AUIManager>
{
public:
	AUIManager();
	~AUIManager();

	void Create();
	void Destroy();
	void Update( TFLOAT a_fDeltaTime );
	void Render();

	void ApplyUIStyle();

	AUIVerticalStack& GetLeftSideList() { return m_LeftSideList; }
	ARunTimer&        GetTimer() { return m_Timer; }
	AGUISpeedometer&  GetSpeedometer() { return m_Speedometer; }

private:
	AUIVerticalStack m_LeftSideList;
	ARunTimer        m_Timer;
	AGUISpeedometer  m_Speedometer;
};
