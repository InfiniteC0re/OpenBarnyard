#pragma once
#include <Toshi/T2DList.h>

class AMainThreadJob : public Toshi::T2DList<AMainThreadJob>::Node
{
public:
	friend class AAssetStreaming;

public:
	// $Barnyard: FUNCTION 00401290
	// $Barnyard: FUNCTION 004012a0
	virtual ~AMainThreadJob() {}                 // +0x00
	virtual void  BeginJob() = 0;                // +0x04
	virtual TBOOL RunJob()   = 0;                // +0x08
	virtual TBOOL CancelJob() { return TFALSE; } // +0x0C, RETURNS_FALSE 00660960

	TBOOL IsFinished() const { return m_bIsFinished; }

protected:
	TBOOL m_bIsFinished = TFALSE;
};
