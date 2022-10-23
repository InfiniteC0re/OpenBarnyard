#pragma once

namespace Toshi
{
	class T2Mutex
	{
	public:
		typedef uint32_t Flags;
		enum Flags_ : Flags
		{
			Flags_None      = 0,
			Flags_DoNotWait = 1
		};

	public:
		T2Mutex() = default;
		~T2Mutex() { Destroy(); }

		// Returns true if success
		bool Create();

		// Returns true if the state is signaled
		bool Lock(Flags flags = Flags_None);

		// Returns true if success
		bool Unlock();

		// Destroys mutex
		void Destroy();

	private:
		void* m_Mutex;
	};
}