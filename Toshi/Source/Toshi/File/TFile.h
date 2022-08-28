#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Strings/TCString.h"

#ifdef CreateFile
#undef CreateFile
#endif

namespace Toshi
{

	class TFileSystem
	{
	public:
		TFileSystem(TFileSystem const&);
		TFileSystem(const char*);
		virtual ~TFileSystem();

		// Override these funcs in TNativeFileSystem
		virtual bool RemoveFile(TCString const&) { return true; }
		virtual bool GetFirstFile(TCString const&, TCString&, unsigned int) { return false; }
		virtual bool GetNextFile(TCString&, unsigned int) { return false; }
		virtual void SetPrefix(TCString const&);

		inline TCString const& GetName() const { return m_sName; }
		inline TCString const& GetPrefix() const { return m_prefix; }

		void* m_unk; // 0x4
		void* m_unk2; // 0x8

		TCString m_sName; // 0xC
		TCString m_prefix; // 0x14
		int m_unk3; // 0x1C
	};

	class TFileManager : public TSingleton<TFileManager>
	{
	public:
		TFileManager() = default;

	private:
		void ValidateSystemPath();

	public:
		void MountFileSystem(TFileSystem* a_pFileSystem);
		class TFileSystem* FindFileSystem(const TCString&);
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


