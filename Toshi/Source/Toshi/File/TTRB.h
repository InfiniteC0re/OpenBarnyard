#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/Toshi2/T2String8.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TTSF;

	class TTRB
	{
	public:
		enum TTRB_ERROR : uint8_t
		{
			ERROR_OK = 0,
			ERROR_WRONG_MAGIC = 4,
			ERROR_FORM_MAGIC = 5,
			ERROR_NO_FILE = 6,
			ERROR_NOT_TRB = 7,
			ERROR_NO_FILEINFO_ON_STACK = 8,
		};

#pragma pack(push, 1)
		struct SecInfo         // HDRX
		{
			char m_Unused[2];  // Padding 0x0
			short m_Unk1;      // ? 0x2
			uint32_t m_Size;   // Size of section 0x4
			void* m_pData;     // 0x8
			uint32_t m_Unk2;   // 0xC
		};
		
		struct Header
		{
			uint32_t m_ui32Version;     // 0x0
			int32_t m_i32SectionCount;  // 0x4

			inline SecInfo& operator[](int index)
			{
				TASSERT(index < m_i32SectionCount, "Index is out of bounds");
				return reinterpret_cast<SecInfo*>(this + 1)[index];
			}
		};

#pragma pack(pop)

		struct RELCEntry
		{
			short HDRX1;
			short HDRX2;
			uint32_t Offset;
		};

		struct SectionFORM
		{
			uint32_t Unk;
		};

		struct SYMBEntry
		{
			uint16_t HDRX;
			uint16_t Type_Offset;
			uint16_t padding;
			uint16_t Type_Hash;
			uint32_t Data_Offset;
		};

		struct SYMB
		{
			int32_t m_i32SymbCount;

			// Returns SYMBEntry by index
			inline SYMBEntry& operator[](int index)
			{
				TASSERT(index < m_i32SymbCount, "Index is out of bounds");
				return reinterpret_cast<SYMBEntry*>(this + 1)[index];
			}

			// Returns pointer to the first SYMBEntry's name
			inline const char* Names()
			{
				return reinterpret_cast<const char*>(
					&reinterpret_cast<SYMBEntry*>(this + 1)[m_i32SymbCount]
				);
			}
		};

	public:
		TTRB() = default;
		inline ~TTRB() { Destroy(); }

		// Opens and reads trb file from path
		void Open(const char* path);

		// Returns pointer to data if found and TNULL if not
		void* FindSymb(const char* symbName);
		
		// Destroys TRB file and the content
		void Destroy();

	private:
		// Reads TRB and parses it
		void Read(TFile* file);

		// Parses sections
		bool Parse(TTSF& ttsf);

		// Destroys section by index
		void DestroySection(int index);

		// Returns index of SYMBEntry
		int FindSymbIndex(const char* symbName);

		// Returns pointer to section by index
		inline void* GetSection(int index) { return m_pHeader->operator[](index).m_pData; }
	
	private:
		Header* m_pHeader;          // 0x0
		SYMB* m_SYMB;               // 0x4
	};
}


