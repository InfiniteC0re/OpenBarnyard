#pragma once
#include "Thread/TSemaphore.h"

#if defined( TOSHI_SKU_WINDOWS ) && !defined( BARNYARD_COMMUNITY_PATCH )
#  define USE_WIN32_CRITICAL_SECTION
#  include <windows.h>
#else // TOSHI_SKU_WINDOWS && !BARNYARD_COMMUNITY_PATCH
#  include "Thread/T2AtomicMutex.h"
#endif // !TOSHI_SKU_WINDOWS || BARNYARD_COMMUNITY_PATCH

TOSHI_NAMESPACE_START

class TGenericFifo
{
protected:
	typedef uint8_t Flags;
	enum Flags_ : Flags
	{
		Flags_None          = 0,
		Flags_PollSemaphore = BITFLAG( 0 ),
		Flags_NoSemaphore   = BITFLAG( 1 ),
	};

protected:
	TGenericFifo() = default;
	~TGenericFifo() = default;

	TBOOL Create( TCHAR* a_pBuffer, TINT a_iMaxItems, TINT a_iItemSize );
	TBOOL Destroy();
	TBOOL Push( void* a_pItem, Flags a_iFlags );
	TBOOL Pop( void* a_pOut, Flags a_iFlags );

private:
#if defined( USE_WIN32_CRITICAL_SECTION )
	CRITICAL_SECTION m_CriticalSection;
#else // USE_WIN32_CRITICAL_SECTION
	T2AtomicMutex m_Mutex;
#endif // !USE_WIN32_CRITICAL_SECTION

	TSemaphore m_semNumFreeItems;
	TSemaphore m_semNumUsedItems;
	TINT       m_iItemSize;
	TINT       m_iMaxItems;
	TCHAR*     m_pDataBegin;
	TCHAR*     m_pDataEnd;
	TCHAR*     m_pDataPopCursor;
	TCHAR*     m_pDataPushCursor;
};

template <class T, TINT MaxItems>
class TFifo : public TGenericFifo
{
public:
	TFifo()
	{
		TGenericFifo::Create( m_Buffer, MaxItems, sizeof( T ) );
	}

	~TFifo()
	{
		TGenericFifo::Destroy();
	}

	TBOOL Push( T* element, Flags flags = TGenericFifo::Flags_None )
	{
		return TGenericFifo::Push( element, flags );
	}

	TBOOL Pop( T& out, Flags flags = TGenericFifo::Flags_None )
	{
		return TGenericFifo::Pop( &out, flags );
	}

private:
	TCHAR m_Buffer[ sizeof( T ) * MaxItems ];
};

TOSHI_NAMESPACE_END
