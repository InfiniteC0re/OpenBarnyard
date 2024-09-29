#pragma once
#include <Toshi/T2DList.h>

class AMainThreadJob : public Toshi::T2DList<AMainThreadJob>::Node
{
public:
	friend class AAssetStreaming;

public:
	virtual ~AMainThreadJob() = default;
	virtual void  BeginJob()  = 0;
	virtual TBOOL RunJob()    = 0;
	virtual TBOOL CancelJob() { return TFALSE; }

	TBOOL IsRunning() const { return m_bIsRunning; }

protected:
	TBOOL m_bIsRunning = TFALSE;
};
