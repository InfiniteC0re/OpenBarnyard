#include "ToshiPCH.h"
#include "Toshi2/T2Mutex.h"

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
		return ReleaseMutex(m_Mutex) != FALSE;
	}

	void T2Mutex::Destroy()
	{
		CloseHandle(m_Mutex);
		m_Mutex = NULL;
	}
}