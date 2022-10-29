#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Strings/TString8.h"
#include "Toshi/Core/TGenericDList.h"

#ifdef CreateFile
#undef CreateFile
#endif

namespace Toshi
{
	class TFile;

	// JPOG Matched
	class TFileSystem : public TDList<TFileSystem>::TNode
	{
	public:
		TFileSystem(const char* name);
		TFileSystem(const TFileSystem& other);
		virtual ~TFileSystem() { UnmountFileSystem(); }

		// Override these funcs in TNativeFileSystem
		virtual TFile* CreateFile(TString8 const& fn, uint32_t flags) = 0;
		virtual void DestroyFile(TFile*) = 0;
		virtual bool RemoveFile(TString8 const&) { return true; }
		virtual TString8 MakeInternalPath(TString8 const&) = 0;
		virtual bool GetFirstFile(TString8 const&, TString8&, unsigned int) { return false; }
		virtual bool GetNextFile(TString8&, uint32_t) { return false; }
		virtual void SetPrefix(const TString8& prefix);
		virtual bool MakeDirectory(TString8 const&) = 0;

		inline void UnmountFileSystem() { TNode::Remove(); }

		inline TString8 const& GetName() const { return m_Name; }
		inline TString8 const& GetPrefix() const { return m_Prefix; }
		
		inline void RemoveNode()       { TNode::Remove(); }
		inline bool IsLinked()         { return TNode::IsLinked(); }
		inline TFileSystem* PrevNode() { return TNode::Prev()->As<TFileSystem>(); }
		inline TFileSystem* NextNode() { return TNode::Next()->As<TFileSystem>(); }

		TFileSystem& operator=(TFileSystem& a_rFileSystem);

	protected:
		TString8 m_Name;   // 0xC
		TString8 m_Prefix; // 0x18
		HANDLE m_Handle;   // 0x1C
	};

	class TNativeFileSystem : public TFileSystem
	{
	public:
		TNativeFileSystem(const char* name);

		virtual TFile* CreateFile(TString8 const& fn, uint32_t flags) override;
		virtual void DestroyFile(TFile*) override;
		virtual TString8 MakeInternalPath(TString8 const&) { return {}; }
		virtual bool MakeDirectory(TString8 const&) override;
		virtual bool GetNextFile(TString8& fileName, uint32_t flags);
	};

	class TFile
	{
	public:
		enum TSEEK
		{
			TSEEK_SET,
			TSEEK_CUR,
			TSEEK_END
		};

		enum OpenFlags
		{
			OpenFlags_Read = BITFIELD(0),
			OpenFlags_Write = BITFIELD(1),
			OpenFlags_ReadWrite = OpenFlags_Read | OpenFlags_Write,
			OpenFlags_CreateNew = BITFIELD(2),
			OpenFlags_NoBuffer = BITFIELD(3),
		};

		TFile(TFileSystem* pFS);
		TFile(const TFile& other);

		virtual size_t Read(LPVOID, size_t) = 0;    //0x0
		virtual int Write(LPVOID, size_t) = 0;      //0x4
		virtual bool Seek(int, TFile::TSEEK) = 0;   //0x8
		virtual uint32_t Tell() = 0;                //0xC
		virtual DWORD GetSize() = 0;                //0x10
		virtual _FILETIME GetDate() { return {}; }  //0x14
		virtual char GetCChar() = 0;
		virtual wchar_t GetWChar() = 0;
		virtual int PutCChar(char character) = 0;
		virtual int PutWChar(wchar_t character) = 0;
		virtual int CPrintf(const char* format, ...) = 0;
		virtual int VCPrintf(const char* format, ...) = 0;
		virtual int VWPrintf(const wchar_t* format, ...) = 0;

		static TString8 ConcatPath(const TString8& a_rcString, const TString8& a_rcString2);
		static TFile* Create(const TString8& filename, uint32_t flags);
		inline TFileSystem* GetFileSystem() const { return m_pFileSystem; }
		inline TFile& operator=(const TFile& a_pFile) { m_pFileSystem = a_pFile.GetFileSystem(); return *this; }

		// FUN_00685f60
		inline void Destroy() { m_pFileSystem->DestroyFile(this); }

	private:
		TFileSystem* m_pFileSystem;
	};

	class TFileManager : public TSingleton<TFileManager>
	{
	public:
		class TSysPathIter
		{
		public:
			TSysPathIter(const TString8& str) : m_String(str), m_Position(-1) { };
			TSysPathIter(const TSysPathIter& other) : m_String(other.m_String), m_Position(other.m_Position) { };

			bool First(TString8& path)
			{
				if (m_String.Length() > 0)
				{
					m_Position = m_String.Find(';', 0);
					path.Copy(m_String, m_Position);
					
					return true;
				}
				else
				{
					m_Position = -1;
					return false;
				}
			}

			bool Next(TString8& path)
			{
				if (m_Position >= 0)
				{
					int strStart = m_Position + 1;
					m_Position = m_String.Find(';', strStart);

					path.Copy(
						m_String.GetString(strStart),
						(m_Position >= 0) ? (m_Position - strStart) : -1
					);

					return true;
				}
				else
				{
					return false;
				}
			}

		private:
			const TString8& m_String;
			int32_t m_Position;
		};
	
	public:
		TFileManager() : m_WorkingDirectory("/"), m_ValidatedCount(0), m_Mutex() { InvalidateSystemPath(); }
		~TFileManager() { Destroy(); }

		void Destroy();
		void MountFileSystem(TFileSystem* a_pFileSystem);
		
		TFile* CreateFile(const TString8& a_sName, uint32_t flags);

		TFileSystem* FindFileSystem(const TString8& name);
		static TFileSystem* FindFileSystem(TDList<TFileSystem>& list, const TString8& name);

		inline TString8 MakeAbsolutePath(const TString8& a_cString) const { return TFile::ConcatPath(a_cString, m_WorkingDirectory); }
		inline void FileSystemRelease() { m_Mutex.Unlock(); }
		inline void FileSystemWait() { m_Mutex.Lock(); }

		inline void SetSystemPath(const TString8& name) { m_SysPath = name; InvalidateSystemPath(); }

		static void DestroyFile(TFile* pFile)
		{
			if (pFile)
			{
				pFile->Destroy();
			}
		}

		static void CreateCommon()
		{
			auto fileManager = TFileManager::CreateSingleton();
			fileManager->m_Mutex.Create();
		}

		static void DestroyCommon() { delete TFileManager::GetSingleton(); }

	public:
		/*
		* Platform specific methods
		* Define them in TNativeFile_{Platform}.cpp
		*/

		static bool Create();

	private:
		void ValidateSystemPath();
		inline void InvalidateSystemPath() { m_IsValidated = false; }

	private:
		bool m_IsValidated;                // 0x0
		TString8 m_SysPath;                // 0x4
		TString8 m_WorkingDirectory;       // 0x10
		uint32_t m_ValidatedCount;         // 0x1C
		TDList<TFileSystem> m_Validated;   // 0x20
		TDList<TFileSystem> m_Invalidated; // 0x28
		T2Mutex m_Mutex;                   // 0x30
	};

	class TNullFileSystem : public TFileSystem, public TSingleton<TNullFileSystem>
	{
	public:
		TNullFileSystem(const char* name) : TFileSystem(name)
		{
			TFileManager::GetSingleton()->MountFileSystem(this);
		}

		// Inherited via TFileSystem
		virtual TFile* CreateFile(TString8 const& fn, uint32_t flags) override;
		virtual void DestroyFile(TFile*) override;
		virtual TString8 MakeInternalPath(TString8 const&) override;
		virtual bool MakeDirectory(TString8 const&) override;
	};
}

