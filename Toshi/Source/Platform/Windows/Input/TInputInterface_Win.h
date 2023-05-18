#pragma once
#include "Includes.h"
#include "Toshi/Input/TInputInterface.h"

namespace Toshi
{
	class TInputDXInterface : public TInputInterface
	{
	private:

		LPDIRECTINPUT8 GetDirectInput() const
		{
			return m_poDirectInput8;
		}

	public:

		TInputDXInterface() : TInputInterface()
		{
			m_poDirectInput8 = NULL;
			m_hMainWindow = NULL;
			ms_pTheInterface = this;
			m_bExclusive = false;
		}

		virtual bool Initialise();
		virtual bool Deinitialise();
		virtual void RefreshDirect() {}

		HWND GetMainWindow() const
		{
			return m_hMainWindow;
		}

		void SetMainWindow(HWND a_hMainWindow)
		{
			m_hMainWindow = a_hMainWindow;
		}

		static TInputDXInterface* GetInterface()
		{
			return ms_pTheInterface;
		}

		bool LostDevice();

		static BOOL CALLBACK EnumerateDeviceCallback(LPCDIDEVICEINSTANCE a_poDeviceInstance, LPVOID poDXInputInterface);

		static inline TInputDXInterface* ms_pTheInterface = TNULL;

		LPDIRECTINPUT8 m_poDirectInput8; // 0x30
		HWND m_hMainWindow;              // 0x34
		bool m_bExclusive;               // 0x38
	};
}
