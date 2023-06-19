#pragma once
#include "TXUIStringTable.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/TStack.h"
#include "Toshi/Strings/TString16.h"
#include "Toshi2/T2String16.h"

namespace Toshi
{
    class XURXUIObjectData;

    class TXUIResource
    {
        static const uint32_t IDXUR = TMAKEFOUR("XUIB");
        static const uint32_t IDXURSTRING = TMAKEFOUR("STRN");
        static const uint32_t IDXURVEC = TMAKEFOUR("VECT");
        static const uint32_t IDXURQUAT = TMAKEFOUR("QUAT");
        static const uint32_t IDXURCUST = TMAKEFOUR("CUST");
        static const uint32_t IDXURDATA = TMAKEFOUR("DATA");

        struct Section
        {
            uint32_t m_uiSectionID;     // 0x0
            uint32_t m_uiOffset;        // 0x4
            uint32_t m_uiSize;          // 0x8
        };

        struct XURHeader
        {
            Section* m_apSections;      // 0x00
            uint32_t m_uiFileID;        // 0x04
            uint32_t m_uiVersion;       // 0x08
            uint32_t m_uiXuiVersion;    // 0x0C
            uint32_t m_usBinSize;       // 0x10
            uint16_t m_uiFlags;         // 0x12
            uint16_t m_usNumSections;   // 0x14
        };

    public:
        TXUIResource()
        {
			m_oHeader.m_apSections = TNULL;
			m_oHeader.m_uiFileID = 0;
			m_oHeader.m_uiVersion = 0;
			m_oHeader.m_uiXuiVersion = 0;
			m_oHeader.m_usBinSize = 0;
			m_oHeader.m_uiFlags = 0;
			m_oHeader.m_usNumSections = 0;
			m_pMemoryHeap = TNULL;
			m_asStringTable = TNULL;
			m_avVectors = TNULL;
			m_aqZRotTable = TNULL;
			m_pCustData = TNULL;
			m_pRoot = TNULL;
			m_uiStringCount = 0;
			m_uiVectorCount = 0;
			m_uiQuatCount = 0;
			m_pStringTable = 0;
        }

		~TXUIResource();

        const wchar_t* GetString(uint16_t index)
		{
			return m_asStringTable[index];
		}

		float GetZRotation(int a_iIndex = 0)
		{
			TASSERT(a_iIndex >= -1);
			return (a_iIndex == -1) ? 0 : m_aqZRotTable[a_iIndex];
		}

		const TVector2* GetVector(int a_iIndex = 0)
		{
			TASSERT(a_iIndex >= -1);
			return (a_iIndex == -1) ? &TVector2::VEC_ZERO : &m_avVectors[a_iIndex];
		}

        TBOOL ReadHeader(uint8_t* buffer);

        // Toshi::TXUIResource::Load(const char*, const char*, TBOOL, Toshi::TTRB*)

        // loadStringTables = when TTRUE load .xus/.trb files (StringTables)
        // filename = f.e Data/XUI/%s.trb or Data/XUI/%s.xur
        // fileNameStringTable = f.e %s/StringTables/%s/%s.xus or %s/StringTables/%s/%s.trb
        // loadTrb = when TTRUE load .xur/.trb files (XUIB)
        // unk3 = probably a function
        void Load(TBOOL loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, TBOOL loadTrb, void* unk3);

        TBOOL Load(uint8_t* buffer);
		TBOOL ReadDataSection(uint8_t* buffer, uint32_t size);
        TBOOL ReadStringSection(uint8_t* buffer, uint32_t size);
        TBOOL ReadCustSection(uint8_t* buffer, uint32_t size);
		TBOOL ReadQuatSection(uint8_t* buffer, uint32_t size);
        int GetStringTableSize(uint8_t* pPtr, uint32_t size);

        void PushID(const wchar_t* a_wsID);
        void PopID();

        TXUIStringTable& LookupStringTable();

        static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, uint16_t index);
        static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, const wchar_t* objectName);

    private:
		void Destroy();

    private:
        XURHeader m_oHeader;                             // 0x00
		TMemoryHeap* m_pMemoryHeap;                      // 0x18
        wchar_t** m_asStringTable;                       // 0x1C
		TVector2* m_avVectors;                           // 0x20
		float* m_aqZRotTable;                            // 0x24
        uint8_t* m_pCustData;                            // 0x28
		XURXUIObjectData* m_pRoot;                       // 0x2C
		uint32_t m_uiStringCount;                        // 0x30
		uint32_t m_uiVectorCount;                        // 0x34
		uint32_t m_uiQuatCount;                          // 0x38
		uint32_t m_uiCustDataSize;                       // 0x3C
		TXUIStringTable* m_pStringTable;                 // 0x40
		TString16 m_ID;                                  // 0x44
        TStack<const wchar_t*, 32> m_oIDComparisonStack; // 0x50
    public:
        static inline int s_iUIDCount = 0;
        static inline TBOOL s_bGenerateUIDs = TFALSE;
    };
}
