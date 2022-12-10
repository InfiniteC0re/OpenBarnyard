#pragma once

namespace Toshi
{
	class TSemaphore
	{
	public:
		TSemaphore() = default;

	public:
		/*
		* Platform specific methods
		* Define them in TThread_{Platform}.cpp
		*/

		bool Create(size_t a_iInitCount, size_t a_iMaxCount)
		{
			TASSERT(a_iInitCount >= 0, "Init count should be greater than zero");
			TASSERT(a_iMaxCount >= a_iInitCount, "Init count cannot be less than max count");

			m_hSemaHnd = CreateSemaphoreA(NULL, a_iInitCount, a_iMaxCount, NULL);
			TASSERT(m_hSemaHnd != NULL, "Unable to create semaphore");

			return true;
		}

		bool Destroy()
		{
			return false;
		}

		bool Signal()
		{
			TASSERT(m_hSemaHnd != NULL, "Handle is NULL");
			return ReleaseSemaphore(m_hSemaHnd, 1, NULL);
		}

		bool Wait()
		{
			return WaitForSingleObject(m_hSemaHnd, INFINITE) == WAIT_OBJECT_0;
		}

		bool Poll()
		{
			return WaitForSingleObject(m_hSemaHnd, 0) == WAIT_OBJECT_0;
		}

	private:
		HANDLE m_hSemaHnd;
	};
}