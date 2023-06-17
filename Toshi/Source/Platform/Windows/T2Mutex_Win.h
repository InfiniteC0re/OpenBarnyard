#pragma once

namespace Toshi
{
	class T2Mutex
	{
	public:
		typedef uint32_t Flags;
		enum Flags_ : Flags
		{
			Flags_Standard = 0,
			Flags_DoNotWait = 1
		};

	public:
		T2Mutex() = default;
		~T2Mutex() { Destroy(); }

		// Returns TTRUE if success
		TBOOL Create();

		// Returns TTRUE if the state is signaled
		TBOOL Lock(Flags flags = Flags_Standard);

		// Returns TTRUE if success
		TBOOL Unlock();

		// Destroys mutex
		void Destroy();

	private:
		void* m_Mutex;
	};
}