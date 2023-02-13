#pragma once

class TGlow
{

public:

	TGlow()
	{
		m_fIntensity = 0.0f;
		m_fDist = 2.0f;
	}

	void SetIntensity(float a_fIntensity)
	{
		TASSERT(a_fIntensity >= 0.0f);
		m_fIntensity = a_fIntensity;
	}

	float GetIntensity() const
	{
		return m_fIntensity;
	}

	void SetDist(float a_fDist)
	{
		TASSERT(a_fDist >= 0.0f);
		m_fDist = a_fDist;
	}

	float GetDist() const
	{
		return m_fDist;
	}

private:
	float m_fIntensity; // 0x0
	float m_fDist;      // 0x4
};

