#pragma once
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Core/TArray.h"

namespace Toshi {
	
	class TInputDevice;

	class TInputInterface :
		public TGenericClassDerived<TInputInterface, TObject, "TInputInterface", TMAKEVERSION(1, 0), TFALSE>,
		public TSingleton<TInputInterface>
	{
	public:
		enum EVENT_TYPE
		{
			EVENT_TYPE_GONE_DOWN,
			EVENT_TYPE_GONE_UP,
			EVENT_TYPE_REPEAT,
			EVENT_TYPE_UNKNOWN,
			EVENT_TYPE_MOVED
		};

		class InputEvent
		{
		public:
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType);
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, wchar_t* a_wszString);
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, int a_iMagnitude1);
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, int a_iMagnitude1, int a_iMagnitude2);
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, float a_fMagnitude1);
			InputEvent(TInputDevice* a_pDevice, int a_iDoodad, EVENT_TYPE a_eEventType, float a_fMagnitude1, float a_fMagnitude2);

			int GetMagnitudeInt(int a_iAxis);

			float GetMagnitudeFloat(int a_iAxis);

			int8_t GetAxisCount() const
			{
				return m_iAxisCount;
			}

			int GetDoodad() const
			{
				return m_iDoodad;
			}

			EVENT_TYPE GetEventType() const
			{
				return m_eEventType;
			}

			TInputDevice* GetSource() const
			{
				return m_pSource;
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
			int m_iDoodad;             // 0x00
			EVENT_TYPE m_eEventType;   // 0x04
			TBOOL m_bIsMagnitudeFloat; // 0x08
			int8_t m_iAxisCount;       // 0x09
			TInputDevice* m_pSource;   // 0x0C
			wchar_t m_wszString[4];    // 0x10

			union
			{
				float Floats[2];
				int Ints[2];
			} m_Magnitude;             // 0x18 De blob 0x10 JPOG
		};

	public:
		TInputInterface() 
		{
			m_bIsExclusiveMode = TFALSE;
		}

		TInputDevice* GetDeviceByIndex(TClass* pClass, size_t index);

		template <class C>
		C* GetDeviceByIndex(size_t index = 0)
		{
			C* pDevice = TSTATICCAST(C*, GetDeviceByIndex(TGetClass(C), index));
			TASSERT(pDevice == TNULL || pDevice->GetClass()->IsA(TGetClass(C)));
			return pDevice;
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
		TNodeList<TInputDevice> m_DeviceList;                                        // 0x04
		TBOOL m_bIsExclusiveMode;                                                    // 0x14 
		TEmitter<TInputInterface, TInputInterface::InputEvent> m_Emitter1;           // 0x24
		TGenericEmitter m_Emitter2;                                                  // 0x28
	};

	class TInputDevice :
		public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), TFALSE>,
		public TNodeList<TInputDevice>::TNode
	{
	public:
		static constexpr int INPUT_DEVICE_MOUSE_BUTTONS = 3;
		static constexpr int INPUT_DEVICE_MOUSE_WHEEL = 4;

		struct DoodadProperties
		{
			int m_iUnk;
			TBOOL m_bFlag;
		};

		struct RepeatInfo
		{
			int m_iDoodad;
		};

	public:
		TInputDevice() :
			m_Repeats(0, 16),
			m_Array2(0, 16)
		{
			m_pInputInterface = TNULL;
			m_bUnknown = TFALSE;
			m_bIsAcquired = TFALSE;
			m_uiDeviceIndex = s_uiDeviceCount++;
			s_aDevices[m_uiDeviceIndex] = this;
		}

		virtual TBOOL Acquire() = 0;
		virtual TBOOL Unacquire() = 0;
		virtual void Release() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual TBOOL Flush() { return TTRUE; }
		virtual int ProcessEvents(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float deltaTime) = 0;
		virtual int GetButtonCount() const = 0;
		virtual int GetAxisCount() const = 0;
		virtual TBOOL GetDoodadProperties(int doodad, DoodadProperties& doodadProps) const = 0;
		virtual TBOOL StartRepeat(int param_1, float param_2, float param_3);
		virtual TBOOL StopRepeat(int param_1);
		virtual TBOOL StopAllRepeats();
		virtual TBOOL IsForceFeedbackDevice() { return TFALSE; }
		virtual Platform GetPlatform() const = 0;
		virtual const char* GetButtonFromDoodad(int a_iDoodad) const = 0;
		virtual TBOOL IsDown(int doodad) const = 0;
		virtual int GetAxisInt(int doodad, int axis) const = 0;
		virtual float GetAxisFloat(int doodad, int axis) const = 0;
		virtual TBOOL IsEnabled() const = 0;
		virtual void ThrowRepeatEvent(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, RepeatInfo* repeatInfo, float deltaTime);

		TBOOL IsAcquired() const
		{
			return m_bIsAcquired;
		}

		void SetAcquired(TBOOL a_bAcquired)
		{
			m_bIsAcquired = a_bAcquired;
		}

		TInputInterface* GetInputInterface()
		{
			return m_pInputInterface;
		}

		void SetInputInterface(TInputInterface* a_pInterface)
		{
			m_pInputInterface = a_pInterface;
		}

	protected:
		int ProcessRepeats(TEmitter<TInputInterface, TInputInterface::InputEvent>& emitter, float flt);

	public:
		static TInputDevice** GetRegisteredDevices()
		{
			return s_aDevices;
		}

		static size_t GetNumRegisteredDevices()
		{
			return s_uiDeviceCount;
		}

	protected:
		static constexpr size_t MAX_DEVICE_COUNT = 14;

		inline static TInputDevice* s_aDevices[MAX_DEVICE_COUNT];
		inline static size_t s_uiDeviceCount;

	protected:
		size_t m_uiDeviceIndex;             // 0x14
		TArray<void*>::Storage m_Repeats;   // 0x18 FIXME: replace void* with some structure whose size is 0xC
		TArray<void*>::Storage m_Array2;    // 0x28 FIXME: replace void* with some structure whose size is 0x4
		TBOOL m_bUnknown;                   // 0x38
		TBOOL m_bIsAcquired;                // 0x39 de blob 0x35 JPOG
		TInputInterface* m_pInputInterface; // 0x3C
	};

}
