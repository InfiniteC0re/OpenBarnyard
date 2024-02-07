#pragma once
#include "Thread/TSemaphore.h"

namespace Toshi
{
	class TGenericFifo
	{
	protected:
		typedef uint8_t Flags;
		enum Flags_ : Flags
		{
			Flags_None          = 0,
			Flags_PollSemaphore = BITFIELD(0),
			Flags_NoSemaphore   = BITFIELD(1),
		};

	protected:
		TGenericFifo() = default;

		TBOOL Create(char* a_pBuffer, int a_iMaxItems, int a_iItemSize);
		TBOOL Destroy();
		TBOOL Push(void* a_pItem, Flags a_iFlags);
		TBOOL Pop(void* a_pOut, Flags a_iFlags);

	private:
#ifdef TOSHI_SKU_WINDOWS
		CRITICAL_SECTION m_CriticalSection;
#endif
		TSemaphore m_Semaphore1;
		TSemaphore m_Semaphore2;
		int m_iItemSize;
		int m_iMaxItems;
		char* m_pDataBegin;
		char* m_pDataEnd;
		char* m_pDataPopCursor;
		char* m_pDataPushCursor;
	};

	template <class T, int MaxItems>
	class TFifo : public TGenericFifo
	{
	public:
		TFifo()
		{
			TGenericFifo::Create(m_Buffer, MaxItems, sizeof(T));
		}

		~TFifo()
		{
			TGenericFifo::Destroy();
		}

		TBOOL Push(T* element, Flags flags = TGenericFifo::Flags_None)
		{
			return TGenericFifo::Push(element, flags);
		}

		TBOOL Pop(T& out, Flags flags = TGenericFifo::Flags_None)
		{
			return TGenericFifo::Pop(&out, flags);
		}

	private:
		char m_Buffer[sizeof(T) * MaxItems];
	};
}