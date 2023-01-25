#pragma once
#include "TInputDeviceKeyboard.h"

namespace Toshi
{
	class TInputInterface :
		public TGenericClassDerived<TInputInterface, TObject, "TInputInterface", TMAKEVERSION(1, 0), false>,
		public TSingleton<TInputInterface>
	{
	public:
		class InputEvent
		{
		private:
			enum EventType
			{

			};

		public:
			int8_t GetAxisCount() const
			{
				return m_iAxisCount;
			}

			int GetDoodad() const
			{
				return m_iDoodad;
			}

			EventType GetEventType() const
			{
				return m_eEventType;
			}

			TInputInterface* GetSource() const
			{
				return m_pSource;
			}

			int GetMagnitudeInt(int a_iAxis)
			{
				TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

				if (IsMagnitudeInt())
				{
					return m_Magnitude.Ints[a_iAxis];
				}
				else
				{
					if (m_Magnitude.Floats[a_iAxis] < -0.5f)
					{
						return -1;
					}

					return 0.5f < m_Magnitude.Floats[a_iAxis];
				}
			}

			float GetMagnitudeFloat(int a_iAxis)
			{
				TASSERT(a_iAxis >= 0 && a_iAxis < GetAxisCount());

				if (IsMagnitudeFloat())
				{
					return m_Magnitude.Floats[a_iAxis];
				}

				return m_Magnitude.Ints[a_iAxis];
			}

			bool IsMagnitudeFloat()
			{
				return m_bIsMagnitudeFloat;
			}

			bool IsMagnitudeInt()
			{
				return !m_bIsMagnitudeFloat;
			}

		public:
			int m_iDoodad;              // 0x0
			EventType m_eEventType;     // 0x4
			bool m_bIsMagnitudeFloat;   // 0x8
			int8_t m_iAxisCount;        // 0x9
			union
			{
				float* Floats;
				int* Ints;
			} m_Magnitude;              // 0x10 JPOG 0x18 De blob
			TInputInterface* m_pSource; // 0xC
		};

	public:
		TInputDevice* GetDeviceByIndex(TClass* pClass, size_t index);
		TInputDeviceKeyboard* GetKeyboardByIndex(size_t index);

	private:
		uint32_t m_Unk;                       // 0x04 
		TNodeList<TInputDevice> m_DeviceList; // 0x08
	};
}


