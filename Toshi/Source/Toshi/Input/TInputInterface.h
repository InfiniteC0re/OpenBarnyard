#pragma once
namespace Toshi
{
	class TInputInterface : TObject, TSingleton<TInputInterface>
	{

		class InputEvent
		{
		private:

			enum EventType
			{

			};

			int m_iDoodad; // 0x0
			EventType m_eEventType; // 0x4
			bool m_bIsMagnitudeFloat; // 0x8
			int8_t m_iAxisCount; // 0x9
			int* m_magnitude; // 0x10 JPOG 0x18 De blob
			TInputInterface* m_pSource; // 0xC
			

		public:
			inline int8_t GetAxisCount() const { return m_iAxisCount; }
			inline int GetDoodad() const { return m_iDoodad; }
			inline EventType GetEventType() const { return m_eEventType; }
			inline TInputInterface* GetSource() const { return m_pSource; }

			int GetMagnitudeInt(int a_iAxis)
			{
				TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount(), "");
				if (IsMagnitudeInt()) return m_magnitude[a_iAxis];
				if (m_magnitude[a_iAxis] < -0.5) return -1;
				return 0.5 < m_magnitude[a_iAxis];
			}

			float GetMagnitudeFloat(int a_iAxis)
			{
				TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount(), "");
				if (IsMagnitudeFloat()) return m_magnitude[a_iAxis];
				if (m_magnitude[a_iAxis] < -0.5) return -1;
				return 0.5 < m_magnitude[a_iAxis];
			}


			inline bool IsMagnitudeFloat() { return m_bIsMagnitudeFloat; }
			inline bool IsMagnitudeInt() { return !m_bIsMagnitudeFloat; }
		};
	};
}


