#pragma once

#include <Toshi/File/TFile.h>
#include "Toshi/Core/TSystem.h"
#include "Toshi2/T2String16.h"
#include "Toshi/Xui/TXUICanvas.h"

namespace Toshi
{
	class TXUIResource
	{
		static const uint32_t IDXUR			= TMAKEFOUR("XUIB");
		static const uint32_t IDXURSTRING	= TMAKEFOUR("STRN");
		static const uint32_t IDXURVEC		= TMAKEFOUR("VECT");
		static const uint32_t IDXURQUAT		= TMAKEFOUR("QUAT");
		static const uint32_t IDXURCUST		= TMAKEFOUR("CUST");
		static const uint32_t IDXURDATA		= TMAKEFOUR("DATA");

		struct Section
		{
			uint32_t m_uiSectionID;		// 0x0
			uint32_t m_uiOffset;		// 0x4
			uint32_t m_uiSize;			// 0x8
		};

		struct Header
		{
			Section* m_apSections;		// 0x0
			uint32_t m_uiFileID;		// 0x4
			uint32_t m_uiUnk;			// 0x8
			uint32_t m_uiUnk2;			// 0xC
			uint32_t m_uiSize;			// 0x10
			uint16_t m_usUnk4;			// 0x14
			uint16_t m_usNumSections;	// 0x16
		};

		inline static TMemoryHeap* ms_pXUIMemoryBlock = TNULL;
		
		Header m_oHeader;							// 0x0
		unsigned short** m_asStringTable = TNULL;	// 0x1C
		TQuaternion* m_pQuat;						// 0x20
		uint8_t* m_pData;
		unsigned char* m_pCust;						// 0x28
		uint32_t m_uiCustDataSize;					// 0x3C
		uint32_t m_uiStringTableCount;				// 0x30
	public:
		inline const wchar_t* GetString(uint16_t index) { return L""; }
		//inline TQuaternion& GetQuat(int index) { return index == -1 ? TQuaternion::IDENTITY : m_pQuat; }

		bool ReadHeader(unsigned char* buffer);

		// Toshi::TXUIResource::Load(const char*, const char*, bool, Toshi::TTRB*)
		// 
		// loadStringTables = when true load .xus/.trb files (StringTables)
		// filename = f.e Data/XUI/%s.trb or Data/XUI/%s.xur
		// fileNameStringTable = f.e %s/StringTables/%s/%s.xus or %s/StringTables/%s/%s.trb
		// loadTrb = when true load .xur/.trb files (XUIB)
		// unk3 = probably a function
		void Load(bool loadStringTables, const char* filenameXUIB, const char* fileNameStringTable, bool loadTrb, void* unk3);

		int Load(unsigned char* buffer);
		int ReadDataSection(unsigned char* buffer, uint32_t size);
		bool ReadStringSection(unsigned short* pPtr, uint32_t size);
		int ReadCustSection(unsigned char* buffer, uint32_t size);
		int GetStringTableSize(unsigned char* pPtr, uint32_t size);

		uint8_t* CreateObjectData(TXUIResource& a_rResource, uint16_t index);
		uint8_t* CreateObjectData(TXUIResource& a_rResource, const wchar_t* objectName);
	};
}

