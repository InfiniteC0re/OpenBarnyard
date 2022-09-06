#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Strings/TCString.h"

#ifdef CreateFile
#undef CreateFile
#endif
#include <Toshi/Core/TGenericDList.h>

namespace Toshi
{
	class TFile;

	// JPOG Matched
	class TFileSystem
	{
	public:
		TFileSystem(TFileSystem const&);
		TFileSystem(const char*);
		virtual ~TFileSystem();

		// Override these funcs in TNativeFileSystem
		virtual TFile* CreateFile(TCString const& fn, uint32_t flags) = 0;
		virtual void DestroyFile(TFile*) = 0;
		virtual bool RemoveFile(TCString const&) { return true; }
		virtual TCString MakeInternalPath(TCString const&) = 0;
		virtual bool GetFirstFile(TCString const&, TCString&, unsigned int) { return false; }
		virtual bool GetNextFile(TCString&, uint32_t) { return false; }
		virtual void SetPrefix(TCString const&);
		virtual bool MakeDirectory(TCString const&) = 0;

		inline TCString const& GetName() const { return m_sName; }
		inline TCString const& GetPrefix() const { return m_prefix; }

		TFileSystem& operator=(TFileSystem& a_rFileSystem);

	public:
		// Creates native file system based on the used platform
		static STL::Ref<TFileSystem> CreateNative(const char* name);
	
	protected:
		void* m_unk; // 0x4
		void* m_unk2; // 0x8

		TCString m_sName; // 0xC
		TCString m_prefix; // 0x14
		int m_unk3; // 0x1C
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

		TFile(TFileSystem*);
		//TFile();
		TFile(const TFile&);


		virtual _FILETIME GetDate() = 0;
		static TCString ConcatPath(const TCString& a_rcString, const TCString& a_rcString2);
		static TFile* Create(const TCString&, unsigned int);
		inline TFileSystem* GetFileSystem() const { return m_pFileSystem; }
		inline TFile& operator=(const TFile& a_pFile) { m_pFileSystem = a_pFile.GetFileSystem(); return *this; }


	public:
		TFileSystem* m_pFileSystem;
	};

	class TFileManager : public TSingleton<TFileManager>
	{
	public:
		TCString m_sSysPath; // 0x4
		TCString m_sWorkingDirectory; // 0xC
		void* m_lList; // 0x14 Actually TDList but my brain can't handle it
		void* m_lList2; // 0x1C
		TMutex m_mMutex; // 0x24


		TFileManager() = default;

		// JPOG Matched
		class TSysPathIter
		{
			TCString m_sSysPath;
			int m_position;
		public:
			bool First(TCString & param_1);
			bool Next(TCString& param_1);

			TSysPathIter(const TCString&);
			TSysPathIter(const TSysPathIter&);
		};

	private:
		void ValidateSystemPath();

	public:
		void MountFileSystem(TFileSystem* a_pFileSystem);
		class TFileSystem* FindFileSystem(const TCString&);
		//static TFileSystem* FindFileSystem(Toshi::TGenericDList<TFileSystem, 0>* param_1):

		inline TCString MakeAbsolutePath(const TCString& a_cString) const { return TFile::ConcatPath(a_cString, m_sWorkingDirectory); }
		inline void FileSystemRelease() { m_mMutex.Unlock(); }
		inline void FileSystemWait() { m_mMutex.Lock(); }
		class TFile* CreateFile(const TCString&, unsigned int);
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

		TFile(TFileSystem*);
		//TFile();
		TFile(const TFile&);

		virtual _FILETIME GetDate() = 0;
		static TFile* Create(const TCString&, unsigned int);
		inline TFileSystem* GetFileSystem() const { return m_pFileSystem; }
		inline TFile& operator=(const TFile& a_pFile) { m_pFileSystem = a_pFile.GetFileSystem(); return *this; }


	public:
		TFileSystem* m_pFileSystem;
	};
}


