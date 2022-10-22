#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/Toshi2/T2String8.h"
#include "Toshi/File/TFile.h"

#undef ERROR

namespace Toshi
{
	class TTSFI;

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

		typedef uint8_t AllocType;
		enum AllocType_ : AllocType
		{
			AllocType_Unk0 = 0,
			AllocType_Unk1 = 1,
			AllocType_Unk2 = 2,
		};

		using t_MemoryFuncAlloc   = void*(*)(AllocType alloctype, size_t size, short unk1, size_t unk2, void* userData);
		using t_MemoryFuncDealloc = void (*)(AllocType alloctype, void* ptr, short unk1, size_t unk2, void* userData);

		struct SecInfo
		{
			char m_Unused[2];  // 0x0 (padding)
			short m_Unk1;      // 0x2
			uint32_t m_Size;   // 0x4
			void* m_Data;      // 0x8
			uint32_t m_Unk2;   // 0xC
		};
		
		struct Header
		{
			uint32_t m_ui32Version;
			int32_t m_i32SectionCount;
		};

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

		struct TTRBSymbol
		{
			uint16_t HDRX;
			uint16_t NameOffset;
			uint16_t Padding;
			uint16_t NameHash;
			uint32_t DataOffset;
		};

		struct SYMB
		{
			int32_t m_i32SymbCount;
		};

	public:
		TTRB();
		~TTRB() { Close(); }

		// Creates TFile and reads file
		ERROR Load(const char* path);

		// Reads TRB from TFile
		ERROR Load(TFile* file);

		// Returns index of TTRBSymbol
		int GetSymbolIndex(const char* symbName);

		// Returns pointer to data if found and TNULL if not
		void* GetSymbolAddress(const char* symbName);

		// Destroys TRB file and the content
		void Close();

		template<typename T>
		T* CastSymbol(const char* symbName)
		{
			return static_cast<T*>(GetSymbolAddress(symbName));
		}

		static size_t GetSymbolTableSize(size_t count)
		{
			return sizeof(TTRBSymbol) * count + sizeof(SYMB);
		}

		static size_t GetHeaderSize(size_t count)
		{
			return sizeof(SecInfo) * count + sizeof(Header);
		}

		SecInfo* GetSectionInfoList() const
		{
			return reinterpret_cast<SecInfo*>(m_pHeader + 1);
		}

		SecInfo* GetSectionInfo(int index) const
		{
			return GetSectionInfoList() + index;
		}

		TTRBSymbol* GetSymbol(int index) const
		{
			if (m_SYMB == TNULL)
			{
				return TNULL;
			}
			if (index < m_SYMB->m_i32SymbCount)
			{
				return reinterpret_cast<TTRBSymbol*>(m_SYMB + 1) + index;
			}

			return TNULL;
		}

		const char* GetSymbolName(int index) const
		{
			if (m_SYMB == TNULL)
			{
				return TNULL;
			}

			return GetSymbolName(reinterpret_cast<TTRBSymbol*>(m_SYMB + 1) + index);
		}

		const char* GetSymbolName(TTRBSymbol* symbol) const
		{
			if (m_SYMB == TNULL)
			{
				return TNULL;
			}

			return reinterpret_cast<const char*>(
				reinterpret_cast<uintptr_t>(m_SYMB) +
				GetSymbolTableSize(m_SYMB->m_i32SymbCount) +
				symbol->NameOffset
			);
		}

		static short HashString(const char* str)
		{
			short hash = 0;
			char character;

			while (*str != '\0')
			{
				character = *(str++);
				hash = (short)character + hash * 0x1f;
			}

			return hash;
		}

		void SetMemoryFunctions(t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* userdata)
		{
			// 00686e00
			m_MemAllocator = allocator;
			m_MemDeallocator = deallocator;
			m_MemUserData = userdata;
		}

	private:
		// Parses sections
		bool ProcessForm(TTSFI& ttsf);

		// Returns pointer to section by index
		inline void* GetSection(int index) { return GetSectionInfo(index)->m_Data; }
		
		void DeleteSymbolTable()
		{
			if (m_SYMB != TNULL)
			{
				m_MemDeallocator(AllocType_Unk2, m_SYMB, 0, 0, m_MemUserData);
				m_SYMB = TNULL;
			}
		}

	private:
		static t_MemoryFuncAlloc s_cbDefAllocator;
		static t_MemoryFuncDealloc s_cbDefDeallocator;
		static void* s_pDefAllocatorUserData;

	private:
		Header* m_pHeader;                    // 0x00
		SYMB* m_SYMB;                         // 0x04
		t_MemoryFuncAlloc m_MemAllocator;     // 0x08
		t_MemoryFuncDealloc m_MemDeallocator; // 0x0C
		void* m_MemUserData;                  // 0x10
	};
}


