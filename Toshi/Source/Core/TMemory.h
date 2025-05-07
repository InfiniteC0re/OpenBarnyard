/**
 * @file TMemory.h
 * Core memory management system for the Toshi engine
 * 
 * This file implements a custom memory allocator that provides:
 * - Memory block management with alignment support
 * - Memory tracking and debugging capabilities
 * - Support for both custom allocator and dlmalloc implementations
 */

#pragma once
#include "Toshi/TNodeList.h"

#ifdef TOSHI_PROFILER_MEMORY
#  include "TMemoryDebug.h"
#endif // TOSHI_PROFILER_MEMORY

#ifdef TMEMORY_USE_DLMALLOC
#  include "TMemory_dlmalloc.h"
#endif // TMEMORY_USE_DLMALLOC

#ifndef TMEMORY_USE_DLMALLOC

// Memory allocation constants
#  define TMEMORY_ROUNDUP             4                                                ///< Minimum alignment for memory blocks
#  define TMEMORY_FLAGS_HOLE_PROCESS  1                                                ///< Flag indicating a memory block is in use
#  define TMEMORY_FLAGS_MASK          ( ( 1 << 2 ) - 1 )                               ///< Mask for memory block flags
#  define TMEMORY_NUM_FREELISTS       9                                                ///< Number of free list buckets for different sizes
#  define TMEMORY_NUM_BLOCK_SLOTS     128                                              ///< Maximum number of memory block slots
#  define TMEMORY_ALLOC_MEMNODE_SIZE  sizeof( Toshi::TMemory::MemNode )                ///< Size of memory node structure
#  define TMEMORY_ALLOC_RESERVED_SIZE ( TMEMORY_ALLOC_MEMNODE_SIZE - sizeof( void* ) ) ///< Reserved space in allocations

TOSHI_NAMESPACE_START

extern class TMemory* g_pMemory;

/**
 * @class TMemory
 * Core memory management class that handles all memory allocations and deallocations
 * 
 * This class implements a custom memory allocator with the following features:
 * - Memory block management with alignment support
 * - Memory tracking and debugging capabilities
 * - Support for memory pools and sub-heaps
 * - Thread-safe operations
 */
class TMemory
{
public:
	/**
	 * @struct MemBlock
	 * Represents a contiguous block of memory that can be allocated from
	 */
	struct MemBlock;

	/**
	 * @struct MemBlockSlot
	 * Slot for tracking memory blocks in the block list
	 */
	struct MemBlockSlot;

	/**
	 * @class MemNode
	 * Represents a node in the memory allocation system
	 * 
	 * Each MemNode tracks a piece of memory that can be either allocated or free.
	 * The node contains metadata about the memory region and links to other nodes.
	 */
	class MemNode
	{
	public:
		/**
		 * Gets the start address of the data region for this node
		 * @return Pointer to the start of the data region
		 */
		void* GetDataRegionStart() const { return (void*)( &pPrevHole ); }

		/**
		 * Gets the end address of the data region for this node
		 * @return Pointer to the end of the data region
		 */
		void* GetDataRegionEnd() const { return (void*)( ( (TUINTPTR)&pPrevHole + TAlignNumDown( uiSize ) ) ); }

	public:
		MemNode* pOwner; ///< Owner node if this is a sub-allocation
		TSIZE    uiSize; ///< Size of the memory region
		union
		{
			MemNode*  pNextHole; ///< Next free hole in the list
			MemBlock* pMemBlock; ///< Memory block this node belongs to
		};

		MemNode* pPrevHole; ///< Previous free hole in the list
	};

	/**
	 * @struct MemBlock
	 * Represents a contiguous block of memory that can be allocated from
	 */
	struct MemBlock
	{
		MemBlockSlot* m_pSlot;                            ///< Slot tracking this block
		TSIZE         m_uiTotalSize1;                     ///< Total size of the block
		MemBlock*     m_pNextBlock;                       ///< Next block in the chain
		TSIZE         m_uiTotalSize2;                     ///< Duplicate of total size for validation
		MemNode*      m_apHoles[ TMEMORY_NUM_FREELISTS ]; ///< Array of free lists by size
		MemNode*      m_pFirstHole;                       ///< First free hole in the block
		TCHAR         m_szSignature[ 8 ];                 ///< Block signature for validation
		TCHAR         m_szName[ 52 ];                     ///< Name of the memory block
		MemNode       m_RootHole;                         ///< Root node for the block
	};

	/**
	 * @struct MemBlockFooter
	 * Footer structure at the end of each memory block for validation
	 */
	struct MemBlockFooter
	{
		TUINT     m_Unk1;        ///< Unknown field 1
		TUINT     m_Unk2;        ///< Unknown field 2
		MemBlock* m_pBlockOwner; ///< Pointer to the owning block
		TUINT     m_Unk4;        ///< Unknown field 4
	};

	/**
	 * @struct MemBlockSlot
	 * Slot for tracking memory blocks in the block list
	 */
	struct MemBlockSlot : public TNodeList<MemBlockSlot>::TNode
	{
		MemBlock* m_pPtr = TNULL; ///< Pointer to the memory block
	};

	/**
	 * @struct MemInfo
	 * Structure containing memory usage statistics
	 */
	struct MemInfo
	{
		TSIZE m_uiTotalSize;       ///< Total size of memory block
		TSIZE m_uiLogicTotalSize;  ///< Logical total size
		TSIZE m_uiTotalFree;       ///< Total free memory
		TSIZE m_uiLogicTotalFree;  ///< Logical total free memory
		TSIZE m_uiTotalUsed;       ///< Total used memory
		TSIZE m_uiLogicTotalUsed;  ///< Logical total used memory
		TINT  m_iNumHoles;         ///< Number of free holes
		TINT  m_iNumProcesses;     ///< Number of allocated regions
		TSIZE m_uiLargestHole;     ///< Size of largest free hole
		TSIZE m_uiLargestProcess;  ///< Size of largest allocation
		TSIZE m_uiSmallestHole;    ///< Size of smallest free hole
		TSIZE m_uiSmallestProcess; ///< Size of smallest allocation
		TSIZE m_uiUnk3;            ///< Unknown field 3
		TSIZE m_uiUnk4;            ///< Unknown field 4
	};

	/**
	 * @struct HALMemInfo
	 * Hardware abstraction layer memory information
	 */
	struct HALMemInfo
	{
		HALMemInfo();

		TUINT m_Unknown1[ 10 ]; ///< Unknown fields 1-10
		TSIZE m_uiMemUsage;     ///< Total memory usage
		TUINT m_Unknown2[ 15 ]; ///< Unknown fields 11-25
	};

public:
	/**
	 * Constructor for TMemory class
	 * Initializes the memory management system
	 */
	TMemory();

	/**
	 * Destructor for TMemory class
	 * Cleans up the memory management system
	 */
	~TMemory();

	/**
	 * Allocates memory with specified size and alignment
	 * @param a_uiSize Size of memory to allocate
	 * @param a_uiAlignment Alignment requirement for the allocation
	 * @param a_pMemBlock Memory block to allocate from (NULL for global block)
	 * @param a_szFileName Source file name for debugging
	 * @param a_iLineNum Source line number for debugging
	 * @return Pointer to allocated memory or NULL if allocation failed
	 */
	void* Alloc( TSIZE a_uiSize, TSIZE a_uiAlignment, MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum );

	/**
	 * Frees previously allocated memory
	 * @param a_pMem Pointer to memory to free
	 * @return TRUE if memory was successfully freed
	 */
	TBOOL Free( const void* a_pMem );

	/**
	 * Allocates memory from the system heap
	 * @param a_uiSize Size of memory to allocate
	 * @return Pointer to allocated memory
	 */
	void* SysAlloc( TSIZE a_uiSize );

	/**
	 * Frees memory back to the system heap
	 * @param a_pMem Pointer to memory to free
	 */
	void SysFree( void* a_pMem );

	/**
	 * Creates a new memory block
	 * @param a_uiSize Size of the memory block
	 * @param a_szName Name of the memory block
	 * @param a_pOwnerBlock Owner block (NULL for global block)
	 * @param a_iUnused Unused parameter
	 * @return Pointer to created memory block
	 */
	MemBlock* CreateMemBlock( TSIZE a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock, TINT a_iUnused );

	/**
	 * Creates a memory block at a specific location
	 * @param a_pMem Pointer to memory location
	 * @param a_uiSize Size of the memory block
	 * @param a_szName Name of the memory block
	 * @return Pointer to created memory block
	 */
	MemBlock* CreateMemBlockInPlace( void* a_pMem, TSIZE a_uiSize, const TCHAR* a_szName );

	/**
	 * Destroys a memory block
	 * @param a_pMemBlock Pointer to memory block to destroy
	 */
	void DestroyMemBlock( MemBlock* a_pMemBlock );

	/**
	 * Gets the global memory block
	 * @return Pointer to global memory block
	 */
	MemBlock* GetGlobalBlock() const;

	/**
	 * Sets the global memory block
	 * @param a_pMemBlock New global memory block
	 * @return Pointer to the previous global memory block
	 */
	TMemory::MemBlock* SetGlobalBlock( MemBlock* a_pMemBlock );

	TUINT GetGlobalFlags() const { return m_uiGlobalFlags; }

	void DumpMemInfo();

private:
	TBOOL FreeMemBlock( MemBlock* a_pMemBlock );
	void  SetMemBlockUnused( MemBlock* a_pMemBlock );

	static MemNode* GetMemNodeFromAddress( void* a_pMem );
	static void     ExtendNodeSize( MemNode* a_pNode, TSIZE a_uiExtendSize ) { a_pNode->uiSize = a_uiExtendSize | ( a_pNode->uiSize & TMEMORY_FLAGS_MASK ); }
	static void     SetHoleSize( MemNode* a_pNode, TSIZE a_uiHoleSize ) { a_pNode->uiSize = a_uiHoleSize; }
	static TBOOL    IsProcess( MemNode* a_pNode ) { return HASANYFLAG( a_pNode->uiSize, TMEMORY_FLAGS_HOLE_PROCESS ); }
	static void     SetProcess( MemBlock* a_pMemBlock, MemNode* a_pNode, TSIZE a_uiHoleSize )
	{
		a_pNode->uiSize    = a_uiHoleSize | g_pMemory->GetGlobalFlags() | TMEMORY_FLAGS_HOLE_PROCESS;
		a_pNode->pMemBlock = a_pMemBlock;
	}
	static TSIZE     GetNodeSize( MemNode* a_pNode ) { return TAlignNumDown( a_pNode->uiSize ); }
	static MemBlock* GetProcessMemBlock( MemNode* a_pNode ) { return a_pNode->pMemBlock; }
	static void      ConvertProcessToHole( MemNode* a_pNode ) { a_pNode->uiSize &= ~TMEMORY_FLAGS_MASK; }
	static int       TestMemIntegrity( MemBlock* a_pMemBlock );
	static int       DebugTestMemoryBlock( MemBlock* a_pMemBlock );

public:
	static void  GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock );
	static void  GetHALMemInfo( HALMemInfo& a_rHALMemInfo );
	static TBOOL Initialise( TSIZE a_uiHeapSize, TSIZE a_uiReservedSize, TUINT a_uiUnused = 0 );
	static void  Deinitialise();
	static TUINT MapSizeToFreeList( TSIZE a_uiSize );
	static void  DebugPrintHALMemInfo( const TCHAR* a_szFormat, ... );

private:
	inline static class TMutex* ms_pGlobalMutex;

private:
	TSIZE                   m_TotalAllocatedSize;
	TSIZE                   m_ReservedSize;
	TSIZE                   m_MainBlockSize;
	TNodeList<MemBlockSlot> m_UsedBlocks;
	TNodeList<MemBlockSlot> m_FreeBlocks;
	MemBlockSlot            m_aBlockSlots[ TMEMORY_NUM_BLOCK_SLOTS ];
	void*                   m_pMemory;
	MemBlock*               m_pGlobalBlock;
	TUINT                   m_uiGlobalFlags;
	TUINT                   m_Unknown2;
	TBOOL                   m_bFlag1;
	TBOOL                   m_bFlag2;
	HALMemInfo              m_HALMemInfo;
};

TOSHI_NAMESPACE_END

#else // !TMEMORY_USE_DLMALLOC

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: A wrapper around TMemoryDL that makes it's API compatible to the
// JPOG/Barnyard memory allocator.
//-----------------------------------------------------------------------------
class TMemory
{
public:
	using MemBlock = TMemoryDLHeap;

	struct MemInfo
	{
		TUINT m_uiTotalSize;
		TUINT m_uiLogicTotalSize;
		TUINT m_uiTotalFree;
		TUINT m_uiLogicTotalFree;
		TUINT m_uiTotalUsed;
		TUINT m_uiLogicTotalUsed;
		TINT  m_iNumHoles;
		TINT  m_iNumProcesses;
		TUINT m_uiLargestHole;
		TUINT m_uiLargestProcess;
		TUINT m_uiSmallestHole;
		TUINT m_uiSmallestProcess;
		TUINT m_uiUnk3;
		TUINT m_uiUnk4;
	};

	struct HALMemInfo
	{
		TUINT m_Unknown1[ 10 ];
		TUINT m_uiMemUsage;
		TUINT m_Unknown2[ 15 ];
	};

public:
	TMemory();

	MemBlock* CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock, TINT a_iUnused );
	MemBlock* CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName );
	void      DestroyMemBlock( MemBlock* a_pMemBlock );

	MemBlock* GetGlobalBlock() const;
	MemBlock* SetGlobalBlock( MemBlock* a_pMemBlock );

	void       SetMemModule( TMemoryDL* a_pMemModule ) { m_pMemModule = a_pMemModule; }
	TMemoryDL* GetMemModule() { return m_pMemModule; }

public:
	static TBOOL Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize, TMemoryDL::Flags a_eFlags = TMemoryDL::Flags_Standard );
	static void  Deinitialise();

	static void GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock );
	static void GetHALMemInfo( HALMemInfo& a_rHALMemInfo );

public:
	TMemoryDL* m_pMemModule;
	TCHAR      PADDING[ 2092 ];
	MemBlock*  m_pGlobalBlock;
};

extern TMemory* g_pMemory;

TOSHI_NAMESPACE_END

#endif // TMEMORY_USE_DLMALLOC

/**
 * Allocates memory with default alignment
 * @param a_uiSize Size of memory to allocate
 * @return Pointer to allocated memory
 */
void* TMalloc( TSIZE a_uiSize );

/**
 * Allocates memory with debug information
 * @param a_uiSize Size of memory to allocate
 * @param a_szFileName Source file name
 * @param a_iLineNum Source line number
 * @return Pointer to allocated memory
 */
void* TMalloc( TSIZE a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum );

/**
 * Allocates memory from a specific memory block
 * @param a_uiSize Size of memory to allocate
 * @param a_pMemBlock Memory block to allocate from
 * @param a_szFileName Source file name (optional)
 * @param a_iLineNum Source line number (optional)
 * @return Pointer to allocated memory
 */
void* TMalloc( TSIZE a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName = TNULL, TINT a_iLineNum = -1 );

/**
 * Allocates aligned memory
 * @param a_uiSize Size of memory to allocate
 * @param a_uiAlignment Alignment requirement
 * @return Pointer to allocated memory
 */
void* TMemalign( TSIZE a_uiSize, TSIZE a_uiAlignment );

/**
 * Allocates aligned memory from a specific memory block
 * @param a_uiAlignment Alignment requirement
 * @param a_uiSize Size of memory to allocate
 * @param a_pMemBlock Memory block to allocate from
 * @return Pointer to allocated memory
 */
void* TMemalign( TSIZE a_uiAlignment, TSIZE a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock );

/**
 * Frees previously allocated memory
 * @param a_pMem Pointer to memory to free
 */
void TFree( void* a_pMem );

/**
 * Constructs an object in place
 * @tparam T Type of object to construct
 * @tparam Args Types of constructor arguments
 * @param a_pMemory Memory location to construct in
 * @param args Constructor arguments
 * @return Pointer to constructed object
 */
template <class T, class... Args>
TFORCEINLINE T* TConstruct( T* a_pMemory, Args&&... args )
{
	return new ( a_pMemory ) T( std::forward<Args>( args )... );
}

/**
 * Constructs an array of objects in place
 * @tparam T Type of objects to construct
 * @tparam Args Types of constructor arguments
 * @param a_pMemory Memory location to construct in
 * @param a_uiNumTimes Number of objects to construct
 * @param args Constructor arguments
 * @return Pointer to first constructed object
 */
template <class T, class... Args>
TFORCEINLINE T* TConstructArray( T* a_pMemory, TSIZE a_uiNumTimes, Args&&... args )
{
	for ( TSIZE i = 0; i < a_uiNumTimes; i++ )
	{
		new ( a_pMemory + i ) T( std::forward<Args>( args )... );
	}

	return a_pMemory;
}

/**
 * Destructs an object
 * @tparam T Type of object to destruct
 * @param a_pPtr Pointer to object to destruct
 */
template <class T>
TFORCEINLINE void TDestruct( T* a_pPtr )
{
	if ( a_pPtr )
	{
		a_pPtr->~T();
	}
}

/**
 * Placement new operator for memory blocks
 * @param size Size to allocate
 * @param block Memory block to allocate from
 * @return Pointer to allocated memory
 */
TFORCEINLINE void* __CRTDECL operator new( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

/**
 * Placement new[] operator for memory blocks
 * @param size Size to allocate
 * @param block Memory block to allocate from
 * @return Pointer to allocated memory
 */
TFORCEINLINE void* __CRTDECL operator new[]( size_t size, Toshi::TMemory::MemBlock* block )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, block, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, block, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

/**
 * Placement delete operator for memory blocks
 * @param ptr Pointer to memory to free
 * @param block Memory block (unused)
 */
TFORCEINLINE void __CRTDECL operator delete( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}

/**
 * Placement delete[] operator for memory blocks
 * @param ptr Pointer to memory to free
 * @param block Memory block (unused)
 */
TFORCEINLINE void __CRTDECL operator delete[]( void* ptr, Toshi::TMemory::MemBlock* block ) noexcept
{
	TFree( ptr );
}
