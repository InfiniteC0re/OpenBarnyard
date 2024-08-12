#pragma once

#define TDECLARE_FREELIST_ALLOCATOR( CLASS_NAME )                                                    \
public:                                                                                              \
    TFORCEINLINE void* operator new( size_t s ) { return ms_oFreeList.New( sizeof( CLASS_NAME ) ); } \
    TFORCEINLINE void* operator new( size_t s, void* where ) { return where; }                       \
                                                                                                     \
    TFORCEINLINE void operator delete( void* ptr ) { ms_oFreeList.Delete( ptr ); }                   \
    TFORCEINLINE void operator delete( void* ptr, void* where ) { delete ptr; }                      \
                                                                                                     \
    static void SetupFreeList( TINT a_iCapacity, TINT a_iGrowSize )                                  \
    {                                                                                                \
        ms_oFreeList.SetCapacity( a_iCapacity );                                                     \
        ms_oFreeList.SetGrowSize( a_iGrowSize );                                                     \
    }                                                                                                \
                                                                                                     \
    static Toshi::TFreeList ms_oFreeList;

#define TDEFINE_FREELIST_ALLOCATOR( CLASS_NAME ) \
    Toshi::TFreeList CLASS_NAME::ms_oFreeList = Toshi::TFreeList( sizeof( CLASS_NAME ), 0, 8, #CLASS_NAME );

#define TDEFINE_FREELIST_ALLOCATOR1( CLASS_NAME, GROW_SIZE ) \
    Toshi::TFreeList CLASS_NAME::ms_oFreeList = Toshi::TFreeList( sizeof( CLASS_NAME ), 0, GROW_SIZE, #CLASS_NAME );

TOSHI_NAMESPACE_START

class TFreeList
{
public:
    struct Node
    {
        Node* pNext = TNULL;
    };

public:
    TFreeList( TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize, const TCHAR* a_pchName );

    Node* Allocate( TINT a_iNumber, TINT a_iSize );
    void  SetCapacity( TINT a_iNewCapacity );

    TINT GetCapacity() const { return m_iCapacity; }
    TINT GetGrowSize() const { return m_iGrowSize; }

    void  SetGrowSize( TINT a_iGrowSize ) { a_iGrowSize < 0 ? m_iGrowSize = 8 : m_iGrowSize = a_iGrowSize; }
    void* New( TUINT a_uiSize );

    void Delete( void* a_Ptr );

private:
    inline static TFreeList* ms_pLastList = TNULL;

private:
    TFreeList*         m_pPrevList;
    TINT               m_iCapacity;
    TUINT              m_uiItemSize;
    Node               m_LastNode;
    Node               m_RootNode;
    TINT               m_iGrowSize;
    TMemory::MemBlock* m_pMemoryHeap;
};

TOSHI_NAMESPACE_END
