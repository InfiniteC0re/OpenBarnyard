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

		bool Create(size_t a_iInitCount, size_t a_iMaxCount);
		bool Destroy();
		bool Signal();
		bool Wait();
		bool Poll();

	private:
		void* m_hSemaHnd;
	};
}