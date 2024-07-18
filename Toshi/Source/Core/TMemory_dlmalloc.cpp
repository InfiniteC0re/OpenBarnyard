#include "ToshiPCH.h"
#include "dlmalloc.h"
#include "TMemory_dlmalloc.h"

TOSHI_NAMESPACE_START

void TMemoryDL::OutOfMem( TMemoryDLHeap* heap, TSIZE size )
{
	// 006fc7c0
	TERROR( "=========================================================================" );
	TERROR( "MEMORY ALLOCATION FAILED" );
	TERROR( "------------------------" );
	TERROR( "HEAP: {0}", heap->m_Name );
	TERROR( "------------------------" );
	TERROR( "failed to allocate {0} bytes (~{1}KB, ~{2}MB)", size, size >> 10, size >> 20 );
	TERROR( "=========================================================================" );
	// ...
}

void* TMemoryDL::dlheapmalloc( TMemoryDLHeap* heap, TSIZE size )
{
	// 006fc520
	if ( heap->m_Flags & TMemoryHeapFlags_AllocAsPile )
	{

		return TMemoryDLHeap::AllocAsPile( heap, size, 4 );
	}

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::AcquireMutex();

	void* chunk = mspace_malloc( heap->m_MSpace, size );
	if ( chunk == TNULL ) TMemoryDL::OutOfMem( heap, size );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::ReleaseMutex();
	return chunk;
}

void* TMemoryDL::dlheapcalloc( TMemoryDLHeap* heap, TSIZE nitems, TSIZE size )
{
	// 006fc580
	if ( heap->m_Flags & TMemoryHeapFlags_AllocAsPile )
	{
		return TMemoryDLHeap::AllocAsPile( heap, nitems * size, 4 );
	}

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::AcquireMutex();

	void* chunk = mspace_calloc( heap->m_MSpace, nitems, size );
	if ( chunk == TNULL ) TMemoryDL::OutOfMem( heap, size );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::ReleaseMutex();
	return chunk;
}

void* TMemoryDL::dlheapmemalign( TMemoryDLHeap* heap, TSIZE alignment, TSIZE size )
{
	// 006fc6f0
	if ( heap->m_Flags & TMemoryHeapFlags_AllocAsPile )
	{
		return TMemoryDLHeap::AllocAsPile( heap, size, 4 );
	}

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::AcquireMutex();

	void* chunk = mspace_memalign( heap->m_MSpace, alignment, size );
	if ( chunk == TNULL ) TMemoryDL::OutOfMem( heap, size );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::ReleaseMutex();
	return chunk;
}

void* TMemoryDL::dlheaprealloc( TMemoryDLHeap* heap, void* mem, TSIZE newsize )
{
	// 006fc5f0
	TASSERT( ( heap->m_Flags & TMemoryHeapFlags_AllocAsPile ) == 0, "Cannot realloc on pile" );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::AcquireMutex();

	void* chunk = mspace_realloc( heap->m_MSpace, mem, newsize );
	if ( chunk == TNULL ) TMemoryDL::OutOfMem( heap, newsize );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::ReleaseMutex();
	return chunk;
}

void TMemoryDL::dlheapfree( TMemoryDLHeap* heap, void* mem )
{
	TASSERT( ( heap->m_Flags & TMemoryHeapFlags_AllocAsPile ) == 0, "Cannot free pile memory" );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::AcquireMutex();

	mspace_free( heap->m_MSpace, mem );

	if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) TMemoryDL::ReleaseMutex();
}

void TMemoryDL::dlheapdestroy( TMemoryDLHeap* heap )
{
	if ( heap != TNULL )
	{
		if ( heap->m_Flags & TMemoryHeapFlags_UseMutex ) heap->DestroyMutex();
		destroy_mspace( heap->m_MSpace );
		heap->m_MSpace = TNULL;

		if ( heap->m_SubHeapBuffer != TNULL )
		{
			TFree( heap->m_SubHeapBuffer );
			heap->m_SubHeapBuffer = TNULL;
		}
	}
}

TMemoryDLHeap* TMemoryDL::dlheapcreatesubheap( TMemoryDLHeap* heap, TSIZE size, TMemoryDLHeapFlags flags, const char name[ HEAP_MAXNAME ] )
{
	// 006fc320
	TMemoryDLHeap* subHeap = TNULL;
	TSIZE subHeapSize = size + sizeof( TMemoryDLHeap );
	void* mem = heap->Malloc( subHeapSize );

	if ( mem != TNULL )
	{
		if ( flags & TMemoryHeapFlags_AllocAsPile )
		{
			subHeap = static_cast<TMemoryDLHeap*>( mem );

			subHeap->m_Flags = flags;
			subHeap->m_MSpace = TNULL;
			subHeap->m_PileData = reinterpret_cast<char*>( subHeap + 1 );
			subHeap->m_PileSize = subHeapSize;
			subHeap->SetName( name );

			if ( flags & TMemoryHeapFlags_UseMutex )
			{
				subHeap->CreateMutex();
			}
			else
			{
				TUtil::MemSet( &subHeap->m_Mutex, 0, sizeof( subHeap->m_Mutex ) );
			}
		}
		else
		{
			subHeap = TMemoryDL::CreateHeapInPlace( mem, subHeapSize, flags, name );
		}


		if ( subHeap != TNULL )
		{
			subHeap->m_SubHeapBuffer = static_cast<char*>( mem );
		}
		else
		{
			TFree( mem );
		}
	}

	return subHeap;
}

TMemoryDLHeap* TMemoryDL::dlheapcreateinplace( void* ptr, TSIZE heapSize, TMemoryDLHeapFlags flags, const char name[ HEAP_MAXNAME ] )
{
	TASSERT( heapSize > 0, "Allocation size is zero" );
	TASSERT( ( heapSize & 3 ) == 0, "Allocation size is not aligned to 4" );

	TMemoryDLHeap* heap = static_cast<TMemoryDLHeap*>( ptr );
	TUtil::MemClear( heap, sizeof( TMemoryDLHeap ) );

	TSIZE capacity = heapSize - sizeof( TMemoryDLHeap );
	heap->m_MSpace = create_mspace_with_base( heap + 1, capacity, 1 );

	if ( heap->m_MSpace != TNULL )
	{
		heap->m_Flags = flags;
		heap->SetName( name );

		if ( flags & TMemoryHeapFlags_UseMutex )
		{
			heap->CreateMutex();
		}
		else
		{
			TUtil::MemSet( &heap->m_Mutex, 0, sizeof( heap->m_Mutex ) );
		}

		return heap;
	}
	else
	{
		return TNULL;
	}
}

void* TMemoryDLHeap::AllocAsPile( TMemoryDLHeap* heap, TSIZE size, TSIZE alignment )
{
	TASSERT( heap->m_Flags & TMemoryHeapFlags_AllocAsPile, "Can't allocate as pile on a non-pile heap" );

	// Align current pointer
	heap->m_PileData = reinterpret_cast<char*>( ( ( (TSIZE)heap->m_PileData + ( alignment - 1 ) ) / alignment ) * alignment );
	TSIZE usedMemorySize = heap->m_PileData - heap->m_SubHeapBuffer;

	if ( usedMemorySize + size > heap->m_PileSize )
	{
		TERROR( "Out of memory in pile '{0}' when trying to alloc {1} bytes", heap->m_Name, size );
		TBREAK();
	}

	void* allocated = heap->m_PileData;
	heap->m_PileData += size;

	return allocated;
}

TOSHI_NAMESPACE_END
