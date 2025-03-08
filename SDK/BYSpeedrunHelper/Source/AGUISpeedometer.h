#pragma once
#include <BYardSDK/AGUI2FontManager.h>
#include <BYardSDK/AGUI2TextBox.h>

class AGUISpeedometer
{
public:
	AGUISpeedometer();
	~AGUISpeedometer();

	void Create();
	void Update();
	void Render();

	void ApplyUIStyle();
	void UpdateUIPosition( TFLOAT a_fY );

private:
	AGUI2TextBox* m_pTextBox = TNULL;
};
