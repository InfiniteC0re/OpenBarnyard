#pragma once
class EnSaveData
{
public:
	// Size should be 0x318 or 792 in decimal
	struct SaveGameSlot
	{
		bool m_bUsedSlot; // should be 0x0 not sure
	};

	struct SaveData
	{
		struct Header
		{
			char m_id[2]; // 0x0
			short padding; // 0x2
			uint32_t m_size; // 0x4
			int m_crc; // 0x8
		} header;
		
		SaveGameSlot* m_SaveGameSlots; // 0xC
	};

	char* m_buffer;
	int m_size;
	SaveData* m_savedata; // 0xC


	/* ------------------------------------------------------------------------- */
	/* CRC-32 CCITT                                                              */
	/* ------------------------------------------------------------------------- */
	#define CRC32POLY      (0xEDB88320L)    /* Generator polynomial number       */
	#define CRC32INIT      (0xFFFFFFFFL)    /* Initial CRC value for calculation */
	#define CRC32POST(crc) (~(crc))         /* CRC Postconditioning before xmit  */
	#define CRC32TEST      (0xDEBB20E3L)    /* Result to test for at receiver    */

	#define crc32upd(crctab,crc,c) \
                  ((crctab)[((int) (crc) ^ (c)) & 0xff] ^ ((crc) >> 8))

	static uint32_t CRCTable[0x100];

	bool Validate();
	static void GenerateCRC();
	static uint32_t CalculateCRC(unsigned char* buffer, int size);
};

