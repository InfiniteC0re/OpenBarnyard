#pragma once

typedef void* HANDLE;

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
		~TMutex();

		// Returns true if success
		bool Create();

		// Returns true if success
		bool Destroy();

		// Returns true if the state is signaled
		bool Lock(uint32_t flags = 0);

		// Returns true if success
		bool Unlock();

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