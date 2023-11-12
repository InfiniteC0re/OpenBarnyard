#pragma once
#ifndef __TOSHI_TTRB_H__
#define __TOSHI_TTRB_H__
#endif

#include "Toshi/Strings/TString8.h"
#include "Toshi/File/TFile.h"
#include "Toshi/File/TCompress_Decompress.h"
#include "Toshi/File/TCompress_Compress.h"

#ifdef ERROR
#undef ERROR
#endif

namespace Toshi {

	class TFORM
	{
	public:
		TUINT32 Unk;
	};

	class TTSF
	{
	public:
		static constexpr TUINT32 IDMAGICB = TMAKEFOUR("TSFB");
		static constexpr TUINT32 IDMAGICL = TMAKEFOUR("TSFL");

		typedef uint8_t Endianess;
		enum Endianess_ : Endianess
		{
			Endianess_Little,
			Endianess_Big,
		};

		struct Header
		{
			TUINT32 Magic;
			TUINT32 FileSize;
		};

		struct FileInfo
		{
			TUINT32 FileStartOffset; // offset to TRBF
			TUINT32 FileSize;        // just the size
		};

		struct Hunk
		{
			TUINT32 Name;
			TUINT32 Size;
		};

	public:
		TTSF() : m_pFile(TNULL) { }

	protected:
		Endianess m_Endianess; // 0x0
		TFile* m_pFile;        // 0x4
	};

	class TTSFI : public TTSF
	{
	public:
		friend class TTRB;

	public:
		TTSFI();
		~TTSFI() { Close(); }

		TUINT32 ReadAlignmentPad()
		{
			TASSERT(m_pFile != TNULL, "File is TNULL");
			static char s_AlignBuffer[4] = { 0, 0, 0, 0 };
			uint8_t alignValue = 4 - (m_pFile->Tell() & 3);

			if (alignValue != 4)
			{
				return m_pFile->Read(s_AlignBuffer, alignValue);
			}

			return 0;
		}

		TUINT32 Open(TFile* a_pFile);
		TUINT32 Open(const char* a_szFilePath);

		TUINT32 PushForm();
		TUINT32 PopForm();

		template<class T>
		void Read(T* dst)
		{
			m_ReadPos += m_pFile->Read(dst, sizeof(T));
		}

		void ReadRaw(void* dst, TUINT32 size)
		{
			m_ReadPos += m_pFile->Read(dst, size);
		}

		// Sections related stuff
		TUINT8 ReadHunk();
		TUINT8 SkipHunk();
		TUINT8 ReadFORM(TFORM* section);
		TUINT8 ReadHunkData(void* dest);

		void Close(TBOOL free = TTRUE);

		void ReadCompressed(void* buffer, TUINT32 size);
		void CompressSection(TFile* file, char* unk, TUINT32 unk2, TUINT32 unk3) { TCompress_Compress::Compress(file, unk, unk2, unk3, m_Endianess); }

		void LogUnknownSection();

		const Hunk& GetCurrentHunk() { return m_CurrentHunk; }

	private:
		TUINT32 m_FileInfoCount; // 0x8
		FileInfo m_FileInfo[32]; // 0xC
		Header m_Header;         // 0x10C
		TUINT32 m_Magic;         // 0x114
		Hunk m_CurrentHunk;      // 0x118
		TBOOL m_UNKFLAG;         // 0x120
		TUINT32 m_ReadPos;       // 0x124
	};

	class TTSFO : public TTSF
	{
	public:
		typedef uint8_t ERROR;
		enum ERROR_ : ERROR
		{
			ERROR_OK,
			ERROR_UNKNOWN,
			ERROR_FILE
		};

		struct HunkMark
		{
			TUINT32 Name;
			TUINT32 Pos;
		};

	public:
		TTSFO() : m_PositionCount(0), m_Positions() { }
		~TTSFO() { Close(); }

		/**
		* Aligns current position to 4.
		*
		* @return number of written bytes
		*/
		TUINT32 WriteAlignmentPad()
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			static char s_AlignBuffer[4] = { 0, 0, 0, 0 };
			uint8_t alignValue = 4 - (m_pFile->Tell() & 3);

			if (alignValue != 4)
			{
				return m_pFile->Write(s_AlignBuffer, alignValue);
			}

			return 0;
		}

		/**
		* Creates file for writing.
		*
		* @param filepath - path to the file
		* @param magic - magic value which is placed after the first hunk
		* @return TTRB::ERROR_OK if successful
		*/
		TTSFO::ERROR Create(const char* filepath, const char* magic = "TRBF", Endianess endianess = Endianess_Little);

		/**
		* Closes the file.
		*
		*/
		void Close();

		/**
		* Begin a new form and saves it's info
		*
		* @param name - 4 bytes long value which will be written after the hunk
		* @return number of written bytes
		*/
		TUINT32 BeginForm(const char* name);

		/**
		* Ends the current form if it exists.
		*
		* @return size of the form
		*/
		TUINT32 EndForm();

		/**
		* Opens new hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be opened
		* @param hunkName - 4 bytes long name of the hunk
		* @return always TTRUE
		*/
		TBOOL OpenHunk(HunkMark* hunkMark, const char* hunkName);

		/**
		* Closes the hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be closed
		* @return always TTRUE
		*/
		TBOOL CloseHunk(HunkMark* hunkMark);

		/**
		* Writes a hunk of data.
		*
		* @param hunkName - magic value that is used to identificate hunk (use TMAKEFOUR to get it)
		* @param buffer - pointer to the buffer of data (can be TNULL)
		* @param bufferSize - size of the buffer (can be 0)
		* @return number of written bytes
		*/
		TUINT32 WriteHunk(TUINT32 hunkName, void* buffer = TNULL, TUINT32 bufferSize = 0);

		template <class T>
		TUINT32 Write(const T& value)
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			return m_pFile->Write(&value, sizeof(T));
		}

		TUINT32 WriteRaw(const void* buffer, TUINT32 size)
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			return m_pFile->Write(buffer, size);
		}

		TUINT32 WriteCompressed(const void* buffer, TUINT32 size)
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			TUINT32 writtenSize = TCompress_Compress::Compress(m_pFile, (char*)buffer, size, 0, m_Endianess == Endianess_Big);
			WriteAlignmentPad();
			return writtenSize;
		}

		void WriteBool(TBOOL value) { Write(value); }
		void WriteInt8(int8_t value) { Write(value); }
		void WriteUInt8(uint8_t value) { Write(value); }
		void WriteUInt16(TUINT16 value) { Write(value); }
		void WriteInt32(int32_t value) { Write(value); }
		void WriteUInt32(TUINT32 value) { Write(value); }
		void WriteFloat(float value) { Write(value); }
		void WriteVector3(TVector3& value) { Write(value); }
		void WriteVector4(TVector4& value) { Write(value); }
		void WriteQuaternion(TQuaternion& value) { Write(value); }

	private:
		TUINT32 m_PositionCount;
		TUINT32 m_Positions[32];
	};

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

		using t_MemoryFuncAlloc   = void*(*)(AllocType alloctype, TUINT32 size, TINT16 unk1, TUINT32 unk2, void* userData);
		using t_MemoryFuncDealloc = void (*)(AllocType alloctype, void* ptr, TINT16 unk1, TUINT32 unk2, void* userData);

		struct SecInfo
		{
			char m_Unused[2]; // 0x0 (padding)
			TINT16 m_Unk1;    // 0x2
			TUINT32 m_Size;   // 0x4
			void* m_Data;     // 0x8
			TUINT32 m_Unk2;   // 0xC
		};
		
		struct Header
		{
			TVersion m_ui32Version;
			int32_t m_i32SectionCount;
		};

		struct RELCEntry
		{
			TINT16 HDRX1;   // HDRX of pointer
			TINT16 HDRX2;   // HDRX of data
			TUINT32 Offset; // Offset to pointer in HDRX1
		};

		struct TTRBSymbol
		{
			TUINT16 HDRX;
			TUINT16 NameOffset;
			TUINT16 Padding;
			TINT16 NameHash;
			TUINT32 DataOffset;
		};

		struct SYMB
		{
			int32_t m_i32SymbCount;
		};

	public:
		TTRB();
		~TTRB() { Close(); }

		// Creates TFile and reads TRB from it
		ERROR Load(const char* a_szFilePath, TUINT32 a_uiUnknown = 0);

		// Returns index of TTRBSymbol
		int GetSymbolIndex(const char* symbName);

		// Returns pointer to data if found and TNULL if not
		void* GetSymbolAddress(const char* symbName);
		void* GetSymbolAddress(TTRBSymbol& symb) { return static_cast<char*>(GetSection(symb.HDRX)) + symb.DataOffset; }

		// Destroys TRB file and the content
		void Close();

		static void SetDefaultMemoryFuncs(t_MemoryFuncAlloc allocator, t_MemoryFuncDealloc deallocator, void* allocatorUserData);

		template<typename T>
		T* CastSymbol(const char* symbName)
		{
			return TSTATICCAST(T*, GetSymbolAddress(symbName));
		}

		static TUINT32 GetSymbolTableSize(TUINT32 count)
		{
			return sizeof(TTRBSymbol) * count + sizeof(SYMB);
		}

		static TUINT32 GetHeaderSize(TUINT32 count)
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
			else if (index < m_SYMB->m_i32SymbCount)
			{
				return reinterpret_cast<TTRBSymbol*>(m_SYMB + 1) + index;
			}

			return TNULL;
		}

		TTRBSymbol* GetSymbol(char const* a_symbolName)
		{
			int index = GetSymbolIndex(a_symbolName);
			if (index == -1)
			{
				return TNULL;
			}

			return GetSymbol(index);
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

		SYMB* GetSymbolTable() const { return m_SYMB; }

		static TINT16 HashString(const char* str)
		{
			TINT16 hash = 0;
			char character;

			while (*str != '\0')
			{
				character = *(str++);
				hash = (TINT16)character + hash * 0x1f;
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
		TBOOL ProcessForm(TTSFI& ttsf);

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
		static void* s_pDefAllocatorUserData;

	private:
		TTSFI m_TTSFI;
		TUINT32 m_UNK;
		Header* m_pHeader;
		SYMB* m_SYMB;
		t_MemoryFuncAlloc m_MemAllocator;
		t_MemoryFuncDealloc m_MemDeallocator;
		void* m_MemUserData;
	};

}


