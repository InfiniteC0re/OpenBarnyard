#pragma once
#include <Toshi/Input/TInputDeviceKeyboard.h>
#include "TInputDeviceMouse.h"

namespace Toshi
{


	class TInputInterface :
		public TGenericClassDerived<TInputInterface, TObject, "TInputInterface", TMAKEVERSION(1, 0), TFALSE>,
		public TSingleton<TInputInterface>
	{
	public:
		class InputEvent
		{
		public:
			enum EventType
			{
				EventType_Unk,
				EventType_Unk2,
				EventType_Repeat,
				EventType_Unk3,
				EventType_MouseMotion
			};

		public:
			InputEvent() = default;
			InputEvent(TInputDevice* device, int doodad, EventType eventType)
			{
				m_pSource = device;
				m_iDoodad = doodad;
				m_eEventType = eventType;
				m_bIsMagnitudeFloat = TFALSE;
				m_iAxisCount = 0;
			}

			InputEvent(TInputDevice* device, int doodad, EventType eventType, float magnitude)
			{
				m_pSource = device;
				m_iDoodad = doodad;
				m_eEventType = eventType;
				m_Magnitude.Floats[0] = magnitude;
				m_bIsMagnitudeFloat = TTRUE;
				m_iAxisCount = 1;
			}

			InputEvent(TInputDevice* device, int doodad, EventType eventType, float magnitude, float magnitude2)
			{
				m_pSource = device;
				m_iDoodad = doodad;
				m_eEventType = eventType;
				m_Magnitude.Floats[0] = magnitude;
				m_Magnitude.Floats[1] = magnitude2;
				m_bIsMagnitudeFloat = TTRUE;
				m_iAxisCount = 2;
			}

			InputEvent(TInputDevice* device, int doodad, EventType eventType, int magnitude)
			{
				m_pSource = device;
				m_iDoodad = doodad;
				m_eEventType = eventType;
				m_Magnitude.Ints[0] = magnitude;
				m_bIsMagnitudeFloat = TFALSE;
				m_iAxisCount = 1;
			}

			InputEvent(TInputDevice* device, int doodad, EventType eventType, int magnitude, int magnitude2)
			{
				m_pSource = device;
				m_iDoodad = doodad;
				m_eEventType = eventType;
				m_Magnitude.Ints[0] = magnitude;
				m_Magnitude.Ints[1] = magnitude2;
				m_bIsMagnitudeFloat = TFALSE;
				m_iAxisCount = 2;
			}

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

			TInputDevice* GetSource() const
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

				return (float)m_Magnitude.Ints[a_iAxis];
			}

			TBOOL IsMagnitudeFloat()
			{
				return m_bIsMagnitudeFloat;
			}

			TBOOL IsMagnitudeInt()
			{
				return !m_bIsMagnitudeFloat;
			}

		public:
			int m_iDoodad;              // 0x0
			EventType m_eEventType;     // 0x4
			TBOOL m_bIsMagnitudeFloat;   // 0x8
			int8_t m_iAxisCount;        // 0x9
			union
			{
				float* Floats;
				int* Ints;
			} m_Magnitude;              // 0x10 JPOG 0x18 De blob
			TInputDevice* m_pSource; // 0xC
		};

	public:

		TInputInterface() 
		{
			m_bIsExclusiveMode = TFALSE;
		}

		TInputDevice* GetDeviceByIndex(TClass* pClass, size_t index);
		TInputDeviceKeyboard* GetKeyboardByIndex(size_t index);
		TInputDeviceMouse* GetMouseByIndex(size_t index);

		TClass* GetSpecificDeviceByIndex(TClass* pClass, size_t index)
		{
			auto pDevice = GetDeviceByIndex(pClass, index);
			TASSERT(pDevice == TNULL || pDevice->GetClass()->IsA(pClass));
			if (pDevice == TNULL) return TNULL;
			return pDevice->GetClass();
		}

		void AddDevice(TInputDevice* device);
		void RemoveDevice(TInputDevice* device);

		virtual TBOOL Initialise() { return TTRUE; }
		virtual TBOOL Deinitialise();

		virtual TBOOL AcquireAll();
		virtual TBOOL UnacquireAll();
		virtual TBOOL FlushAll();
		virtual void SetExclusiveMode(TBOOL mode);
		virtual TBOOL GetExclusiveMode() const;
		virtual int ProcessEvents(float fUnk);
		virtual void StopAllRepeats();



	private:
		TBOOL m_bIsExclusiveMode;              // 0x04 
		TNodeList<TInputDevice> m_DeviceList; // 0x08
		TGenericEmitter m_Emitter1;           // 0x24
		TGenericEmitter m_Emitter2;           // 0x28
	};
}


