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
	TFileStreamJob( TSemaphore* semaphore ) :
		m_pSemaphore( semaphore ), m_bIsProcessed( TTRUE ) {}

	virtual ~TFileStreamJob() = default;
	virtual void Process()	  = 0;

	TBOOL IsProcessed() const { return m_bIsProcessed; }

protected:
	TSemaphore* m_pSemaphore;
	TBOOL		m_bIsProcessed;
};

class TFileStream : public TThread
{
public:
	TFileStream() = default;

	// This method will be executed by the thread
	virtual void Main() override;

	virtual ~TFileStream() = default;

	// Adds job to the FIFO
	void AddStream( TFileStreamJob* job );

private:
	TFifo<TFileStreamJob*, 32> m_Jobs;
};

class TTRBStreamJob : public TFileStreamJob
{
public:
	TTRBStreamJob() :
		TFileStreamJob( TNULL )
	{
		m_trb			= TNULL;
		m_fileName[ 0 ] = '\0';
	}

	virtual ~TTRBStreamJob() = default;

	virtual void Process()
	{
		m_trb->Load( m_fileName );
	}

	void Init( TTRB* trb, const TCHAR* fileName )
	{
		m_trb = trb;
		T2String8::Copy( m_fileName, fileName, -1 );
	}

	TTRB*		 GetTRB() const { return m_trb; }
	const TCHAR* GetFileName() const { return m_fileName; }

public:
	TTRB* m_trb;
	TCHAR m_fileName[ 64 ];
};

TOSHI_NAMESPACE_END
