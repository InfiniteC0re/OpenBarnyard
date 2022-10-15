#pragma once
namespace Toshi
{
	class TMatrix44
	{
		float m_f11, m_f12, m_f13, m_f14,
			m_f21, m_f22, m_f23, m_f24,
			m_f31, m_f32, m_f33, m_f34,
			m_f41, m_f42, m_f43, m_f44;

	public:
		TMatrix44(float a_f11, float a_f12, float a_f13, float a_f14,
			float a_f21, float a_f22, float a_f23, float a_f24,
			float a_f31, float a_f32, float a_f33, float a_f34,
			float a_f41, float a_f42, float a_f43, float a_f44);
		TMatrix44(const TMatrix44& a_rMatrix);
		TMatrix44() {};

		inline void Set(float a_f11, float a_f12, float a_f13, float a_f14,
			float a_f21, float a_f22, float a_f23, float a_f24,
			float a_f31, float a_f32, float a_f33, float a_f34,
			float a_f41, float a_f42, float a_f43, float a_f44);
	};
}
