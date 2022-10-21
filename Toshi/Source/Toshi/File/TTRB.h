#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/Toshi2/T2String8.h"
#include "Toshi/File/TFile.h"

#undef ERROR

namespace Toshi
{
	class TTSF;

	class TTRB
	{
	public:
		typedef uint8_t ERROR;

		enum ERROR_ : ERROR
		{
			ERROR_OK                   = 0,
			ERROR_NO_HEADER            = 1,
			ERROR_NOT_TRBF             = 2,
			ERROR_PARSE_ERROR          = 3,
			ERROR_WRONG_MAGIC          = 4,
			ERROR_FORM_MAGIC           = 5,
			ERROR_NO_FILE              = 6,
			ERROR_NOT_TRB              = 7,
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

			inline SecInfo* GetSecInfo(int index = 0)
			{
				TASSERT(index < m_i32SectionCount, "Index is out of bounds");
				return &reinterpret_cast<SecInfo*>(this + 1)[index];
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
		TTRB() : m_pHeader(nullptr), m_SYMB(nullptr) { }
		~TTRB() { Close(); }

		// Creates TFile and reads file
		ERROR Open(const char* path);

		// Reads TRB from TFile
		ERROR Open(TFile* file);

		// Returns pointer to data if found and TNULL if not
		void* FindSymb(const char* symbName);

		template<typename T>
		inline T* GetSymb(const char* symbName) { return static_cast<T*>(FindSymb(symbName)); }
		
		// Destroys TRB file and the content
		void Close();

	private:
		// Parses sections
		bool ProcessForm(TTSF& ttsf);

		// Destroys section by index
		void DestroySection(int index);

		// Returns index of SYMBEntry
		int FindSymbIndex(const char* symbName);

		// Returns pointer to section by index
		inline void* GetSection(int index) { return m_pHeader->GetSecInfo(index)->m_pData; }
	
	private:
		Header* m_pHeader;          // 0x0
		SYMB* m_SYMB;               // 0x4
	};
}


