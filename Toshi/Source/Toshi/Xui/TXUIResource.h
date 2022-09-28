#pragma once

#include <Toshi/File/TFile.h>
#include "Toshi/Core/TSystem.h"
#include "Toshi/Toshi2/T2String16.h"

namespace Toshi
{
	class TXUIResource
	{

		const uint32_t IDXUR		= TMAKEFOUR("XUIB");
		const uint32_t IDXURSTRING	= TMAKEFOUR("STRN");
		const uint32_t IDXURVEC		= TMAKEFOUR("VECT");
		const uint32_t IDXURQUAT	= TMAKEFOUR("QUAT");
		const uint32_t IDXURCUST	= TMAKEFOUR("CUST");
		const uint32_t IDXURDATA	= TMAKEFOUR("DATA");

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
			uint32_t m_uiUnk3;			// 0x10
			uint16_t m_usUnk4;			// 0x14
			uint32_t m_uiNumSections;	// 0x16
		};

		
		Header m_oHeader;							// 0x0
		unsigned short** m_asStringTable = TNULL;	// 0x1C
		unsigned char* m_pCust;						// 0x28
		uint32_t m_uiCustSize;						// 0x3C
		uint32_t m_uiStringTableCount;				// 0x30
	public:
		inline const wchar_t* GetString(uint16_t index) { return L""; }

		bool LoadXUIB(unsigned char* buffer);
		int ProcessSections(unsigned char* buffer);
		int ProcessDATA(unsigned char* buffer);
		bool ProcessSTRN(unsigned short* pPtr, uint32_t size);
		int GetStringTableSize(unsigned char* pPtr, uint32_t size);
	};
}

