#pragma once
#include <Toshi/File/TFileStream.h>
#include <Toshi2/T2DList.h>

class AMainThreadJob2 :
	public Toshi::T2DList<AMainThreadJob2>::Node
{
public:
	friend class AAssetStreaming;

public:
	virtual ~AMainThreadJob2() = default;
	virtual void BeginJob() = 0;
	virtual TBOOL RunJob() = 0;
	virtual TBOOL CancelJob() { return TFALSE; }

protected:
	TBOOL m_bIsRunning;
	Toshi::TTRBStreamJob m_oStreamJob;
};