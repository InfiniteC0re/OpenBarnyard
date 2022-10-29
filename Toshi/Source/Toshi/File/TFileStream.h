#pragma once
#include "Toshi/Core/TFifo.h"

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

	class TFileStream
	{
	public:
		TFileStream()
		{
			m_Unk1 = (int*)&m_Unk1;
			m_Unk2 = (int*)&m_Unk1;
			m_Unk3 = 0;
			m_Unk4 = 0;
		}

		// This function has an infinite loop and tries to complete every job
		virtual void Main();

		// Adds job to the FIFO
		void AddStream(TFileStreamJob* job);

	private:
		int* m_Unk1;
		int* m_Unk2;
		int m_Unk3;
		int m_Unk4;
		TFifo<TFileStreamJob*, 32> m_Jobs;
	};
}