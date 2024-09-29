#pragma once
#include "Toshi/TFifo.h"
#include "Thread/TThread.h"
#include "File/TTRB.h"

TOSHI_NAMESPACE_START

class TFileStreamJob
{
public:
	friend class TFileStream;

public:
	TFileStreamJob( TSemaphore* semaphore )
	    : m_pSemaphore( semaphore ), m_bIsProcessed( TTRUE ) {}

	virtual ~TFileStreamJob() = default;
	virtual void Process()    = 0;

	TBOOL IsProcessed() const { return m_bIsProcessed; }

protected:
	TSemaphore* m_pSemaphore;
	TBOOL       m_bIsProcessed;
};

class TFileStream : public TThread
{
public:
	TFileStream();

	// This method will be executed by the thread
	virtual void Main() override;

	virtual ~TFileStream();

	// Adds job to the FIFO
	void AddStream( TFileStreamJob* job );

private:
	TFifo<TFileStreamJob*, 32> m_Jobs;
};

class TTRBStreamJob : public TFileStreamJob
{
public:
	TTRBStreamJob();
	virtual ~TTRBStreamJob();

	virtual void Process();

	void Init( TTRB* trb, const TCHAR* fileName );

	TTRB*        GetTRB() const;
	const TCHAR* GetFileName() const;

public:
	TTRB* m_trb;
	TCHAR m_fileName[ 64 ];
};

TOSHI_NAMESPACE_END
