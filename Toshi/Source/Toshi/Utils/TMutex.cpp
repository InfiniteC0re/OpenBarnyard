#include "ToshiPCH.h"
#include "TMutex.h"

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

		return result != 0;
	}

	bool TMutex::Lock(uint32_t flags)
	{
		DWORD waitForMs = (flags & TMutexLockFlag_DoNotWait) ? 0 : INFINITE;
		DWORD result = WaitForSingleObject(m_Handle, waitForMs);

		return result == WAIT_OBJECT_0;
	}

	bool TMutex::Unlock()
	{
		BOOL result = ReleaseMutex(m_Handle);
		return result != 0;
	}
}