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

	TBOOL IsFinished() const { return m_bIsFinished; }

protected:
	TBOOL m_bIsFinished = TFALSE;
};
