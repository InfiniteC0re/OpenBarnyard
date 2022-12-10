#pragma once
#include "Toshi/Core/TFifo.h"
#include "Toshi/Thread/TThread.h"

namespace Toshi
{
	class TFileStreamJob
	{
	public:
		friend class TFileStream;

	public:
		TFileStreamJob(TSemaphore* semaphore) : m_pSemaphore(semaphore), m_bIsProcessed(false) { }

		virtual ~TFileStreamJob() = 0;

		virtual void Process() = 0;

	private:
		TSemaphore* m_pSemaphore;
		bool m_bIsProcessed;
	};

	class TFileStream : public TThread
	{
	public:
		TFileStream() = default;
		
		// This method will be executed by the thread
		virtual void Main() override;

		// Adds job to the FIFO
		void AddStream(TFileStreamJob* job);

	private:
		TFifo<TFileStreamJob*, 32> m_Jobs;
	};
}