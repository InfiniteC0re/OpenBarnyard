#pragma once

#include "Toshi/Toshi2/T2String8.h"

class EnSaveData
{
public:

	const int SAVEGAME_SLOT_COUNT = 3;

	// Size should be 0x318 or 792 in decimal
	struct SaveGameSlot
	{
		bool m_bUsedSlot; // should be 0x0 not sure
	};

	struct SaveData
	{
		struct Header
		{
			char m_id[4]; // 0x0 "BB"
			uint32_t m_size; // 0x4
			int m_crc; // 0x8
		} header;
		
		SaveGameSlot* m_SaveGameSlots; // 0xC
	};

	unsigned char* m_buffer;
	int m_size;
	SaveData* m_savedata; // 0xC

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

	static uint32_t s_aiCRC32LUT[CRC_TABSIZE];

	bool Validate();
	static void GenerateCRC();
	static uint32_t CalculateCRC(unsigned char* buffer, int size);
};

