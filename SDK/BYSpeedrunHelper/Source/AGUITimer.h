#pragma once
#include <BYardSDK/AGUI2FontManager.h>
#include <BYardSDK/AGUI2TextBox.h>

class AGUITimer
{
public:
	AGUITimer();
	~AGUITimer();

	void Create();
	void ApplyUIStyle();
	void UpdateUIPosition( TFLOAT a_fY );

	void Update();
	void Render();
	void EnablePrefix( const TWCHAR* a_wcsPrefix, TBOOL a_bEnable );

	void Reset() { SetTime( 0.0f ); }
	void SetTime( FLOAT a_fTime ) { m_fTotalTime = a_fTime; }

	TBOOL  IsValid() { return m_pTextBox; }

	TFLOAT GetTotalTime() const { return m_fTotalTime; }

	static void GetTime( TFLOAT a_fTime, TINT& a_iMilliseconds, TINT& a_iSeconds, TINT& a_iMinutes, TINT& a_iHours );

private:
	AGUI2TextBox* m_pTextBox;
	TFLOAT        m_fTotalTime;
	TWCHAR        m_wcsBuffer[ 48 ];
	TBOOL         m_bPrefixEnabled;
	const TWCHAR* m_wcsPrefix;
};
