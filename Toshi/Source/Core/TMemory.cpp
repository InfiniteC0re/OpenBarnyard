#include "ToshiPCH.h"
#include "TMemory.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#ifdef TOSHI_PROFILER_MEMORY
#include "Profiler/tracy/Tracy.hpp"
#endif // TOSHI_PROFILER_MEMORY

#define MEM_TO_HOLE(PTR) (((Hole*)(((TUINT)PTR) + sizeof(void*))) - 1)

void* __CRTDECL operator new( size_t size )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new( size_t size, ::std::nothrow_t const& ) noexcept
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new[]( size_t size )
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void* __CRTDECL operator new[]( size_t size, ::std::nothrow_t const& ) noexcept
{
#ifdef TOSHI_PROFILER_MEMORY
	return TMalloc( size, TNULL, TMemory__FILE__, TMemory__LINE__ );
#else
	return TMalloc( size, TNULL, TNULL, -1 );
#endif // TOSHI_PROFILER_MEMORY
}

void __CRTDECL operator delete( void* ptr ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete( void* ptr, ::std::nothrow_t const& ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr, ::std::nothrow_t const& ) noexcept
{
	TFree( ptr );
}

void __CRTDECL operator delete[]( void* ptr, size_t _Size ) noexcept
{
	TFree( ptr );
}

namespace Toshi {

	TMemory::TMemory()
	{
		m_TotalAllocatedSize = 0;

		for ( TINT i = 0; i < NUM_BLOCK_SLOTS; i++ )
		{
			m_FreeBlocks.InsertTail( m_aBlockSlots[ i ] );
		}

		m_bFlag1 = TFALSE;
		m_bFlag2 = TTRUE;
	}

	TMemory::~TMemory()
	{

	}

	// General utility macro
#define PP_CAT( A, B ) A ## B
#define PP_EXPAND(...) __VA_ARGS__

// Macro overloading feature support
#define PP_VA_ARG_SIZE(...) PP_EXPAND(PP_APPLY_ARG_N((PP_ZERO_ARGS_DETECT(__VA_ARGS__), PP_RSEQ_N)))

#define PP_ZERO_ARGS_DETECT(...) PP_EXPAND(PP_ZERO_ARGS_DETECT_PREFIX_ ## __VA_ARGS__ ## _ZERO_ARGS_DETECT_SUFFIX)
#define PP_ZERO_ARGS_DETECT_PREFIX__ZERO_ARGS_DETECT_SUFFIX ,,,,,,,,,,,0

#define PP_APPLY_ARG_N(ARGS) PP_EXPAND(PP_ARG_N ARGS)
#define PP_ARG_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N,...) N
#define PP_RSEQ_N 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define PP_OVERLOAD_SELECT(NAME, NUM) PP_CAT( NAME ## _, NUM)
#define PP_MACRO_OVERLOAD(NAME, ...) PP_OVERLOAD_SELECT(NAME, PP_VA_ARG_SIZE(__VA_ARGS__))(__VA_ARGS__)

#define CALL_THIS_4(ADDR, TYPE, RET_TYPE, THIS) ((RET_TYPE(__thiscall*)(TYPE pThis))(ADDR))(THIS)
#define CALL_THIS_6(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1) ((RET_TYPE(__thiscall*)(TYPE, TYPE1))(ADDR))(THIS, VALUE1)
#define CALL_THIS_8(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2))(ADDR))(THIS, VALUE1, VALUE2)
#define CALL_THIS_10(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3))(ADDR))(THIS, VALUE1, VALUE2, VALUE3)
#define CALL_THIS_12(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3, TYPE4))(ADDR))(THIS, VALUE1, VALUE2, VALUE3, VALUE4)
#define CALL_THIS_14(ADDR, TYPE, RET_TYPE, THIS, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5) ((RET_TYPE(__thiscall*)(TYPE, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5))(ADDR))(THIS, VALUE1, VALUE2, VALUE3, VALUE4, VALUE5)
#define CALL_THIS(...) PP_MACRO_OVERLOAD(CALL_THIS, __VA_ARGS__)

#define CALL_2(ADDR, RET_TYPE) ((RET_TYPE(__stdcall*)())(ADDR))()
#define CALL_4(ADDR, RET_TYPE, TYPE1, VALUE1) ((RET_TYPE(__stdcall*)(TYPE1))(ADDR))(VALUE1)
#define CALL_6(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2))(ADDR))(VALUE1, VALUE2)
#define CALL_8(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3))(ADDR))(VALUE1, VALUE2, VALUE3)
#define CALL_10(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3, TYPE4))(ADDR))(VALUE1, VALUE2, VALUE3, VALUE4)
#define CALL_12(ADDR, RET_TYPE, TYPE1, VALUE1, TYPE2, VALUE2, TYPE3, VALUE3, TYPE4, VALUE4, TYPE5, VALUE5) ((RET_TYPE(__stdcall*)(TYPE1, TYPE2, TYPE3, TYPE4, TYPE5))(ADDR))(VALUE1, VALUE2, VALUE3, VALUE4, VALUE5)
#define CALL(...) PP_MACRO_OVERLOAD(CALL, __VA_ARGS__)

	void* TMemory::Alloc( TUINT a__uiSize, TINT a__uiAlignment, MemBlock* a__pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum )
	{
		//return CALL_THIS(0x006b5230, TMemory*, void*, this, TUINT, a__uiSize, TINT, a__uiAlignment, MemBlock*, a__pMemBlock, const TCHAR*, a_szFileName, TINT, a_iLineNum );

		int v6; // ebx
		int v7; // edi
		unsigned int v8; // esi
		int v9; // ebp
		unsigned int v11; // edi
		unsigned int v12; // eax
		_DWORD* v13; // ecx
		unsigned int v14; // edi
		int v15; // ebx
		int v16; // edx
		int v17; // eax
		_DWORD* v18; // eax
		_DWORD* v19; // edx
		int v20; // ebp
		int v21; // eax
		_DWORD* v22; // eax
		_DWORD* v23; // ebp
		unsigned int v24; // edi
		_DWORD* v25; // ecx
		int v26; // ebp
		unsigned int v27; // eax
		unsigned int v28; // eax
		int v29; // edx
		int v30; // [esp+10h] [ebp-Ch] BYREF
		unsigned int v31; // [esp+14h] [ebp-8h]
		int v32; // [esp+18h] [ebp-4h]
		unsigned int v33; // [esp+20h] [ebp+4h]
		_DWORD** v34; // [esp+24h] [ebp+8h]
		int v35; // [esp+28h] [ebp+Ch]

		TUINT a_uiSize = (TUINT)a__uiSize;
		TUINT a_uiAlignment = (TUINT)a__uiAlignment;
		TUINT a_pMemBlock = (TUINT)a__pMemBlock;

		v6 = (int)this;
		v32 = (int)this;
		TMutexLock lock( ms_pGlobalMutex );
		v7 = a_uiSize;
		v8 = 0;
		if ( a_uiSize < 4 )
			v7 = 4;
		v9 = a_uiAlignment;
		if ( a_uiAlignment < 0x10 )
			v9 = 16;
		if ( !a_pMemBlock )
			a_pMemBlock = *(_DWORD*)( v6 + 2096 );
		if ( !v7 )
		{
			TDebug_FinalPrintf( "Out of Toshi Memory on block [%s]\n", a_pMemBlock + 64 );
			return 0;
		}
		v11 = ( v7 + 3 ) & 0xFFFFFFFC;
		v33 = v11;
		v12 = MapSizeToFreeList( v11 );
		v31 = v12;
		if ( v12 >= 9 )
			goto LABEL_20;
		v34 = (_DWORD**)( a_pMemBlock + 4 * v12 + 16 );
		do
		{
			v13 = *v34;
			if ( *v34 )
			{
				while ( 1 )
				{
					v8 = ~( v9 - 1 ) & ( (unsigned int)v13 + v9 + 11 );
					v14 = (unsigned int)v13 + ( v13[ 1 ] & 0xFFFFFFFC ) - v8 + 12;
					if ( (unsigned int)v13 + ( v13[ 1 ] & 0xFFFFFFFC ) + 12 > v8 && v14 >= v33 )
						break;
					v13 = (_DWORD*)v13[ 2 ];
					if ( !v13 )
					{
						//LOBYTE( v11 ) = v33;
						goto LABEL_17;
					}
				}
				if ( *v13 )
				{
					*(_DWORD*)( *v13 + 4 ) = *(_DWORD*)( *v13 + 4 ) & 3 | ( v8 - *v13 - 24 );
					v12 = v31;
				}
				else if ( (_DWORD*)( v8 - 12 ) != v13 )
				{
					v15 = a_pMemBlock;
					*(_DWORD*)( a_pMemBlock + 52 ) = v8 - 12;
LABEL_25:
					if ( v14 > v33 + 16 )
					{
						v20 = v13[ 3 ];
						if ( v20 )
							*(_DWORD*)( v20 + 8 ) = v13[ 2 ];
						else
							*(_DWORD*)( v15 + 4 * v12 + 16 ) = v13[ 2 ];
						v21 = v13[ 2 ];
						if ( v21 )
							*(_DWORD*)( v21 + 12 ) = v13[ 3 ];
						v22 = (_DWORD*)( v8 - 12 );
						if ( v13 != (_DWORD*)( v8 - 12 ) )
						{
							v23 = (_DWORD*)( (char*)v13 + ( v13[ 1 ] & 0xFFFFFFFC ) + 12 );
							*v22 = *v13;
							*v23 = (TUINT32)v22;
						}
						v22[ 1 ] = v14 | m_Unknown1 | 1;
						v24 = v22[ 1 ] & 0xFFFFFFFC;
						v22[ 2 ] = v15;
						v25 = (_DWORD*)( v8 + v33 );
						v25[ 1 ] = v24 - v33 - 12;
						v26 = m_Unknown1;
						v22[ 2 ] = v15;
						v22[ 1 ] = v26 | v33 | 1;
						*v25 = (TUINT32)v22;
						*(float*)&v35 = (float)(int)( ( *(_DWORD*)( v8 + v33 + 4 ) & 0xFFFFFFFC ) - 1 );
						v27 = ( v35 >> 23 ) - 127;
						if ( ( v27 & 1 ) != 0 )
							++v27;
						v28 = v27 >> 1;
						if ( v28 )
							--v28;
						if ( v28 >= 9 )
							v28 = 8;
						v29 = *(_DWORD*)( v15 + 4 * v28 + 16 );
						v25[ 2 ] = v29;
						if ( v29 )
							*(_DWORD*)( v29 + 12 ) = (TUINT32)v25;
						v25[ 3 ] = 0;
						*(_DWORD*)( v15 + 4 * v28 + 16 ) = (TUINT32)v25;
						*(_DWORD*)( (char*)v25 + ( v25[ 1 ] & 0xFFFFFFFC ) + 12 ) = (TUINT32)v25;
						return (void*)v8;
					}
					else
					{
						v16 = v13[ 3 ];
						if ( v16 )
							*(_DWORD*)( v16 + 8 ) = v13[ 2 ];
						else
							*(_DWORD*)( v15 + 4 * v12 + 16 ) = v13[ 2 ];
						v17 = v13[ 2 ];
						if ( v17 )
							*(_DWORD*)( v17 + 12 ) = v13[ 3 ];
						v18 = (_DWORD*)( v8 - 12 );
						if ( v13 != (_DWORD*)( v8 - 12 ) )
						{
							v19 = (_DWORD*)( (char*)v13 + ( v13[ 1 ] & 0xFFFFFFFC ) + 12 );
							*v18 = *v13;
							*v19 = (TUINT32)v18;
						}
						v18[ 1 ] = v14 | m_Unknown1 | 1;
						v18[ 2 ] = v15;
						return (void*)v8;
					}
				}
				v15 = a_pMemBlock;
				goto LABEL_25;
			}
LABEL_17:
			v31 = ++v12;
			++v34;
		} while ( v12 < 9 );
		if ( v13 )
			goto LABEL_21;
		v6 = v32;
LABEL_20:
		TDebug_FinalPrintf( "Out of Toshi Memory on block [%s]\n", a_pMemBlock + 64 );
		TDebug_FinalPrintf( "Requested memory block size: %d\n", v11 );
		DumpMemInfo();
		v8 = 0;
LABEL_21:
		return (void*)v8;

//		TMutexLock lock( ms_pGlobalMutex );
//
//		if ( a_uiSize < 4 )
//		{
//			a_uiSize = 4;
//		}
//
//		if ( a_uiAlignment < 16 )
//		{
//			a_uiAlignment = 16;
//		}
//		else if ( a_uiAlignment < 4 )
//		{
//			PrintDebug( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
//			TASSERT( TFALSE );
//			return TNULL;
//		}
//
//		if ( !a_pMemBlock )
//		{
//			a_pMemBlock = m_pGlobalBlock;
//		}
//
//		if ( a_uiSize == 0 )
//		{
//			PrintDebug( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
//			TASSERT( TFALSE );
//			return TNULL;
//		}
//
//		a_uiSize = TAlignNumUp( a_uiSize );
//		TUINT uiFreeListId = MapSizeToFreeList( a_uiSize );
//
//		TUINT iDataStart = 0;
//		TUINT iDataEnd;
//		TUINT iDataSize;
//		Hole* pFreeList = TNULL;
//
//#ifdef TOSHI_PROFILER_MEMORY
//#include "TMemoryDebugOff.h"
//
//		auto SaveDebugInfo = [ a_szFileName, a_iLineNum, a_uiSize ]( void* pMem ) {
//			if ( pMem )
//			{
//				TracyAlloc( pMem, a_uiSize );
//			}
//		};
//#endif // TOSHI_PROFILER_MEMORY
//
//		for ( TUINT i = uiFreeListId; i < NUM_FREE_LISTS; i++ )
//		{
//			pFreeList = a_pMemBlock->m_pHoles[ i ];
//
//			while ( pFreeList != TNULL )
//			{
//				iDataEnd = TREINTERPRETCAST( TUINT, &pFreeList->m_pPrevHole ) + TAlignNumDown( pFreeList->m_uiSize );
//				iDataStart = TREINTERPRETCAST( TUINT, &pFreeList->m_pNextHole ) + a_uiAlignment + 3 & ~( a_uiAlignment - 1 );
//				iDataSize = iDataEnd - iDataStart;
//
//				if ( iDataStart < iDataEnd && a_uiSize <= iDataSize )
//				{
//					auto pHole = MEM_TO_HOLE( iDataStart );
//					auto unk1 = pFreeList->m_Unk1;
//
//					if ( unk1 == TNULL )
//					{
//						if ( pHole != pFreeList )
//						{
//							a_pMemBlock->m_pFirstHole = pHole;
//						}
//					}
//					else
//					{
//						unk1->m_uiSize = (TUINT)( iDataStart + ( -0x18 - (TUINT)unk1 ) ) | unk1->m_uiSize & 3;
//					}
//
//					if ( a_uiSize + sizeof( Hole ) < iDataSize )
//					{
//						if ( pFreeList->m_pPrevHole == TNULL )
//						{
//							a_pMemBlock->m_pHoles[ i ] = pFreeList->m_pNextHole;
//						}
//						else
//						{
//							pFreeList->m_pPrevHole->m_pNextHole = pFreeList->m_pNextHole;
//						}
//
//						if ( pFreeList->m_pNextHole != TNULL )
//						{
//							pFreeList->m_pNextHole->m_pPrevHole = pFreeList->m_pPrevHole;
//						}
//
//						if ( pFreeList != pHole )
//						{
//							auto uiSize = pFreeList->m_uiSize;
//							pHole->m_Unk1 = pFreeList->m_Unk1;
//							*(Hole**)( (TUINT)&pFreeList->m_pPrevHole + TAlignNumDown( uiSize ) ) = pHole;
//						}
//
//						pHole->m_uiSize = GetSingleton()->m_Unknown1 | iDataSize | 1;
//						pHole->m_pMemBlock = a_pMemBlock;
//
//						auto pNewHole = (Hole*)( iDataStart + a_uiSize );
//						pNewHole->m_uiSize = ( ( TAlignNumDown( pHole->m_uiSize ) - iDataStart ) - a_uiSize ) + (TUINT)pHole;
//
//						pHole->m_pMemBlock = a_pMemBlock;
//						pHole->m_uiSize = (TINT)pNewHole + ( -0xc - (TINT)pHole ) | ms_pSingleton->m_Unknown1 | 1;
//						pNewHole->m_Unk1 = pHole;
//
//						TUINT uiNewFreeListId = MapSizeToFreeList( pNewHole->m_uiSize );
//
//						auto pOldHole = a_pMemBlock->m_pHoles[ uiNewFreeListId ];
//						pNewHole->m_pNextHole = pOldHole;
//
//						if ( pOldHole != TNULL )
//						{
//							pOldHole->m_pPrevHole = pNewHole;
//						}
//
//						pNewHole->m_pPrevHole = TNULL;
//						a_pMemBlock->m_pHoles[ uiNewFreeListId ] = pNewHole;
//						*(Hole**)( (TUINT)&pNewHole->m_pPrevHole + TAlignNumDown( pNewHole->m_uiSize ) ) = pNewHole;
//
//#ifdef TOSHI_PROFILER_MEMORY
//						SaveDebugInfo( (void*)iDataStart );
//#endif // TOSHI_PROFILER_MEMORY
//						return (void*)iDataStart;
//					}
//					else
//					{
//						if ( pFreeList->m_pPrevHole == TNULL )
//						{
//							a_pMemBlock->m_pHoles[ i ] = pFreeList->m_pNextHole;
//						}
//						else
//						{
//							pFreeList->m_pPrevHole->m_pNextHole = pFreeList->m_pNextHole;
//						}
//
//						if ( pFreeList->m_pNextHole != TNULL )
//						{
//							pFreeList->m_pNextHole->m_pPrevHole = pFreeList->m_pPrevHole;
//						}
//
//						if ( pFreeList != pHole )
//						{
//							auto uiSize = pFreeList->m_uiSize;
//							pHole->m_Unk1 = pFreeList->m_Unk1;
//							*(Hole**)( (TUINT)&pFreeList->m_pPrevHole + TAlignNumDown( uiSize ) ) = pHole;
//						}
//
//						pHole->m_uiSize = ms_pSingleton->m_Unknown1 | iDataSize | 1;
//						pHole->m_pMemBlock = a_pMemBlock;
//
//#ifdef TOSHI_PROFILER_MEMORY
//
//						SaveDebugInfo( (void*)iDataStart );
//
//#endif // TOSHI_PROFILER_MEMORY
//
//						return (void*)iDataStart;
//					}
//				}
//
//				pFreeList = pFreeList->m_pNextHole;
//			}
//
//			pFreeList = TNULL;
//		}
//
//		if ( pFreeList )
//		{
//#ifdef TOSHI_PROFILER_MEMORY
//
//			SaveDebugInfo( (void*)iDataStart );
//
//#endif // TOSHI_PROFILER_MEMORY
//
//			return (void*)iDataStart;
//		}
//
//		PrintDebug( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
//		PrintDebug( "Requested memory block size: %d\n", a_uiSize );
//		DumpMemInfo();
//		TASSERT( TFALSE );
//
//		return TNULL;
	}

	TBOOL TMemory::Free( void* a_pAllocated )
	{
		//return CALL_THIS(0x006b4a20, TMemory*, TBOOL, this, void*, a_pAllocated );

		TMutexLock lock( ms_pGlobalMutex );

		TUINTPTR uiMemAddr = TREINTERPRETCAST( TUINTPTR, a_pAllocated );

		if ( a_pAllocated && ( uiMemAddr & 3 ) == 0 )
		{
			Hole* pThisHole = MEM_TO_HOLE( uiMemAddr );
			MemBlock* pThisMemBlock = pThisHole->m_pMemBlock;

			TUINT uiAllocatedSize = TAlignNumDown( pThisHole->m_uiSize );
			Hole* pNextMemHole = TREINTERPRETCAST( Hole*, uiMemAddr + uiAllocatedSize );

			Hole* pOwnerHole = pThisHole->m_pOwnerHole;
			Hole** ppNextUsedHole = &pThisHole->m_pNextHole;

			pThisHole->m_uiSize = uiAllocatedSize;

			if ( pOwnerHole && ( pOwnerHole->m_uiSize & 1 ) == 0 )
			{
				// Merge with the owner hole

				TUINT uiOwnerHoleSize = pOwnerHole->m_uiSize;
				pOwnerHole->m_uiSize = uiAllocatedSize + ( sizeof( Hole ) - sizeof( void* ) ) + TAlignNumDown( uiOwnerHoleSize ) | uiOwnerHoleSize & 3;

				pNextMemHole->m_pOwnerHole = pOwnerHole;
				ppNextUsedHole = &pOwnerHole->m_pNextHole;

				if ( pOwnerHole->m_pPrevHole == TNULL )
				{
					// There's no other hole made of this one
					TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( uiOwnerHoleSize ) );
					pThisMemBlock->m_pHoles[ uiMappedHoleId ] = *ppNextUsedHole;
				}
				else
				{
					// Some other hole created from the owner still exists
					pOwnerHole->m_pPrevHole->m_pNextHole = *ppNextUsedHole;
				}

				pThisHole = pOwnerHole;

				if ( *ppNextUsedHole )
				{
					// Update previous hole of the next used hole
					( *ppNextUsedHole )->m_pPrevHole = pOwnerHole->m_pPrevHole;
				}
			}

			if ( pNextMemHole && ( pNextMemHole->m_uiSize & 1 ) == 0 )
			{
				if ( pNextMemHole->m_pPrevHole == TNULL )
				{
					TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( pNextMemHole->m_uiSize ) );
					pThisMemBlock->m_pHoles[ uiMappedHoleId ] = pNextMemHole->m_pNextHole;
				}
				else
				{
					pNextMemHole->m_pPrevHole->m_pNextHole = pNextMemHole->m_pNextHole;
				}

				if ( pNextMemHole->m_pNextHole != TNULL )
				{
					pNextMemHole->m_pNextHole->m_pPrevHole = pNextMemHole->m_pPrevHole;
				}

				TUINT uiSize = TAlignNumDown( pNextMemHole->m_uiSize );

				pThisHole->m_uiSize = uiSize + ( sizeof( Hole ) - sizeof( void* ) ) + TAlignNumDown( pThisHole->m_uiSize ) | pThisHole->m_uiSize & 3;
				*(TMemory::Hole**)( (TUINTPTR)&pNextMemHole->m_pPrevHole + uiSize ) = pThisHole;
			}

			TUINT uiMappedHoleId = MapSizeToFreeList( TAlignNumDown( pThisHole->m_uiSize ) );
			pThisHole->m_pPrevHole = TNULL;

			Hole* pOldHole = pThisMemBlock->m_pHoles[ uiMappedHoleId ];
			*ppNextUsedHole = pOldHole;

			if ( pOldHole != TNULL )
			{
				pOldHole->m_pPrevHole = pThisHole;
			}

			pThisMemBlock->m_pHoles[ uiMappedHoleId ] = pThisHole;

#ifdef TOSHI_PROFILER_MEMORY

			TracyFree( a_pAllocated );

#endif // TOSHI_PROFILER_MEMORY

			return TTRUE;
		}

		return TFALSE;
	}

	TMemory::MemBlock* TMemory::CreateMemBlock( TUINT a_uiSize, const TCHAR* a_szName, MemBlock* a_pOwnerBlock )
	{
		void* pMem = Alloc( a_uiSize, 16, a_pOwnerBlock, TNULL, -1 );
		return CreateMemBlockInPlace( pMem, a_uiSize, a_szName );
	}

	TMemory::MemBlock* TMemory::CreateMemBlockInPlace( void* a_pMem, TUINT a_uiSize, const TCHAR* a_szName )
	{
		TMutexLock lock( ms_pGlobalMutex );

		if ( a_pMem && a_uiSize != 0 && !m_FreeBlocks.IsEmpty() )
		{
			auto pBlock = TREINTERPRETCAST( MemBlock*, TAlignNumDown( TREINTERPRETCAST( TUINT32, a_pMem ) ) );
			auto uiBlockTotalSize = ( TREINTERPRETCAST( TUINT32, a_pMem ) + TAlignNumDown( a_uiSize ) ) - TREINTERPRETCAST( TUINT32, pBlock );

			if ( pBlock )
			{
				auto pSlot = m_FreeBlocks.RemoveHead();
				m_UsedBlocks.InsertHead( *pSlot );

				pBlock->m_pSlot = pSlot;
				pBlock->m_pSlot->m_pPtr = pBlock;
			}

			if ( uiBlockTotalSize != 0 )
			{
				constexpr TUINT CHUNK_RESERVED_SIZE = ( sizeof( MemBlock ) + 12 );

				pBlock->m_uiTotalSize1 = uiBlockTotalSize;
				TUtil::MemClear( pBlock->m_pHoles, sizeof( pBlock->m_pHoles ) );
				pBlock->m_pFirstHole = &pBlock->m_RootHole;

				auto pHole = &pBlock->m_RootHole;
				pHole->m_uiSize = uiBlockTotalSize - CHUNK_RESERVED_SIZE;
				pHole->m_pNextHole = TNULL;
				pHole->m_pPrevHole = TNULL;
				pHole->m_pOwnerHole = TNULL;

				auto uiFreeListId = MapSizeToFreeList( uiBlockTotalSize - CHUNK_RESERVED_SIZE );
				pBlock->m_pHoles[ uiFreeListId ] = pHole;

				auto pBlockFooter = TREINTERPRETCAST( MemBlockFooter*, TREINTERPRETCAST( TUINT32, pBlock ) + uiBlockTotalSize ) - 1;
				pBlockFooter->m_pBlockOwner = TNULL;
				pBlockFooter->m_Unk4 = 0;
				pBlockFooter->m_Unk1 = 0;
				pBlockFooter->m_Unk2 = GetSingleton()->m_Unknown1 | 1;
				pBlockFooter->m_pBlockOwner = pBlock;

				pBlock->m_pNextBlock = pBlock;
				pBlock->m_uiTotalSize2 = uiBlockTotalSize;
				TStringManager::String8Copy( pBlock->m_szSignature, "#MB_uID" );
				TStringManager::String8Copy( pBlock->m_szName, a_szName );

				return pBlock;
			}
		}

		return TNULL;
	}

	void TMemory::DestroyMemBlock( MemBlock* a_pMemBlock )
	{
		FreeMemBlock( a_pMemBlock );
		Free( a_pMemBlock );
	}

	TBOOL TMemory::FreeMemBlock( MemBlock* a_pMemBlock )
	{
		TMutexLock lock( ms_pGlobalMutex );

		SetMemBlockUnused( a_pMemBlock );
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead( *a_pMemBlock->m_pSlot );

		return TTRUE;
	}

	void TMemory::SetMemBlockUnused( MemBlock* a_pMemBlock )
	{
		a_pMemBlock->m_pSlot->Remove();
		m_FreeBlocks.InsertHead( *a_pMemBlock->m_pSlot );

		CreateMemBlockInPlace( a_pMemBlock, a_pMemBlock->m_uiTotalSize1, "_unused" );
		TStringManager::String8Copy( a_pMemBlock->m_szSignature, "xxxxxxx" );
	}

	void TMemory::PrintDebug( const TCHAR* a_szFormat, ... )
	{
		va_list args;
		va_start( args, a_szFormat );

		auto pBuffer = TStringManager::GetTempString8();
		vsprintf( pBuffer, a_szFormat, args );
		OutputDebugStringA( pBuffer );
		TTRACE( pBuffer );

		va_end( args );
	}

	TBOOL TMemory::Initialise( TUINT a_uiHeapSize, TUINT a_uiReservedSize )
	{
		auto tmemory = TSTATICCAST( TMemory, calloc( sizeof( TMemory ), 1 ) );
		new ( tmemory ) TMemory();

		tmemory->m_pMemory = TNULL;
		tmemory->m_pGlobalBlock = TNULL;
		tmemory->m_Unknown1 = 0;
		tmemory->m_Unknown2 = 0;

		ms_pGlobalMutex = TSTATICCAST( TMutex, malloc( sizeof( TMutex ) ) );
		new ( ms_pGlobalMutex ) TMutex();

		tmemory->m_ReservedSize = a_uiReservedSize;
		tmemory->m_TotalAllocatedSize = ( a_uiHeapSize == 0 ) ? 128 * 1024 * 1024 : a_uiHeapSize;
		tmemory->m_MainBlockSize = tmemory->m_TotalAllocatedSize - a_uiReservedSize;
		tmemory->m_pMemory = malloc( tmemory->m_TotalAllocatedSize );
		tmemory->m_pGlobalBlock = tmemory->CreateMemBlockInPlace(
			tmemory->m_pMemory,
			tmemory->m_TotalAllocatedSize,
			"Toshi"
		);

		return TTRUE;
	}

	void TMemory::Deinitialise()
	{
		TASSERT( TTRUE == IsSingletonCreated() );
		auto pMemManager = TMemory::GetSingleton();
		auto pMainBlockMemory = pMemManager->m_pMemory;

		pMemManager->m_UsedBlocks.RemoveAll();
		pMemManager->m_FreeBlocks.RemoveAll();
		pMemManager->~TMemory();

		free( pMemManager );
		free( pMainBlockMemory );
	}

	TUINT TMemory::MapSizeToFreeList( TUINT a_uiSize )
	{
		TFLOAT fSize = TFLOAT( TAlignNumUp( a_uiSize ) - 1 );
		TUINT uiExponentSign = ( *(TINT*)&fSize ) >> 23;
		TUINT uiResult = uiExponentSign - 127;

		if ( uiResult & 1 )
			uiResult = uiExponentSign - 126;

		uiResult = uiResult >> 1;

		if ( uiResult != 0 )
			uiResult = uiResult - 1;

		if ( uiResult >= NUM_FREE_LISTS )
			uiResult = NUM_FREE_LISTS - 1;

		return uiResult;
	}

	void TMemory::DumpMemInfo()
	{
		MemInfo memInfo;

		for ( auto it = m_UsedBlocks.Tail(); it != m_UsedBlocks.End(); --it )
		{
			GetMemInfo( memInfo, it->m_pPtr );

			PrintDebug( "Pool: \'%s\'\n", it->m_pPtr->m_szName );
			PrintDebug( "\tLargest Hole    : %d\n", memInfo.m_uiLargestHole );
			PrintDebug( "\tSmallest Hole   : %d\n", memInfo.m_uiSmallestHole );
			PrintDebug( "\tLargest Process : %d\n", memInfo.m_uiLargestProcess );
			PrintDebug( "\tSmallest Process: %d\n", memInfo.m_uiSmallestProcess );
			PrintDebug( "\tTotal Free      : %d\n", memInfo.m_uiTotalFree );
			PrintDebug( "\tTotal Used      : %d\n", memInfo.m_uiTotalUsed );
			PrintDebug( "\tTotal Size      : %d\n", memInfo.m_uiTotalSize );
			PrintDebug( "\tLogic Total Free: %d\n", memInfo.m_uiLogicTotalFree );
			PrintDebug( "\tLogic Total Used: %d\n", memInfo.m_uiLogicTotalUsed );
			PrintDebug( "\tLogic Total Size: %d\n", memInfo.m_uiLogicTotalSize );

			TFLOAT fLogicTotalUsed = TMath::Abs( TFLOAT( memInfo.m_uiLogicTotalUsed ) );
			TFLOAT fLogicTotalSize = TMath::Abs( TFLOAT( memInfo.m_uiLogicTotalSize ) );
			PrintDebug( "\t%%Logical Used   : %f\n", ( fLogicTotalUsed / fLogicTotalSize ) * 100.0 );

			TFLOAT fTotalUsed = TMath::Abs( TFLOAT( memInfo.m_uiTotalUsed ) );
			TFLOAT fTotalSize = TMath::Abs( TFLOAT( memInfo.m_uiTotalSize ) );
			PrintDebug( "\t%%Used\t          : %f\n", ( fTotalUsed / fTotalSize ) * 100.0 );
			PrintDebug( "------\n\n" );
		}
	}

	void TMemory::GetMemInfo( MemInfo& a_rMemInfo, MemBlock* a_pMemBlock )
	{
		TMutexLock lock( ms_pGlobalMutex );

		if ( !a_pMemBlock )
		{
			a_pMemBlock = GetSingleton()->m_pGlobalBlock;
		}

		a_rMemInfo.m_uiUnk3 = 28;
		a_rMemInfo.m_uiUnk4 = 28;
		a_rMemInfo.m_uiTotalSize = 0;
		a_rMemInfo.m_uiLogicTotalSize = 0;
		a_rMemInfo.m_uiLargestHole = 0;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiSmallestHole = 0;
		a_rMemInfo.m_uiSmallestProcess = 0;
		a_rMemInfo.m_uiTotalFree = 0;
		a_rMemInfo.m_uiLogicTotalFree = 0;
		a_rMemInfo.m_uiTotalUsed = 0;
		a_rMemInfo.m_uiLogicTotalUsed = 0;
		a_rMemInfo.m_iNumHoles = 0;
		a_rMemInfo.m_iNumProcesses = 0;
		a_rMemInfo.m_uiTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiLogicTotalSize = a_pMemBlock->m_uiTotalSize1;
		a_rMemInfo.m_uiSmallestProcess = -1;
		a_rMemInfo.m_uiSmallestHole = -1;
		a_rMemInfo.m_uiLargestProcess = 0;
		a_rMemInfo.m_uiLargestHole = 0;

		auto uiUnk = (TUINT)a_pMemBlock->m_pFirstHole + ( -88 - (TUINT)a_pMemBlock );
		a_rMemInfo.m_uiUnk3 = uiUnk;
		a_rMemInfo.m_uiUnk4 = uiUnk;

		auto pHole = a_pMemBlock->m_pFirstHole;
		TUINT uiHoleSize = pHole->m_uiSize;

		while ( TAlignNumDown( uiHoleSize ) != 0 )
		{
			a_rMemInfo.m_uiUnk3 += 12;
			uiHoleSize = TAlignNumDown( pHole->m_uiSize );

			if ( ( pHole->m_uiSize & 1 ) == 0 )
			{
				a_rMemInfo.m_iNumHoles += 1;
				a_rMemInfo.m_uiTotalFree += uiHoleSize;

				if ( a_rMemInfo.m_uiLargestHole <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestHole )
				{
					a_rMemInfo.m_uiLargestHole = uiHoleSize;
				}

				if ( uiHoleSize < a_rMemInfo.m_uiSmallestHole )
				{
					a_rMemInfo.m_uiSmallestHole = uiHoleSize;
				}
			}
			else
			{
				a_rMemInfo.m_iNumProcesses += 1;
				a_rMemInfo.m_uiTotalUsed += uiHoleSize;

				if ( a_rMemInfo.m_uiLargestProcess <= uiHoleSize && uiHoleSize != a_rMemInfo.m_uiLargestProcess )
				{
					a_rMemInfo.m_uiLargestProcess = uiHoleSize;
				}

				if ( uiHoleSize < a_rMemInfo.m_uiSmallestProcess )
				{
					a_rMemInfo.m_uiSmallestProcess = uiHoleSize;
				}
			}

			pHole = (Hole*)( (TUINT)&pHole->m_pPrevHole + uiHoleSize );
			uiHoleSize = pHole->m_uiSize;
		}

		a_rMemInfo.m_uiLogicTotalFree = a_rMemInfo.m_uiTotalFree;
		a_rMemInfo.m_uiLogicTotalUsed = a_rMemInfo.m_uiTotalUsed + uiUnk;

		if ( a_rMemInfo.m_uiSmallestHole == -1 )
		{
			a_rMemInfo.m_uiSmallestHole = 0;
		}
	}

	void TMemory::GetHALMemInfo( HALMemInfo& a_rHALMemInfo )
	{
		TUtil::MemClear( &a_rHALMemInfo, sizeof( a_rHALMemInfo ) );
	}

	TMemory::HALMemInfo::HALMemInfo()
	{
		TUtil::MemClear( this, sizeof( *this ) );
	}
}

void* TMalloc( TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	auto pMemManager = Toshi::TMemory::GetSingleton();

	if ( !a_pMemBlock )
	{
		a_pMemBlock = pMemManager->GetGlobalBlock();
	}

	auto pMem = pMemManager->Alloc( a_uiSize, 16, a_pMemBlock, a_szFileName, a_iLineNum );

	if ( !pMem )
	{
		pMemManager->PrintDebug( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
	}

	return pMem;
}

void* TMalloc( TUINT a_uiSize, const TCHAR* a_szFileName, TINT a_iLineNum )
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc( a_uiSize, 16, pMemBlock, a_szFileName, a_iLineNum );

	if ( !pMem )
	{
		pMemManager->PrintDebug( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void* TMalloc( TUINT a_uiSize )
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc( a_uiSize, 16, pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		pMemManager->PrintDebug( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void* TMemalign( TINT a_iAlignment, TUINT a_uiSize, Toshi::TMemory::MemBlock* a_pMemBlock )
{
	auto pMemManager = Toshi::TMemory::GetSingleton();

	if ( !a_pMemBlock )
	{
		a_pMemBlock = pMemManager->GetGlobalBlock();
	}

	auto pMem = pMemManager->Alloc( a_uiSize, a_iAlignment, a_pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		pMemManager->PrintDebug( "Out of Toshi Memory on block [%s]\n", a_pMemBlock->m_szName );
	}

	return pMem;
}

void* TMemalign( TUINT a_uiSize, TINT a_iAlignment )
{
	auto pMemManager = Toshi::TMemory::GetSingleton();
	auto pMemBlock = pMemManager->GetGlobalBlock();

	auto pMem = pMemManager->Alloc( a_uiSize, a_iAlignment, pMemBlock, TNULL, -1 );

	if ( !pMem )
	{
		pMemManager->PrintDebug( "Out of Toshi Memory on block [%s]\n", pMemBlock->m_szName );
	}

	return pMem;
}

void TFree( void* a_pMem )
{
	Toshi::TMemory::GetSingleton()->Free( a_pMem );
}
