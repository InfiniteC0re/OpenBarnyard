#pragma once

TOSHI_NAMESPACE_START

class TMutex
{
public:
	enum FLAG
	{
		FLAG_NONE    = 0,
		FLAG_NO_WAIT = BITFLAG( 0 ),
	};

public:
	TMutex();
	~TMutex();

	// Returns TTRUE if success
	TBOOL Create();

	// Returns TTRUE if success
	TBOOL Destroy();

	// Returns TTRUE if the state is signaled
	TBOOL Lock( FLAG a_eFlags = FLAG_NONE );

	// Returns TTRUE if success
	TBOOL Unlock();

public:
	TMutex& operator=( const TMutex& mutex );
	TMutex& operator=( void* handle );

private:
	void* m_Handle;
};

TOSHI_NAMESPACE_END
