#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/TDList.h"
#include "Toshi/TString8.h"

#include "Thread/T2MutexLock.h"

#ifdef CreateFile
#undef CreateFile
#endif

namespace Toshi {

    class TFile;

	enum TSEEK
	{
		TSEEK_SET,
		TSEEK_CUR,
		TSEEK_END
	};

	typedef TUINT16 TFILEMODE;
	enum TFILEMODE_ : TFILEMODE
	{
		TFILEMODE_READ = BITFLAG(0),
		TFILEMODE_WRITE = BITFLAG(1),
		TFILEMODE_READWRITE = BITFLAG(2),
		TFILEMODE_CREATENEW = BITFLAG(3),
		TFILEMODE_NOBUFFER = BITFLAG(4),
	};

    // JPOG Matched
    class TFileSystem : public TDList<TFileSystem>::TNode
    {
    public:
        TFileSystem(const TCHAR* a_szName);
        TFileSystem(const TFileSystem& other);

        virtual ~TFileSystem() { UnmountFileSystem(); }
        
        /**
         * Creates file.
         * @param a_rcFileName path to the file to open
         * @param a_eFileMode mode to open the file with
         * @return pointer to the opened file or TNULL if couldn't open
         */
        virtual TFile* CreateFile(const TString8& a_rcFileName, TFILEMODE a_eFileMode) = 0;
        
        /**
         * Destroys specified file.
         */
        virtual void DestroyFile(TFile* a_pFile) = 0;

        /**
         * Removes specified file.
         */
        virtual TBOOL RemoveFile(const TString8& a_rcFileName) { return TTRUE; }

        /**
         * Not documented.
         */
        virtual TString8 MakeInternalPath(const TString8& a_rcPath) = 0;
        
        /**
         * Finds out what's the first file at the specified path.
         */
        virtual TBOOL GetFirstFile(const TString8& a_rcPath, TString8& a_rOutFileName, TUINT32 a_uiFlags) { return TFALSE; }
        
        /**
         * Not documented.
         */
        virtual TBOOL GetNextFile(TString8& a_rOutFileName, TUINT32 a_uiFlags) { return TFALSE; }
        
        /**
         * Sets the directory prefix to work with.
         */
        virtual void SetPrefix(const TString8& a_rcPrefix);
        
        /**
         * Creates directory.
         */
        virtual TBOOL MakeDirectory(const TString8& a_rcDirectory) = 0;
        
        /**
         * Unmounts this file system from the TFileManager.
         */
        void UnmountFileSystem() { TNode::Remove(); }

        const TString8& GetName() const { return m_Name; }
        const TString8& GetPrefix() const { return m_Prefix; }

        TFileSystem& operator=(TFileSystem& a_rFileSystem);

    protected:
        TString8 m_Name;   // 0xC
        TString8 m_Prefix; // 0x18
        HANDLE m_Handle;   // 0x1C
    };

    class TFile
    {
    public:
        TFile(TFileSystem* a_pFS);
        TFile(const TFile& a_rOther);

        /**
         * Reads specified number of bytes from the file into the buffer.
		 * @param a_pDst destination buffer
		 * @param a_uiSize number of bytes to read
         * @return the number of read bytes
         */
        virtual TUINT Read(void* a_pDst, TUINT a_uiSize) = 0;

        /**
         * Writes specified number of bytes to the file from the buffer.
         * @param a_pSrc sourde buffer
         * @param a_uiSize number of bytes to write
         * @return the number of written bytes
         */
        virtual TUINT Write(const void* a_pSrc, TUINT a_uiSize) = 0;
        
        /**
         * Shifts current file cursor based on the specified offset and origin.
         * @param a_iOffset number of characters to shift the position
         * @param a_eOrigin position to which a_iOffset is added
         * @return TTRUE if succeeded
         */
        virtual TBOOL Seek(TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR) = 0;

        /**
         * @return current position of the file cursor
         */
        virtual TUINT Tell() = 0;

        /**
         * @return size of the file
         */
        virtual TUINT GetSize() = 0;

        /**
         * @return last write time
         */

        virtual _FILETIME GetDate() { return {}; }

        /**
         * Reads one character of type TCHAR from the file.
         * @return the read character
         */
        virtual TCHAR GetCChar() = 0;

        /**
         * Reads one character of type wchar from the file.
         * @return the read character
         */
        virtual TWCHAR GetWChar() = 0;

        /**
         * Writes one character of type TCHAR to the file.
         * @return code of the written character
         */
        virtual TINT PutCChar(TCHAR a_cCharacter) = 0;
        
        /**
         * Writes one character of type wchar to the file.
         * @return number of written bytes
         */
        virtual TINT PutWChar(TWCHAR a_wcCharacter) = 0;

        /**
         * Analogue of printf but writes result to the file.
         * @return number of characters written
         */
        virtual TINT CPrintf(const TCHAR* a_szFormat, ...) = 0;

        /**
         * Analogue of wprintf but writes result to the file.
         * @return number of characters written
         */
        virtual TINT WPrintf(const TWCHAR* a_wszFormat, ...) = 0;

        /**
         * Analogue of vprintf but writes result to the file.
         * @return number of characters written
         */
        virtual TINT VCPrintf(const TCHAR* a_szFormat, va_list a_vargs) = 0;

        /**
         * Analogue of vwprintf but writes result to the file.
         * @return number of characters written
         */
        virtual TINT VWPrintf(const TWCHAR* a_wszFormat, va_list a_vargs) = 0;

        virtual ~TFile() { }
        
        TFileSystem* GetFileSystem() const
        {
            return m_pFileSystem;
        }

        void Destroy()
        {
            TVALIDPTR(m_pFileSystem);
            m_pFileSystem->DestroyFile(this); 
        }

	public:
		static TString8 ConcatPath(const TString8& a_rcString, const TString8& a_rcString2);

		static TFile* Create(const TString8& a_rcFilename, TFILEMODE a_eMode = TFILEMODE_READ);

    private:
        TFileSystem* m_pFileSystem;
    };

    class TFileManager : public TSingleton<TFileManager>
    {
    public:
        class TSysPathIter
        {
        public:
            TSysPathIter(const TString8& str);;
            TSysPathIter(const TSysPathIter& other);;

            TBOOL First(TString8& path);

            TBOOL Next(TString8& path);

        private:
            const TString8& m_String;
            int32_t m_Position;
        };

    public:
        TFileManager();
        ~TFileManager() { Destroy(); }

        void Destroy();
        void MountFileSystem(TFileSystem* a_pFileSystem);

        TFile* CreateFile(const TString8& a_sName, TFILEMODE flags);

        TFileSystem* FindFileSystem(const TString8& name);
        static TFileSystem* FindFileSystem(TDList<TFileSystem>& list, const TString8& name);

        TString8 MakeAbsolutePath(const TString8& a_cString) const;
        
        void FileSystemRelease() { m_Mutex.Unlock(); }
        void FileSystemWait() { m_Mutex.Lock(); }

        void SetSystemPath(const TString8& name) { m_SysPath = name; InvalidateSystemPath(); }

    public:
        /*
        * Platform specific methods
        * Define them in TFileManager_{Platform}.cpp
        */

        static TBOOL Create();

    private:
        void ValidateSystemPath();
        void InvalidateSystemPath() { m_IsValidated = TFALSE; }

		static void CreateCommon();
		static void DestroyCommon();

    private:
        TBOOL m_IsValidated;               // 0x0
        TString8 m_SysPath;                // 0x4
        TString8 m_WorkingDirectory;       // 0x10
        TUINT32 m_ValidatedCount;         // 0x1C
        TDList<TFileSystem> m_Validated;   // 0x20
        TDList<TFileSystem> m_Invalidated; // 0x28
        T2Mutex m_Mutex;                   // 0x30
    };

}

