#pragma once
#include "Toshi/File/TCompress_Decompress.h"
#include "Toshi/File/TCompress_Compress.h"
#include "Toshi/File/TTRB.h"

namespace Toshi
{
	class TTSF
	{
	public:
		static constexpr uint32_t IDMAGICB = TMAKEFOUR("TSFB");
		static constexpr uint32_t IDMAGICL = TMAKEFOUR("TSFL");

		typedef uint8_t Endianess;
		enum Endianess_ : Endianess
		{
			Endianess_Little,
			Endianess_Big,
		};

		struct Header
		{
			uint32_t Magic;
			uint32_t FileSize;
		};

		struct FileInfo
		{
			uint32_t FileStartOffset; // offset to TRBF
			uint32_t FileSize;        // just the size
		};

		struct Hunk
		{
			uint32_t Name;
			uint32_t Size;
		};

	public:
		TTSF() : m_Endianess(Endianess_Little), m_pFile(TNULL) { }

	protected:
		Endianess m_Endianess; // 0x0
		TFile* m_pFile;        // 0x4
	};

	class TTSFI : public TTSF
	{
	public:
		friend TTRB;

	public:
		TTSFI();
		~TTSFI() { Close(); }

		size_t ReadAlignmentPad()
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
		
		TTRB::ERROR Open(TFile* a_pFile);

		TTRB::ERROR PushForm();
		TTRB::ERROR PopForm();

		// 0x00688250
		inline void ReadRaw(void* dest, uint32_t size) { m_ReadPos += m_pFile->Read(dest, size); }

		// Sections related stuff
		TTRB::ERROR ReadHunk();
		TTRB::ERROR SkipHunk();
		TTRB::ERROR ReadFORM(TTRB::SectionFORM* section);
		TTRB::ERROR ReadHunkData(void* dest);

		void Close(bool free = true);

		void ReadCompressed(void* buffer, uint32_t size);
		inline void CompressSection(TFile* file, char* unk, uint32_t unk2, uint32_t unk3) { TCompress_Compress::Compress(file, unk, unk2, unk3, m_Endianess); }

		void LogUnknownSection();

	private:
		uint32_t m_FileInfoCount; // 0x8
		FileInfo m_FileInfo[32];  // 0xC
		Header m_Header;          // 0x10C
		uint32_t m_Magic;         // 0x114
		Hunk m_CurrentSection;    // 0x118
		uint32_t m_ReadPos;       // 0x120
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
			uint32_t Name;
			uint32_t Pos;
		};

	public:
		TTSFO() : m_PositionCount(0), m_Positions() { }
		~TTSFO() { Close(); }

		/**
		* Aligns current position to 4.
		*
		* @return number of written bytes
		*/
		size_t WriteAlignmentPad()
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
		size_t BeginForm(const char* name);

		/**
		* Ends the current form if it exists.
		* 
		* @return size of the form
		*/
		size_t EndForm();

		/**
		* Opens new hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be opened
		* @param hunkName - 4 bytes long name of the hunk
		* @return always true
		*/
		bool OpenHunk(HunkMark* hunkMark, const char* hunkName);

		/**
		* Closes the hunk.
		*
		* @param hunkMark - pointer to TTSFO::HunkMark which has to be closed
		* @return always true
		*/
		bool CloseHunk(HunkMark* hunkMark);

		/**
		* Writes a hunk of data.
		*
		* @param hunkName - magic value that is used to identificate hunk (use TMAKEFOUR to get it)
		* @param buffer - pointer to the buffer of data (can be TNULL)
		* @param bufferSize - size of the buffer (can be 0)
		* @return number of written bytes
		*/
		size_t WriteHunk(uint32_t hunkName, void* buffer = TNULL, size_t bufferSize = 0);

		template <class T>
		size_t Write(const T& value)
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			return m_pFile->Write(&value, sizeof(T));
		}
		
		size_t WriteRaw(void* buffer, size_t size)
		{
			TASSERT(m_pFile != TNULL, "TTSFO is not created");
			return m_pFile->Write(buffer, size);
		}
		
		void WriteBool(bool value)               { Write(value); }
		void WriteInt8(int8_t value)             { Write(value); }
		void WriteUInt8(uint8_t value)           { Write(value); }
		void WriteUInt16(uint16_t value)         { Write(value); }
		void WriteInt32(int32_t value)           { Write(value); }
		void WriteUInt32(uint32_t value)         { Write(value); }
		void WriteFloat(float value)             { Write(value); }
		void WriteVector3(TVector3& value)       { Write(value); }
		void WriteVector4(TVector4& value)       { Write(value); }
		void WriteQuaternion(TQuaternion& value) { Write(value); }

	private:
		size_t m_PositionCount;
		size_t m_Positions[32];
	};
}