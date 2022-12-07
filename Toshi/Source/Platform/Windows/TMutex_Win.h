#pragma once

namespace Toshi
{
	enum TMutexLockFlag
	{
		TMutexLockFlag_DoNotWait = 1,
	};

	class TMutex
	{
	public:
		TMutex() = default;

		~TMutex()
		{
			Destroy();
		}

		// Returns true if success
		bool Create()
		{
			m_Handle = CreateMutexA(NULL, FALSE, NULL);
			return true;
		}

		// Returns true if success
		bool Destroy()
		{
			BOOL result = CloseHandle(m_Handle);
			m_Handle = NULL;

			return result;
		}

		// Returns true if the state is signaled
		bool Lock(uint32_t flags = 0)
		{
			DWORD waitForMs = (flags & TMutexLockFlag_DoNotWait) ? 0 : INFINITE;
			DWORD result = WaitForSingleObject(m_Handle, waitForMs);

			return result == WAIT_OBJECT_0;
		}

		// Returns true if success
		bool Unlock()
		{
			return ReleaseMutex(m_Handle);
		}

	public:
		TMutex& operator = (const TMutex& mutex)
		{
			m_Handle = mutex.m_Handle;
			return *this;
		}

	private:
		HANDLE m_Handle;
	};
}