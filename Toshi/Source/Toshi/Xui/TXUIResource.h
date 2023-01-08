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
			Section* m_apSections;		// 0x0
			uint32_t m_uiFileID;		// 0x4
			uint32_t m_uiUnk;			// 0x8
			uint32_t m_uiUnk2;			// 0xC
			uint32_t m_uiSize;			// 0x10
			uint16_t m_usUnk4;			// 0x14
			uint16_t m_usNumSections;	// 0x16
		};
	public:
		TXUIResource()
		{
			m_uiStringTableCount = 0;
			m_asStringTable = TNULL;
			m_pQuat = TNULL;
		}

		~TXUIResource() = default;

		inline const wchar_t* GetString(uint16_t index) { return m_asStringTable[index]; }
		TQuaternion* GetQuat(int a_iIndex = 0);
		TVector4* GetVector(int a_iIndex = 0);
		//inline TQuaternion& GetQuat(int index) { return index == -1 ? TQuaternion::IDENTITY : m_pQuat; }

		bool ReadHeader(unsigned char* buffer);

		// Toshi::TXUIResource::Load(const char*, const char*, bool, Toshi::TTRB*)

		// loadStringTables = when true load .xus/.trb files (StringTables)
		// filename = f.e Data/XUI/%s.trb or Data/XUI/%s.xur
		// fileNameStringTable = f.e %s/StringTables/%s/%s.xus or %s/StringTables/%s/%s.trb
		// loadTrb = when true load .xur/.trb files (XUIB)
		// unk3 = probably a function
		void Load(bool loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, bool loadTrb, void* unk3);

		bool Load(unsigned char* buffer);
		int ReadDataSection(unsigned char* buffer, uint32_t size);
		bool ReadStringSection(wchar_t* pPtr, uint32_t size);
		int ReadCustSection(unsigned char* buffer, uint32_t size);
		int GetStringTableSize(unsigned char* pPtr, uint32_t size);

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
		XURHeader m_oHeader;                           // 0x0
		wchar_t** m_asStringTable = TNULL;          // 0x1C
		uint8_t* m_pData;
		TVector4* m_pVect; // 0x1C globs
		TQuaternion* m_pQuat;                       // 0x20 both
		unsigned char* m_pCust;                     // 0x28 / 0x24 globs
		uint32_t m_uiCustDataSize;                  // 0x3C / 0x38 globs
		uint32_t m_uiStringTableCount;              // 0x30 / 0x3C globs
		TString16* m_uIDStr; // 0x40
		uint32_t m_uIDStrLength; // 0x44

		XURXUIObjectData* m_root;

		TStack<const wchar_t*, 32> m_uIDs;
	public:
		static inline int s_iUIDCount = 0;
		static inline bool s_bGenerateUIDs = false;
	};
}
