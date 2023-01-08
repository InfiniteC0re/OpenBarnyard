#pragma once
#include <cstdint>

#include "Toshi/Utils/TSingleton.h"

namespace Toshi
{
	class TUtil : public TSingleton<TUtil>
	{
	public:
		static bool ToshiCreate(int argc, char** argv, TMemory& memorySettings);
		
		static void ToshiDestroy()
		{
			ToshiDestroySubsystems();
			ToshiDestroyMemory();
		}
		
		static void ToshiDestroySubsystems()
		{
			TLog::Destroy();
			// ....
		}

		static void ToshiDestroyMemory()
		{
			TMemory::Shutdown();
		}
		
		static uint64_t GetUnixSeconds(uint64_t* pOut = nullptr);
		static void MemSet(void* ptr, size_t value, size_t size);
		
		static void* MemCopy(void* dst, const void* src, size_t size)
		{
			return memcpy(dst, src, size);
		}

		static void MemClear(void* ptr, size_t size)
		{
			memset(ptr, 0, size);
		}

		static int MemCompare(void* ptr1, void* ptr2, int size)
		{
			return memcmp(ptr1, ptr2, size);
		}
		
#pragma region CRC

		// Source: https://lentz.com.au/blog/tag/crc-table-generator

		/*
			LICENCE

			This package may be freely distributed provided the files remain together,
			in their original unmodified form.
			All files, executables and sourcecode remain the copyrighted property of
			Arjen G. Lentz and LENTZ SOFTWARE-DEVELOPMENT.
			Licence for any use granted, provided this notice & CRC.DOC are included.
			For executable applications, credit should be given in the appropriate
			places in the program and documentation.
			These notices must be retained in any copies of any part of this
			documentation and/or software.

			Any use of, or operation on (including copying/distributing) any of
			the above mentioned files implies full and unconditional acceptance of
			this licence and disclaimer.
		*/

		/* ------------------------------------------------------------------------- */
		/* CRC-32 CCITT                                                              */
		/* ------------------------------------------------------------------------- */
		#define CRC32POLY      (0xEDB88320L)    /* Generator polynomial number       */
		#define CRC32POST(crc) (~(crc))         /* CRC Postconditioning before xmit  */

		#define crc32upd(crctab,crc,c) \
					  ((crctab)[((int) (crc) ^ (c)) & 0xff] ^ ((crc) >> 8))

		#define CRC_TABSIZE (256)               /* Normal 256-entry table            */

		//=============================================================================

		inline static uint32_t s_aiCRC32LUT[CRC_TABSIZE] = {};

		static void CRCInitialise();
		static uint32_t CRC32(unsigned char* buffer, uint32_t len);

#pragma endregion

	};
}