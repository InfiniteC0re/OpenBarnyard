#pragma once

TOSHI_NAMESPACE_START

class T2Mutex
{
public:
	typedef TUINT32 Flags;
	enum Flags_ : Flags
	{
		Flags_Standard  = 0,
		Flags_DoNotWait = 1
	};

public:
	constexpr T2Mutex() :
	    m_Mutex( TNULL ) {}
	~T2Mutex() { Destroy(); }

	// Returns TTRUE if success
	TBOOL Create();

	// Returns TTRUE if the state is signaled
	TBOOL Lock( Flags flags = Flags_Standard );

	// Returns TTRUE if success
	TBOOL Unlock();

	// Destroys mutex
	void Destroy();

	operator TBOOL() const { return m_Mutex; }

private:
	void* m_Mutex;
};

TOSHI_NAMESPACE_END
