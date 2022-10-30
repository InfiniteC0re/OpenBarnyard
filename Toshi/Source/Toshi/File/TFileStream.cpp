#include "ToshiPCH.h"
#include "TFileStream.h"

namespace Toshi
{
	void TFileStream::Main()
	{
		TFileStreamJob* pJob;

		do {
			do {
				m_Jobs.Pop(&pJob);
				pJob->Process();
				pJob->m_bIsProcessed = true;
			} while (pJob->m_pSemaphore == nullptr);

			pJob->m_pSemaphore->Signal();

		} while (true);
	}

	void TFileStream::AddStream(TFileStreamJob* job)
	{
		job->m_bIsProcessed = false;
		m_Jobs.Push(&job);
	}
}