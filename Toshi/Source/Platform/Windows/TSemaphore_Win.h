#pragma once

TOSHI_NAMESPACE_START

class TSemaphore
{
public:
	TSemaphore() = default;
	~TSemaphore();

public:
	TBOOL Create( TINT a_iInitCount, TINT a_iMaxCount );
	TBOOL Destroy();
	TBOOL Signal();
	TBOOL Wait();
	TBOOL Poll();

private:
	void* m_hSemaHnd;
};

TOSHI_NAMESPACE_END
