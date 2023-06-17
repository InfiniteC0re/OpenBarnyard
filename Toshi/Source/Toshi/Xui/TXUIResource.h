#pragma once

#include "Toshi/File/TFile.h"

#include "Toshi/Core/TSystem.h"
#include "Toshi2/T2String16.h"
#include "Toshi/Core/TStack.h"
#include "Toshi/Strings/TString16.h"
#include "TXUIStringTable.h"

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
			uint32_t m_uiSectionID;		// 0x0
			uint32_t m_uiOffset;		// 0x4
			uint32_t m_uiSize;			// 0x8
		};

		struct XURHeader
		{
			Section* m_apSections;      // 0x00
			uint32_t m_uiFileID;        // 0x04
			uint32_t m_uiVersion;       // 0x08
			uint32_t m_uiFlags;         // 0x0C
			uint16_t m_uiXuiVersion;    // 0x10
			uint32_t m_usBinSize;       // 0x14
			uint16_t m_usNumSections;   // 0x18
		};

	public:
		TXUIResource()
		{
			m_uiStringTableCount = 0;
			m_asStringTable = TNULL;
			m_pQuat = TNULL;
		}

		~TXUIResource() = default;

		const wchar_t* GetString(uint16_t index) { return m_asStringTable[index]; }
		TQuaternion* GetQuat(int a_iIndex = 0);
		TVector4* GetVector(int a_iIndex = 0);
		//inline TQuaternion& GetQuat(int index) { return index == -1 ? TQuaternion::IDENTITY : m_pQuat; }

		TBOOL ReadHeader(uint8_t* buffer);

		// Toshi::TXUIResource::Load(const char*, const char*, TBOOL, Toshi::TTRB*)

		// loadStringTables = when TTRUE load .xus/.trb files (StringTables)
		// filename = f.e Data/XUI/%s.trb or Data/XUI/%s.xur
		// fileNameStringTable = f.e %s/StringTables/%s/%s.xus or %s/StringTables/%s/%s.trb
		// loadTrb = when TTRUE load .xur/.trb files (XUIB)
		// unk3 = probably a function
		void Load(TBOOL loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, TBOOL loadTrb, void* unk3);

		TBOOL Load(uint8_t* buffer);
		int ReadDataSection(uint8_t* buffer, uint32_t size);
		TBOOL ReadStringSection(uint8_t* buffer, uint32_t size);
		int ReadCustSection(uint8_t* buffer, uint32_t size);
		int GetStringTableSize(uint8_t* pPtr, uint32_t size);

		void PushID(const wchar_t* a_wsID);
		void PopID();

		TXUIStringTable& LookupStringTable();

		static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, uint16_t index);
		static XURXUIObjectData* CreateObjectData(TXUIResource& a_rResource, const wchar_t* objectName);

	private:
		void Destroy()
		{
			for (size_t i = 0; i < m_uiStringTableCount; i++)
			{
				delete m_asStringTable[i];
			}

			if (m_asStringTable) delete[] m_asStringTable;
			if (m_pQuat) delete[] m_pQuat;

			// this+0x2C(1) probably function pointer parameter=int (value 1 in this case)
		}

	private:
		XURHeader m_oHeader;                        // 0x0
		wchar_t** m_asStringTable = TNULL;          // 0x1C
		uint8_t* m_pData;
		TVector4* m_pVect;                          // 0x1C globs
		TQuaternion* m_pQuat;                       // 0x20 both
		uint8_t* m_pCust;                           // 0x28 / 0x24 globs
		uint32_t m_uiCustDataSize;                  // 0x3C de blob / 0x38 globs
		uint32_t m_uiStringTableCount;              // 0x30 / 0x3C globs
		TString16* m_uIDStr;                        // 0x40
		uint32_t m_uIDStrLength;                    // 0x44

		XURXUIObjectData* m_root;

		TStack<const wchar_t*, 32> m_oIDComparisonStack;
	public:
		static inline int s_iUIDCount = 0;
		static inline TBOOL s_bGenerateUIDs = TFALSE;
	};
}
