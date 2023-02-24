#pragma once

namespace Toshi
{
	class T2GUITransform
	{
	public:
		static constexpr float FLOAT_QUALITY = 256;
		static constexpr float FLOAT_SCALE = 1 / FLOAT_QUALITY;

		using Float = int16_t;

		class Rotation
		{
		public:
			Rotation()
			{

			}
			Rotation(int16_t x, int16_t y) : m_X(x), m_Y(y)
			{

			}

			void SetX(float x)
			{
				m_X = PackFloat(x);
			}

			void SetY(float y)
			{
				m_Y = PackFloat(y);
			}

			float GetX() const
			{
				return UnpackFloat(m_X);
			}

			float GetY() const
			{
				return UnpackFloat(m_Y);
			}

		private:
			Float m_X;
			Float m_Y;
		};

	public:
		T2GUITransform() : m_Rot{ { PackFloat(1.0f), 0 }, { 0, PackFloat(1.0f) } }
		{

		}

		void Reset()
		{
			m_Rot[0] = { PackFloat(1.0f), 0 };
			m_Rot[1] = { 0, PackFloat(1.0f) };
		}

		void Rotate(float angle);
		void RotateTo(float angle);
		void PreMultiply(const T2GUITransform& transform);
		void PostMultiply(const T2GUITransform& transform);
		void Matrix44(TMatrix44& outMatrix);

		void Transform(TVector2& outVec, const TVector2& transformVec) const
		{
			outVec.x = m_Pos.x + m_Rot[0].GetX() * transformVec.x + m_Rot[1].GetX() * transformVec.y;
			outVec.y = m_Pos.y + m_Rot[0].GetY() * transformVec.x + m_Rot[1].GetY() * transformVec.y;
		}

		TVector2& GetPos()
		{
			return m_Pos;
		}

		static constexpr float UnpackFloat(Float value)
		{
			return value * FLOAT_SCALE;
		}

		static constexpr Float PackFloat(float value)
		{
			return static_cast<Float>(value * FLOAT_QUALITY);
		}

		static void Multiply(T2GUITransform& outTransform, const T2GUITransform& a, const T2GUITransform& b);

	public:
		Rotation m_Rot[2];
		TVector2 m_Pos;
	};
}