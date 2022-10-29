#include "ToshiPCH.h"
#include "Toshi/Core/TSemaphore.h"
#include "Toshi/Utils/TMutex.h"
#include "Toshi2/T2Mutex.h"

// TSemaphore
namespace Toshi
{
	bool TSemaphore::Create(size_t a_iInitCount, size_t a_iMaxCount)
	{
		TASSERT(a_iInitCount >= 0, "Init count should be greater than zero");
		TASSERT(a_iMaxCount >= a_iInitCount, "Init count cannot be less than max count");
		
		m_hSemaHnd = CreateSemaphoreA(NULL, a_iInitCount, a_iMaxCount, NULL);
		TASSERT(m_hSemaHnd != NULL, "Unable to create semaphore");

		return true;
	}

	bool TSemaphore::Destroy()
	{
		return false;
	}

	bool TSemaphore::Signal()
	{
		TASSERT(m_hSemaHnd != NULL, "Handle is NULL");
		return ReleaseSemaphore(m_hSemaHnd, 1, NULL);
	}

	bool TSemaphore::Wait()
	{
		return WaitForSingleObject(m_hSemaHnd, INFINITE) == WAIT_OBJECT_0;
	}

	bool TSemaphore::Poll()
	{
		return WaitForSingleObject(m_hSemaHnd, 0) == WAIT_OBJECT_0;
	}
}

// TMutex
namespace Toshi
{
	TMutex::~TMutex()
	{
		Destroy();
	}

	bool TMutex::Create()
	{
		m_Handle = CreateMutexA(NULL, FALSE, NULL);
		return true;
	}

	bool TMutex::Destroy()
	{
		BOOL result = CloseHandle(m_Handle);
		m_Handle = NULL;

		return result;
	}

	bool TMutex::Lock(uint32_t flags)
	{
		DWORD waitForMs = (flags & TMutexLockFlag_DoNotWait) ? 0 : INFINITE;
		DWORD result = WaitForSingleObject(m_Handle, waitForMs);

		return result == WAIT_OBJECT_0;
	}

	bool TMutex::Unlock()
	{
		return ReleaseMutex(m_Handle);
	}
}

// T2Mutex
namespace Toshi
{
	bool T2Mutex::Create()
	{
		Destroy();
		m_Mutex = CreateMutexA(NULL, FALSE, NULL);
		TASSERT(m_Mutex != NULL, "Failed to create mutex");
		return true;
	}

	bool T2Mutex::Lock(Flags flags)
	{
		TASSERT(m_Mutex != NULL, "Mutex is NULL");

		DWORD waitForMs = (flags & Flags_DoNotWait) ? 0 : INFINITE;
		DWORD result = WaitForSingleObject(m_Mutex, waitForMs);

		return result == WAIT_OBJECT_0;
	}

	bool T2Mutex::Unlock()
	{
		TASSERT(m_Mutex != NULL, "Mutex is NULL");
		return ReleaseMutex(m_Mutex);
	}

	void T2Mutex::Destroy()
	{
		CloseHandle(m_Mutex);
		m_Mutex = NULL;
	}
}